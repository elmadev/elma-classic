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

static double MaxFrictionVolume = 0.0;

// Modulo wheel to be between 0 and 2Pi (kind of), only when we aren't braking
static void clamp_wheel_rotation(double* rotation) {
    if (*rotation < -PI) {
        *rotation += 2 * PI;
    }
    if (*rotation > PI) {
        *rotation -= 2 * PI;
    }
}

static void calculate_wheel_forces(motorst* mot, rigidbody* rb, vect2 i1, vect2 j1, double wheel_dx,
                                   double wheel_dy, vect2* force_wheel, vect2* force_bike,
                                   double* torque_bike, double* torque_wheel) {
    // (1) Spring Tension Force
    // Calculate the wheel's neutral position, based on the bike rotation
    vect2 wheel_neutral_position_relative = i1 * wheel_dx + j1 * wheel_dy;
    vect2 wheel_neutral_position_absolute = wheel_neutral_position_relative + mot->bike.r;

    // Get the wheel relative displacement
    vect2 wheel_displacement = wheel_neutral_position_absolute - rb->r;
    // Calculate the wheel spring tension force
    double tension_force_wheel_parallel = 0.0;
    double tension_force_wheel_orthogonal = 0.0;
    if (wheel_displacement.x < -0.0001 || wheel_displacement.x > 0.0001 ||
        wheel_displacement.y < -0.0001 || wheel_displacement.y > 0.0001) {
        // Get unit vectors representing neutral bike->wheel spring
        double neutral_spring_length = wheel_neutral_position_relative.length();
        vect2 neutral_spring_unit = wheel_neutral_position_relative * (1.0 / neutral_spring_length);
        vect2 neutral_spring_unit_ortho = rotate_90deg(neutral_spring_unit);

        // Get the parallel spring tension force and orthogonal spring tension force
        tension_force_wheel_parallel =
            (wheel_displacement * neutral_spring_unit) * SpringTensionCoefficient;
        tension_force_wheel_orthogonal =
            (wheel_displacement * neutral_spring_unit_ortho) * SpringTensionCoefficient;

        // Add the two forces together to make the spring tension force
        *force_wheel = tension_force_wheel_parallel * neutral_spring_unit +
                       tension_force_wheel_orthogonal * neutral_spring_unit_ortho;

        // Opposite force on bike
        *force_bike = Vect2null - *force_wheel;

        // Only the orthogonal spring tension force causes a torque on the bike
        *torque_bike = -tension_force_wheel_orthogonal * neutral_spring_length;
    } else {
        // No signficant wheel tension
        *force_wheel = vect2();
        *force_bike = vect2();
        *torque_bike = 0.0;
    }

    // (2) Spring Damping Force
    // Get vectors and unit vectors representing actual bike->wheel spring
    vect2 current_spring = rb->r - mot->bike.r;
    double current_spring_length = current_spring.length();
    double current_spring_length_reciprocal = 1.0 / current_spring_length;
    vect2 current_spring_unit = current_spring * current_spring_length_reciprocal;
    vect2 current_spring_perp = rotate_90deg(current_spring);
    vect2 current_spring_unit_perp = rotate_90deg(current_spring_unit);

    // The "neutral" wheel velocity is the bike velocity + angular velocity
    // Angular velocity in linear units is the bike's rotation times the length of the spring
    // The difference in velocity is thus bike_angualar_velocity + bike_velocity - wheel_velocity
    vect2 relative_velocity =
        (current_spring_perp * mot->bike.angular_velocity + mot->bike.v) - rb->v;

    // Split the velocity into parallel and orthogonal velocities
    double relative_velocity_parallel = relative_velocity * current_spring_unit;
    double relative_velocity_orthogonal = relative_velocity * current_spring_unit_perp;

    // Calculate the spring damping force
    vect2 damping_force_wheel_parallel =
        (relative_velocity_parallel * SpringResistanceCoefficient) * current_spring_unit;
    vect2 damping_force_wheel_orthogonal =
        (relative_velocity_orthogonal * SpringResistanceCoefficient) * current_spring_unit_perp;

    // (3) Gas/Brake force
    // Torque from gas/brake is applied to the spoke of the wheel. Assume a gear in the middle of
    // the bike that connects directly to the spoke of the wheel. The gear changes in radius when
    // the wheel gets closer or farther from the bike. The amount of force required to produce a
    // constant torque (from gas or brake) depends on the size of the gear. As the gear becomes
    // smaller and smaller, it takes more and more force to apply the same torque. This is where bug
    // bounces come from, because it takes an infinite force to apply a torque to an infinitely
    // small gear.
    vect2 gasbrake_force_body =
        current_spring_unit_perp * (*torque_wheel * current_spring_length_reciprocal);

    // Add up all the above forces
    *force_wheel = *force_wheel + damping_force_wheel_parallel + damping_force_wheel_orthogonal -
                   gasbrake_force_body;
    *torque_bike += -(damping_force_wheel_orthogonal * current_spring_perp);
    *force_bike = *force_bike - damping_force_wheel_parallel - damping_force_wheel_orthogonal +
                  gasbrake_force_body;

    // Calculate bike friction volume (squeak sound)
    update_friction_volume(mot, wheel_displacement, relative_velocity);
}

