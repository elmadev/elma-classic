#include "LEPTET.H"
#include "EDITUJ.H"
#include "level.h"
#include "object.h"
#include "physics_collision.h"
#include "physics_init.h"
#include "physics_move.h"
#include "platform_utils.h"
#include "recorder.h"
#include <cmath>

static void update_friction_volume(motorst* mot, vect2 wheel_displacement,
                                   vect2 wheel_relative_velocity);

static double MaxFrictionVolume = 0;

static void clamp_wheel_rotation(double* rotation) {
    if (*rotation < -PI) {
        *rotation += 2 * PI;
    }
    if (*rotation > PI) {
        *rotation -= 2 * PI;
    }
}

/*static double Elmozdhatar1 = 10.4;
static double Elmozdhatar2 = 10.5;
static double Atmenet = Elmozdhatar2 - Elmozdhatar1;
static double Szorzo = 2.0;

static pic8* Ppic8 = NULL;
*/

static void calculate_wheel_forces(motorst* mot, rigidbody* rb, vect2 i1, vect2 j1, double wheel_dx,
                                   double wheel_dy, vect2* force_wheel, vect2* force_bike,
                                   double* torque_bike, double* torque_wheel) {
    vect2 wheel_neutral_position_relative = i1 * wheel_dx + j1 * wheel_dy;
    vect2 wheel_neutral_position_absolute = wheel_neutral_position_relative + mot->bike.r;

    // Gumi huzoero kiszamolasa:
    vect2 wheel_displacement = wheel_neutral_position_absolute - rb->r;
    double tension_force_wheel_parallel = 0;
    double tension_force_wheel_orthogonal = 0;
    if (wheel_displacement.x < -0.0001 || wheel_displacement.x > 0.0001 ||
        wheel_displacement.y < -0.0001 || wheel_displacement.y > 0.0001) {
        // Gumieronek szetbontasa ket komponensre:
        double neutral_spring_length = wheel_neutral_position_relative.length();
        vect2 neutral_spring_unit = wheel_neutral_position_relative * (1.0 / neutral_spring_length);
        vect2 neutral_spring_unit_ortho = rotate_90deg(neutral_spring_unit);

        tension_force_wheel_parallel =
            (wheel_displacement * neutral_spring_unit) * SpringTensionCoefficient;
        tension_force_wheel_orthogonal =
            (wheel_displacement * neutral_spring_unit_ortho) * SpringTensionCoefficient;

        *force_wheel = tension_force_wheel_parallel * neutral_spring_unit +
                       tension_force_wheel_orthogonal * neutral_spring_unit_ortho;

        *force_bike = Vect2null - *force_wheel;
        *torque_bike = -tension_force_wheel_orthogonal * neutral_spring_length;
    } else {
        *force_wheel = vect2();
        *force_bike = vect2();
        *torque_bike = 0;
    }
    // Most jon surlodas szamitasa:
    vect2 current_spring = rb->r - mot->bike.r;
    double current_spring_length = current_spring.length();
    double current_spring_length_reciprocal = 1.0 / current_spring_length;
    vect2 current_spring_unit = current_spring * current_spring_length_reciprocal;
    vect2 current_spring_perp = rotate_90deg(current_spring);
    vect2 current_spring_unit_perp = rotate_90deg(current_spring_unit);
    vect2 relative_velocity =
        (current_spring_perp * mot->bike.angular_velocity + mot->bike.v) - rb->v;
    double relative_velocity_parallel = relative_velocity * current_spring_unit;
    double relative_velocity_orthogonal = relative_velocity * current_spring_unit_perp;
    vect2 damping_force_wheel_parallel =
        (relative_velocity_parallel * SpringResistanceCoefficient) * current_spring_unit;
    vect2 damping_force_wheel_orthogonal =
        (relative_velocity_orthogonal * SpringResistanceCoefficient) * current_spring_unit_perp;

    // Most jon kerek forgatonyomateka:
    vect2 gasbrake_force_body =
        current_spring_unit_perp * (*torque_wheel * current_spring_length_reciprocal);

    *force_wheel = *force_wheel + damping_force_wheel_parallel + damping_force_wheel_orthogonal -
                   gasbrake_force_body;
    *torque_bike += -(damping_force_wheel_orthogonal * current_spring_perp);
    *force_bike = *force_bike - damping_force_wheel_parallel - damping_force_wheel_orthogonal +
                  gasbrake_force_body;

    update_friction_volume(mot, wheel_displacement, relative_velocity);
    // surlodasverseny( Ftang, vtang );
}

