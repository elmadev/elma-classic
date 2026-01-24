#include "physics_move.h"
#include "physics_collision.h"
#include "physics_init.h"
#include "recorder.h"
#include "sound_engine.h"
#include <cmath>

// Push the wheel out from the ground so it is standing on the anchor point
static void move_wheel_out_of_ground(rigidbody* rb, vect2* point) {
    double length = (rb->r - *point).length();
    vect2 n = (rb->r - *point) * (1.0 / length);
    if (length < rb->radius - WheelDeformationLength) {
        rb->r = rb->r + n * (rb->radius - WheelDeformationLength - length);
    }
}

// Minimum speed loss to trigger a bump sound effect
constexpr double BumpThreshold = 1.5;

// Handle collision between a wheel and one anchor point
// Return true if there is collision
// Delete all of the velocity towards the point and keep velocity perpendicular to the point
static bool simulate_anchor_point_collision(rigidbody* rb, vect2* point, vect2 force) {
    // Return false if no collision
    double length = (rb->r - *point).length();
    vect2 n = (rb->r - *point) * (1.0 / length);
    if (n * rb->v > -GroundEscapeVelocity && n * force > 0) {
        return false;
    }
    // Remove the velocity component that is parallel to the axis between wheel and point
    vect2 deleted_velocity = n * rb->v * n;
    rb->v = rb->v - deleted_velocity;
    // Make a "bump" sound effect if enough velocity deleted
    double bump_magnitude = deleted_velocity.length();
    if (bump_magnitude > BumpThreshold) {
        bump_magnitude = bump_magnitude / 0.8 * 0.1;
        if (bump_magnitude >= 0.99) {
            bump_magnitude = 0.99;
        }
        add_event_buffer(WavEvent::Bump, bump_magnitude, -1);
    }
    return true;
}

// Check to see whether the wheel is stuck
// Assuming that the wheel hits point point2
// Assuming that the wheel is rolling on point2, do the forces on the wheel push towards point1 or
// away?
// If the wheel is pushed towards point1, return 1 (stuck wheel).
// The reason why this causes stuck in Across is we ignore existing velocity and only consider
// acceleration/torque
static bool valid_anchor_points_old(vect2 point1, vect2 point2, rigidbody* rb, vect2 force,
                                    double torque) {
    // Get the unit vectors
    double length = (rb->r - point2).length();
    vect2 n = (rb->r - point2) * (1.0 / length);
    vect2 n90 = rotate_90deg(n);

    // Convert linear force into torque and get the net torque (gas/brake torque + linear force
    // torque)
    double total_torque = torque + length * n90 * force;
    // Check if torque is in same direction as point1 or not
    return !((point1 - point2) * n90 * total_torque < 0);
}

// Check to see whether the wheel is stuck, Elma-exclusive
// Assuming that the wheel hits point point2
// Assuming that the wheel is rolling on point2, does the wheel slide towards point1 or away?
static bool valid_anchor_points_new(vect2 point1, vect2 point2, rigidbody* rb) {
    // Get the unit vectors
    double length = (rb->r - point2).length();
    vect2 n = (rb->r - point2) * (1.0 / length);
    vect2 n90 = rotate_90deg(n);

    // Get the velocity orthogonal to the point of collision + angular velocity
    // The units here don't really match (m/s + rad/s) so it's kind of flawed
    double speed_direction = rb->angular_velocity + length * n90 * rb->v;
    // Check if the velocity is in the same direction as point1
    return !((point1 - point2) * n90 * speed_direction < 0);
}