void reset_motor_forces(motorst* mot) {
    mot->prev_brake = false;
    mot->left_wheel_brake_rotation = 0.0;
    mot->right_wheel_brake_rotation = 0.0;
    mot->volting_right = false;
    mot->volting_left = false;
    mot->right_volt_time = -1.0;
    mot->left_volt_time = -1.0;
    mot->angular_velocity_pre_right_volt = -1.0;
    mot->angular_velocity_pre_left_volt = -1.0;
}

void set_head_position(motorst* mot) {
    vect2 i1(cos(mot->bike.rotation), sin(mot->bike.rotation));
    vect2 j1 = rotate_90deg(i1);

    // Head is always at a fixed position relative to body
    if (mot->flipped_bike) {
        mot->head_r = mot->body_r + i1 * 0.09 + j1 * 0.63;
    } else {
        mot->head_r = mot->body_r - i1 * 0.09 + j1 * 0.63;
    }
}

void simulate_bike_physics(motorst* mot, double time, double dt, bool gas, bool brake,
                           bool right_volt, bool left_volt) {
    // Initialize bike friction volume (squeak sound)
    MaxFrictionVolume = 0;

    // Unit vectors: i1 points to the right of the bike, and j1 is top of the bike
    vect2 i1(cos(mot->bike.rotation), sin(mot->bike.rotation));
    vect2 j1 = rotate_90deg(i1);

    // If you just pressed brake, save the wheel rotation so we can stretch the wheel rotation
    if (!mot->prev_brake && brake) {
        mot->left_wheel_brake_rotation = mot->left_wheel.rotation - mot->bike.rotation;
        mot->right_wheel_brake_rotation = mot->right_wheel.rotation - mot->bike.rotation;
    }
    mot->prev_brake = brake;

    // Create torque from pressing gas, up to a max rotation
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
        // If we are braking, create torque from two sources
        // 1) Position - Wheel Angular Spring Tension Force (i.e. hang stretch)
        // 2) Velocity - Wheel Angular Velocity Damping Force (i.e. slow down rotating wheel)
        // This are basically angular equivalents to the calculations in calculate_wheel_forces
        double ANGULAR_SPRING_TENSION_COEFFICIENT = 1000.0;
        double ANGULAR_SPRING_RESISTANCE_COEFFICIENT = 100.0;

        // Get the rotational difference between the wheel now and when brake was applied
        double relative_rotation =
            mot->left_wheel.rotation - (mot->bike.rotation + mot->left_wheel_brake_rotation);
        // Get the wheel rotational velocity (relative to bike)
        double relative_angular_velocity =
            mot->left_wheel.angular_velocity - mot->bike.angular_velocity;
        // Put the two sources of torque together
        torque_left_wheel = -ANGULAR_SPRING_TENSION_COEFFICIENT * relative_rotation -
                            ANGULAR_SPRING_RESISTANCE_COEFFICIENT * relative_angular_velocity;

        // Do the same for the other wheel
        relative_rotation =
            mot->right_wheel.rotation - (mot->bike.rotation + mot->right_wheel_brake_rotation);
        relative_angular_velocity = mot->right_wheel.angular_velocity - mot->bike.angular_velocity;
        torque_right_wheel = -ANGULAR_SPRING_TENSION_COEFFICIENT * relative_rotation -
                             ANGULAR_SPRING_RESISTANCE_COEFFICIENT * relative_angular_velocity;
    } else {
        // If we aren't braking, we can bound the wheel rotations between 0-2Pi
        clamp_wheel_rotation(&mot->left_wheel.rotation);
        clamp_wheel_rotation(&mot->right_wheel.rotation);
    }

    // Calculate the forces related to one wheel
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

    // When we start a volt, save the angular velocity right before the new volt is applied
    double prevolt_angular_velocity;
    if (right_volt || left_volt) {
        prevolt_angular_velocity = mot->bike.angular_velocity;
    }

    constexpr double VOLT_ANGULAR_VELOCITY = 12.0;
    constexpr double COUNTERVOLT_ANGULAR_VELOCITY = 3.0;

    // End a Right Volt if we are right-volting and 25% of the volt delay has passed
    // (or if somehow we managed to send a new volt request by dying and respawning in flagtag)
    if (mot->volting_right &&
        (right_volt || left_volt || time > mot->right_volt_time + VoltDelay * 0.25)) {
        // Remove the rotation obtained at the beginning of the volt
        mot->bike.angular_velocity += VOLT_ANGULAR_VELOCITY;
        // We need to be spinning at least as fast as before the start of the volt!
        // If we've slowed down, magically increase the spin speed to match the prevolt speed!
        // This is basically a mechanism where you can "spend" the angular momentum you got at the
        // start of a volt and convert it into wheel motion or linear bike motion.
        // At the end of the volt, you "lose" all the unspent momentum
        // You can theoretically slow down infinitely, so technically you can generate an unlimited
        // amount of free momentum here.
        // Alovolt bug: Since the bike didn't get its rotational velocity during the volt,
        // we generally generate VOLT_ANGULAR_VELOCITY worth of free momentum
        if (mot->bike.angular_velocity > mot->angular_velocity_pre_right_volt) {
            mot->bike.angular_velocity = mot->angular_velocity_pre_right_volt;
        }
        // If we are volting against the current rotation of the bike, then give a small
        // bonus to the angular velocity in the form of free momentum, capped to an
        // amount that will neutralize the rotation
        if (mot->bike.angular_velocity > 0.0) {
            mot->bike.angular_velocity -= COUNTERVOLT_ANGULAR_VELOCITY;
            if (mot->bike.angular_velocity < 0.0) {
                mot->bike.angular_velocity = 0.0;
            }
        }
        // Reset vars
        mot->volting_right = false;
        mot->angular_velocity_pre_right_volt = -1.0;
        mot->right_volt_time = -1.0;
    }
    // End a left volt (same as above with different alovolt implications)
    if (mot->volting_left &&
        (right_volt || left_volt || time > mot->left_volt_time + VoltDelay * 0.25)) {
        mot->bike.angular_velocity -= VOLT_ANGULAR_VELOCITY;
        // Alovolt penalty: You can't speed up during the alovolt because the
        // extra momentum will be removed by this check
        // e.g. your left wheel is bumped upwards and you gain spin speed
        if (mot->bike.angular_velocity < mot->angular_velocity_pre_left_volt) {
            mot->bike.angular_velocity = mot->angular_velocity_pre_left_volt;
        }
        // Alovolt penalty: lose COUNTERVOLT_ANGULAR_VELOCITY if the bike is already
        // spinning clockwise. That's why you should rotate slightly counterclockwise before doing
        // an alovolt at the start of Bumpy Journey.
        if (mot->bike.angular_velocity < 0.0) {
            mot->bike.angular_velocity += COUNTERVOLT_ANGULAR_VELOCITY;
            if (mot->bike.angular_velocity > 0.0) {
                mot->bike.angular_velocity = 0.0;
            }
        }
        mot->volting_left = false;
        mot->angular_velocity_pre_left_volt = -1.0;
        mot->left_volt_time = -1.0;
    }

    // Start a new Right Volt
    if (right_volt) {
        // Temporarily give free angular velocity
        // (we will take it all back at the end of the volt, see above)
        mot->volting_right = true;
        mot->angular_velocity_pre_right_volt = mot->bike.angular_velocity;
        mot->right_volt_time = time;
        mot->bike.angular_velocity -= VOLT_ANGULAR_VELOCITY;
    }
    // Start a new Left Volt (same thing as right volt)
    if (left_volt) {
        // In an alovolt, right and left volt cancel each other out
        mot->volting_left = true;
        mot->angular_velocity_pre_left_volt = mot->bike.angular_velocity;
        mot->left_volt_time = time;
        mot->bike.angular_velocity += VOLT_ANGULAR_VELOCITY;
    }

    // Based on the direction of the volt, apply a horizontal velocity to the body
    // in the direction of the volt, scaling with the body's height
    // This makes the kuski lean into the volt instead of flopping backwards
    if (right_volt || left_volt) {
        double bike_relative_angular_velocity =
            mot->bike.angular_velocity - prevolt_angular_velocity;
        vect2 body_spring_perp = rotate_90deg(mot->body_r - mot->bike.r);
        mot->body_v = mot->body_v + body_spring_perp * bike_relative_angular_velocity;
    }

    // Update the body position and bike/wheel positions based on gravity
    vect2 direction;
    switch (mot->gravity_direction) {
    case MotorGravity::Down:
        direction = vect2(0.0, -1.0);
        break;
    case MotorGravity::Up:
        direction = vect2(0.0, 1.0);
        break;
    case MotorGravity::Left:
        direction = vect2(-1.0, 0.0);
        break;
    case MotorGravity::Right:
        direction = vect2(1.0, 0.0);
        break;
    }
    body_movement(mot, direction, i1, j1, dt);
    rigidbody_movement(&mot->bike,
                       force_body_from_left_wheel + force_body_from_right_wheel +
                           direction * mot->bike.mass * Gravity,
                       torque_body_from_left_wheel + torque_body_from_right_wheel, dt, false);
    rigidbody_movement(&mot->left_wheel,
                       force_left_wheel + direction * mot->left_wheel.mass * Gravity,
                       torque_left_wheel, dt, true);
    rigidbody_movement(&mot->right_wheel,
                       force_right_wheel + direction * mot->right_wheel.mass * Gravity,
                       torque_right_wheel, dt, true);

    // Lastly, calculate the head position based on the body position
    set_head_position(mot);
}