// static Voltfek = 0;
// static double Dfek2 = 0, Dfek4 = 0;
static double VOLT_ANGULAR_VELOCITY = 12.0;
static double COUNTERVOLT_ANGULAR_VELOCITY = 3.0;

// static int Ugrasban1 = 1, Ugrasban2 = 1; // Ezeket nullazni kell minden kezdesnel!
// static double Ugras1kezd = -1.0, Ugras2kezd = -1.0;
// static double Kezdoomega1 = -1.0, Kezdoomega2 = -1.0;

void reset_motor_forces(motorst* mot) {
    mot->prev_brake = 0;
    mot->left_wheel_brake_rotation = mot->right_wheel_brake_rotation = 0;
    mot->volting_right = mot->volting_left = 0;
    mot->right_volt_time = mot->left_volt_time = -1.0;
    mot->angular_velocity_pre_right_volt = mot->angular_velocity_pre_left_volt = -1.0;
}

// Kor1-bol eloallitja Fejr-et!:
void set_head_position(motorst* mot) {
    vect2 i1(cos(mot->bike.rotation), sin(mot->bike.rotation));
    vect2 j1 = rotate_90deg(i1);

    if (mot->flipped_bike) {
        mot->head_r = mot->body_r + i1 * 0.09 + j1 * 0.63;
    } else {
        mot->head_r = mot->body_r - i1 * 0.09 + j1 * 0.63;
    }
}