// Handle wheel/bike movement
// do_collision = true if solid object (i.e. wheels and not bike)
void rigidbody_movement(rigidbody* rb, vect2 force, double torque, double dt, bool do_collision) {
    int anchor_point_count = 0;
    vect2 point1;
    vect2 point2;

    // Get up to two points of collision between wheel and polygons
    if (do_collision) {
        anchor_point_count = get_two_anchor_points(rb->r, rb->radius, &point1, &point2);
    }

    // Move the wheel out of the ground
    if (anchor_point_count > 0) {
        move_wheel_out_of_ground(rb, &point1);
    }
    if (anchor_point_count > 1) {
        move_wheel_out_of_ground(rb, &point2);
    }

    // If we have two points of collision, check whether the wheel is stuck in the corner
    // When the wheel is moving fast, try to discard one of the collision points if you are moving
    // away from it
    if (anchor_point_count == 2 && rb->v.length() > 1.0) {
        // Elma-exclusive bike-stuck fixes:
        if (!valid_anchor_points_new(point1, point2, rb)) {
            anchor_point_count = 1;
            point1 = point2;
        } else {
            if (!valid_anchor_points_new(point2, point1, rb)) {
                anchor_point_count = 1;
            }
        }
    }
    // When the wheel is moving slow, try to discard one of the collision points if the force pushes
    // away from it
    if (anchor_point_count == 2 && rb->v.length() < 1.0) {
        // Original across bike-stuck (previously always checked, regardless of velocity)
        if (!valid_anchor_points_old(point1, point2, rb, force, torque)) {
            anchor_point_count = 1;
            point1 = point2;
        } else {
            if (!valid_anchor_points_old(point2, point1, rb, force, torque)) {
                anchor_point_count = 1;
            }
        }
    }

    // Assuming we have only one point of collision, check to see if we actually collide with that
    // point! Discard the point if there's no collision
    if (anchor_point_count == 2) {
        if (!simulate_anchor_point_collision(rb, &point2, force)) {
            anchor_point_count = 1;
        }
    }
    if (anchor_point_count >= 1) {
        if (!simulate_anchor_point_collision(rb, &point1, force)) {
            if (anchor_point_count == 2) {
                anchor_point_count = 1;
                point1 = point2;
            } else {
                anchor_point_count = 0;
            }
        }
    }

    // No collision, do normal physics
    if (anchor_point_count == 0) {
        // Angular position, velocity and acceleration
        double angular_acceleration = torque / rb->inertia;
        rb->angular_velocity += angular_acceleration * dt;
        rb->rotation += rb->angular_velocity * dt;
        // Linear position, velocity and acceleration
        vect2 a = force * (1.0 / rb->mass);
        rb->v = rb->v + a * dt;
        rb->r = rb->r + rb->v * dt;
        return;
    }
    // We collide with two points, so we are stuck
    if (anchor_point_count == 2) {
        rb->v = Vect2null;
        rb->angular_velocity = 0;
        return;
    }

    // We collide with one point, so roll the wheel on the ground
    // We need to roll perpendicular to the point of contact, so get the corresponding unit vector
    double length = (rb->r - point1).length();
    vect2 n = (rb->r - point1) * (1.0 / length);
    vect2 n90 = rotate_90deg(n);
    // Take our linear velocity - since we are rolling, convert it into equivalent angular velocity
    rb->angular_velocity = rb->v * n90 * (1.0 / rb->radius);
    torque += force * n90 * rb->radius;
    // Calculate the moment of inertia for rolling the wheel on the ground (as opposed to rolling
    // wheel from the center spoke)
    // https://en.wikipedia.org/wiki/Parallel_axis_theorem
    // Moment(radius) = Moment(0) + mass*distance*distance
    double inertia_edge = rb->inertia + rb->mass * length * length;
    // Calculate our angular acceleration
    double angular_acceleration = torque / inertia_edge;

    // Calculate angular velocity and position
    rb->angular_velocity += angular_acceleration * dt;
    rb->rotation += rb->angular_velocity * dt;
    // Calculate linear velocity and position based on a rolling wheel
    rb->v = rb->angular_velocity * rb->radius * n90;
    rb->r = rb->r + rb->v * dt;
}