BikeState check_object_collision(motorst* mot) {
    // If the head touches any line, we are dead
    vect2 point1;
    vect2 point2;
    if (get_two_anchor_points(mot->head_r, HeadRadius, &point1, &point2)) {
        return BikeState::Dead;
    }

    // Collect all the object event interactions (but don't parse the interactions just yet)
    bool again = true;
    while (again) {
        again = false;
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
                    again = true;
                }
            }
        }
    }
    return BikeState::Normal;
}

// Calculate bike squeak sound. Save the loudest squeak sound (since we have up to 4 wheels)
static void update_friction_volume(motorst* mot, vect2 wheel_displacement,
                                   vect2 wheel_relative_velocity) {
    // Calculate the wheel position based on the bike rotation
    vect2 j1(cos(mot->bike.rotation - 0.5 * PI), sin(mot->bike.rotation - 0.5 * PI));
    double upwards_displacement = j1 * wheel_displacement;
    double upwards_velocity = j1 * wheel_relative_velocity;
    // Only make sound if the wheel is displaced upwards and the wheel velocity is upwards
    if (upwards_displacement <= 0.0 || upwards_velocity <= 0.0) {
        return;
    }

    double friction_volume = upwards_displacement * upwards_velocity;
    if (friction_volume > MaxFrictionVolume) {
        MaxFrictionVolume = friction_volume;
    }
}

double get_bike_friction_volume() { return MaxFrictionVolume; }