void simulate_bike_physics(motorst* mot, double time, double dt, int gas, int brake, int right_volt,
                           int left_volt) {
    MaxFrictionVolume = 0;

    vect2 i1(cos(mot->bike.rotation), sin(mot->bike.rotation));
    vect2 j1 = rotate_90deg(i1);

    // Fek, gaz:
    if (!mot->prev_brake && brake) {
        mot->left_wheel_brake_rotation = mot->left_wheel.rotation - mot->bike.rotation;
        mot->right_wheel_brake_rotation = mot->right_wheel.rotation - mot->bike.rotation;
    }
    mot->prev_brake = brake;
    double torque_left_wheel = 0;
    double torque_right_wheel = 0;
    if (gas) {
        double MAX_ANGULAR_VELOCITY = 110.0;
        double GAS_TORQUE = 600.0;
        if (mot->flipped_bike) {
            if (mot->left_wheel.angular_velocity > -MAX_ANGULAR_VELOCITY) {
                torque_left_wheel = -GAS_TORQUE;
            }
        } else {
            if (mot->right_wheel.angular_velocity < MAX_ANGULAR_VELOCITY) {
                torque_right_wheel = GAS_TORQUE;
            }
        }
    }
    if (brake) {
        double ANGULAR_SPRING_TENSION_COEFFICIENT = 1000.0;
        double ANGULAR_SPRING_RESISTANCE_COEFFICIENT = 100.0;

        double relative_rotation =
            mot->left_wheel.rotation - (mot->bike.rotation + mot->left_wheel_brake_rotation);
        double relative_angular_velocity =
            mot->left_wheel.angular_velocity - mot->bike.angular_velocity;
        torque_left_wheel = -ANGULAR_SPRING_TENSION_COEFFICIENT * relative_rotation -
                            ANGULAR_SPRING_RESISTANCE_COEFFICIENT * relative_angular_velocity;

        relative_rotation =
            mot->right_wheel.rotation - (mot->bike.rotation + mot->right_wheel_brake_rotation);
        relative_angular_velocity = mot->right_wheel.angular_velocity - mot->bike.angular_velocity;
        torque_right_wheel = -ANGULAR_SPRING_TENSION_COEFFICIENT * relative_rotation -
                             ANGULAR_SPRING_RESISTANCE_COEFFICIENT * relative_angular_velocity;
    } else {
        clamp_wheel_rotation(&mot->left_wheel.rotation);
        clamp_wheel_rotation(&mot->right_wheel.rotation);
    }

    vect2 force_left_wheel;
    vect2 force_body_from_left_wheel;
    double torque_body_from_left_wheel;
    calculate_wheel_forces(mot, &mot->left_wheel, i1, j1, LeftWheelDX, LeftWheelDY,
                           &force_left_wheel, &force_body_from_left_wheel,
                           &torque_body_from_left_wheel, &torque_left_wheel);

    vect2 force_right_wheel;
    vect2 force_body_from_right_wheel;
    double torque_body_from_right_wheel;
    calculate_wheel_forces(mot, &mot->right_wheel, i1, j1, RightWheelDX, RightWheelDY,
                           &force_right_wheel, &force_body_from_right_wheel,
                           &torque_body_from_right_wheel, &torque_right_wheel);

    // Ugras elintezese:
    // Eloszor ugras befejezese, ha kell:
    double prevolt_angular_velocity;
    if (right_volt || left_volt) {
        prevolt_angular_velocity = mot->bike.angular_velocity;
    }

    if (mot->volting_right &&
        (right_volt || left_volt || time > mot->right_volt_time + VoltDelay * 0.25)) {
        mot->bike.angular_velocity += VOLT_ANGULAR_VELOCITY;
        if (mot->bike.angular_velocity > mot->angular_velocity_pre_right_volt) {
            mot->bike.angular_velocity = mot->angular_velocity_pre_right_volt;
        }
        // Meg marado szogsebesseg valtozast is okozunk, ha nem forgatjuk tul:
        if (mot->bike.angular_velocity > 0.0) {
            mot->bike.angular_velocity -= COUNTERVOLT_ANGULAR_VELOCITY;
            if (mot->bike.angular_velocity < 0.0) {
                mot->bike.angular_velocity = 0.0;
            }
        }
        mot->volting_right = 0;
        mot->angular_velocity_pre_right_volt = -1.0;
        mot->right_volt_time = -1.0;
    }
    if (mot->volting_left &&
        (right_volt || left_volt || time > mot->left_volt_time + VoltDelay * 0.25)) {
        mot->bike.angular_velocity -= VOLT_ANGULAR_VELOCITY;
        if (mot->bike.angular_velocity < mot->angular_velocity_pre_left_volt) {
            mot->bike.angular_velocity = mot->angular_velocity_pre_left_volt;
        }
        // Meg marado szogsebesseg valtozast is okozunk, ha nem forgatjuk tul:
        if (mot->bike.angular_velocity < 0.0) {
            mot->bike.angular_velocity += COUNTERVOLT_ANGULAR_VELOCITY;
            if (mot->bike.angular_velocity > 0.0) {
                mot->bike.angular_velocity = 0.0;
            }
        }
        mot->volting_left = 0;
        mot->angular_velocity_pre_left_volt = -1.0;
        mot->left_volt_time = -1.0;
    }

    // if( crcido % 511 == 103 )
    //   crccheck2(); Most ez nincs is belinkelve

    // Most ugras kezdes, ha kell:
    if (right_volt) {
        // Csak azert van kikomentezve, mert egyszer elojott:
        // if( Ugrasban1 || Ugrasban2 )
        //  internal_error( "Ugrasban!" );
        mot->volting_right = 1;
        mot->angular_velocity_pre_right_volt = mot->bike.angular_velocity;
        mot->right_volt_time = time;
        mot->bike.angular_velocity -= VOLT_ANGULAR_VELOCITY;
    }
    if (left_volt) {
        // if( Ugrasban1 || Ugrasban2 )
        //   internal_error( "Ugrasban!" );
        mot->volting_left = 1;
        mot->angular_velocity_pre_left_volt = mot->bike.angular_velocity;
        mot->left_volt_time = time;
        mot->bike.angular_velocity += VOLT_ANGULAR_VELOCITY;
    }
    if (right_volt || left_volt) {
        // Vezetot kulon forgatjuk meg:
        double bike_relative_angular_velocity =
            mot->bike.angular_velocity - prevolt_angular_velocity;
        vect2 body_spring_perp = rotate_90deg(mot->body_r - mot->bike.r);
        mot->body_v = mot->body_v + body_spring_perp * bike_relative_angular_velocity;
    }

    /*
    rigidbody_movement( &pmot->bike, Ftest2+Ftest4-Vect2j*pmot->bike.mass*Gravity,
                    Mtest2+Mtest4, dt, false );
    rigidbody_movement( &pmot->left_wheel, Fkerek2-Vect2j*pmot->left_wheel.mass*Gravity, Mkerek2,
    dt, true ); rigidbody_movement( &pmot->right_wheel,
    Fkerek4-Vect2j*pmot->right_wheel.mass*Gravity, Mkerek4,      dt, true
    );
    */

    switch (mot->gravity_direction) {
    case MotorGravity::Down:
        body_movement(mot, vect2(0.0, -1.0), i1, j1, dt); // vezeto
        rigidbody_movement(&mot->bike,
                           force_body_from_left_wheel + force_body_from_right_wheel -
                               Vect2j * mot->bike.mass * Gravity,
                           torque_body_from_left_wheel + torque_body_from_right_wheel, dt, false);
        rigidbody_movement(&mot->left_wheel,
                           force_left_wheel - Vect2j * mot->left_wheel.mass * Gravity,
                           torque_left_wheel, dt, true);
        rigidbody_movement(&mot->right_wheel,
                           force_right_wheel - Vect2j * mot->right_wheel.mass * Gravity,
                           torque_right_wheel, dt, true);
        break;
    case MotorGravity::Up:
        body_movement(mot, vect2(0.0, 1.0), i1, j1, dt); // vezeto
        rigidbody_movement(&mot->bike,
                           force_body_from_left_wheel + force_body_from_right_wheel +
                               Vect2j * mot->bike.mass * Gravity,
                           torque_body_from_left_wheel + torque_body_from_right_wheel, dt, false);
        rigidbody_movement(&mot->left_wheel,
                           force_left_wheel + Vect2j * mot->left_wheel.mass * Gravity,
                           torque_left_wheel, dt, true);
        rigidbody_movement(&mot->right_wheel,
                           force_right_wheel + Vect2j * mot->right_wheel.mass * Gravity,
                           torque_right_wheel, dt, true);
        break;
    case MotorGravity::Left:
        body_movement(mot, vect2(-1.0, 0.0), i1, j1, dt); // vezeto
        rigidbody_movement(&mot->bike,
                           force_body_from_left_wheel + force_body_from_right_wheel -
                               Vect2i * mot->bike.mass * Gravity,
                           torque_body_from_left_wheel + torque_body_from_right_wheel, dt, false);
        rigidbody_movement(&mot->left_wheel,
                           force_left_wheel - Vect2i * mot->left_wheel.mass * Gravity,
                           torque_left_wheel, dt, true);
        rigidbody_movement(&mot->right_wheel,
                           force_right_wheel - Vect2i * mot->right_wheel.mass * Gravity,
                           torque_right_wheel, dt, true);
        break;
    case MotorGravity::Right:
        body_movement(mot, vect2(1.0, 0.0), i1, j1, dt); // vezeto
        rigidbody_movement(&mot->bike,
                           force_body_from_left_wheel + force_body_from_right_wheel +
                               Vect2i * mot->bike.mass * Gravity,
                           torque_body_from_left_wheel + torque_body_from_right_wheel, dt, false);
        rigidbody_movement(&mot->left_wheel,
                           force_left_wheel + Vect2i * mot->left_wheel.mass * Gravity,
                           torque_left_wheel, dt, true);
        rigidbody_movement(&mot->right_wheel,
                           force_right_wheel + Vect2i * mot->right_wheel.mass * Gravity,
                           torque_right_wheel, dt, true);
        break;
    }

    set_head_position(mot);
}