// Teleport the body to be within the specified boundaries
// See the image file body_boundaries.png in docs/
static void body_boundaries(motorst* mot, vect2 i, vect2 j) {
    // Flip the body temporarily if the bike is turned
    double body_x, body_y;
    if (mot->flipped_bike) {
        body_x = i * (mot->bike.r - mot->body_r);
        body_y = j * (mot->body_r - mot->bike.r);
    } else {
        body_x = i * (mot->body_r - mot->bike.r);
        body_y = j * (mot->body_r - mot->bike.r);
    }
    vect2 body_r(body_x, body_y);

    // Restrict bottom with a diagonal line
    static const vect2 LINE_POINT(-0.35, 0.13);
    static const vect2 LINE_SLOPE(0.14 - (-0.35), 0.36 - (0.13));
    static const vect2 LINE_SLOPE_ORTHO(-LINE_SLOPE.y, LINE_SLOPE.x);
    static const vect2 LINE_SLOPE_ORTHO_UNIT = unit_vector(LINE_SLOPE_ORTHO);
    if ((body_r - LINE_POINT) * LINE_SLOPE_ORTHO_UNIT < 0.0) {
        double distance = (body_r - LINE_POINT) * LINE_SLOPE_ORTHO_UNIT;
        body_r = body_r - LINE_SLOPE_ORTHO_UNIT * distance;
    }

    // Restrict top
    static const double ELLIPSE_HEIGHT = 0.48;
    if (body_r.y > ELLIPSE_HEIGHT) {
        body_r.y = ELLIPSE_HEIGHT;
    }
    // Restrict front
    if (body_r.x < -0.5) {
        body_r.x = -0.5;
    }
    // Restrict back (not very important, see next restriction)
    static const double ELLIPSE_WIDTH = 0.26;
    if (body_r.x > ELLIPSE_WIDTH) {
        body_r.x = ELLIPSE_WIDTH;
    }

    // Restrict back-top corner with a curved line (ellipse)
    // Ellipse equation
    // (x/0.26)^2 + (y/0.48)^2 = 1.0
    // or alternatively what we use below is:
    // (x*0.48/0.26)^2 + y^2 = 0.48^2
    // (x*a)^2 + y^2 = r^2
    static const double ELLIPSE_R2 = ELLIPSE_HEIGHT * ELLIPSE_HEIGHT;
    static const double ELLIPSE_R = sqrt(ELLIPSE_R2);
    if (body_r.x > 0 && body_r.y > 0) {
        static const double ELLIPSE_A2 =
            (ELLIPSE_HEIGHT / ELLIPSE_WIDTH) * (ELLIPSE_HEIGHT / ELLIPSE_WIDTH);
        double distance2 = body_r.x * body_r.x * ELLIPSE_A2 + body_r.y * body_r.y;
        if (distance2 > ELLIPSE_R2) {
            double distance = sqrt(distance2);
            double displacement_ratio = ELLIPSE_R / distance;
            body_r.x *= displacement_ratio;
            body_r.y *= displacement_ratio;
        }
    }

    // Unflip body if we flipped it at the start
    if (mot->flipped_bike) {
        mot->body_r = j * body_r.y - i * body_r.x + mot->bike.r;
    } else {
        mot->body_r = i * body_r.x + j * body_r.y + mot->bike.r;
    }
}

// Adjust the body position
void body_movement(motorst* mot, vect2 gravity, vect2 i, vect2 j, double dt) {
    // Teleport the body to be within acceptable bounds
    body_boundaries(mot, i, j);

    // Calculate the relative body offset compared to neutral position
    vect2 neutral_body_r = mot->bike.r + j * BodyDY;
    vect2 delta_body_r = neutral_body_r - mot->body_r;

    // Acting as if the body is a spring, calculate the spring force
    // Force = Stretch*Tension
    // where Tension is 5x the tension of the wheel spring
    double spring_length = delta_body_r.length();
    if (spring_length < 0.0000001) {
        spring_length = 0.0000001;
    }
    vect2 force_spring_unit = delta_body_r * (1.0 / spring_length);
    double force_spring_scalar = spring_length * SpringTensionCoefficient * 5.0;
    vect2 force_spring = force_spring_unit * force_spring_scalar;

    // Acting as if the body is a spring, calculate the dampening force
    // Dampening force is proportional to velocity
    // Force = Velocity*Resistance
    // Where Resistance is 3x the resistance of the wheel resistance
    // It's a bit more complicated to calculate the relative velocity of the body
    // RelativeVelocity = BodyVel - BikeVel
    // However, the bike velocity has linear motion as well as angular motion
    // BikeVelocity = AngularVel(body_length) - LinearVel
    // AngularVel = radius(m)*rotational_velocity(rad/s)
    vect2 body_length_ortho = rotate_90deg(mot->body_r - mot->bike.r);
    vect2 neutral_v = body_length_ortho * mot->bike.angular_velocity + mot->bike.v;
    vect2 relative_v = mot->body_v - neutral_v;
    vect2 force_damping = relative_v * SpringResistanceCoefficient * 3.0;

    // Calculate gravity and then get the net force
    vect2 force_total = force_spring - force_damping + gravity * mot->bike.mass * Gravity;

    // Update velocity and position
    vect2 a = force_total * (1.0 / mot->bike.mass);
    mot->body_v = mot->body_v + a * dt;
    mot->body_r = mot->body_r + mot->body_v * dt;
}