// 0-meghalt, 2-semmi kulonos
int check_object_collision(motorst* mot) {
    vect2 point1, point2;
    if (get_two_anchor_points(mot->head_r, HeadRadius, &point1, &point2)) {
        return 0;
    }

    // Objektumokkal utkozesvizsgalat:
    int again = 1;
    while (again) { // Ha nem volt kaja kilepunk, kulonben vegtelen ciklus
        again = 0;
        int object_indices[3];
        object_indices[0] = get_touching_object(mot->left_wheel.r, mot->left_wheel.radius);
        object_indices[1] = get_touching_object(mot->right_wheel.r, mot->right_wheel.radius);
        object_indices[2] = get_touching_object(mot->head_r, HeadRadius);
        for (int i = 0; i < 3; i++) {
            if (object_indices[i] >= 0) {
                add_event_buffer(WavEvent::None, 0.0, object_indices[i]);
                object* obj = Ptop->get_object(object_indices[i]);
                if (obj->type == object::Type::Food) {
                    obj->active = false;
                    again = 1; // Hatha van meg tobb kaja is
                }
            }
        }
    }

    return 2;
}

static double FRICTION_VOLUME_COEFFICIENT = 1.0 / 1.0;

static void update_friction_volume(motorst* mot, vect2 wheel_displacement,
                                   vect2 wheel_relative_velocity) {
    vect2 j1(cos(mot->bike.rotation - 0.5 * PI), sin(mot->bike.rotation - 0.5 * PI));
    double upwards_displacement = j1 * wheel_displacement;
    double upwards_velocity = j1 * wheel_relative_velocity;
    if (upwards_displacement <= 0 || upwards_velocity <= 0) {
        return;
    }
    double friction_volume = upwards_displacement * upwards_velocity * FRICTION_VOLUME_COEFFICIENT;
    if (friction_volume > MaxFrictionVolume) {
        MaxFrictionVolume = friction_volume;
    }
}

double get_bike_friction_volume(void) { return MaxFrictionVolume; }
