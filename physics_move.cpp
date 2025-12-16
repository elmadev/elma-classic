#include "ALL.H"

// Push the wheel out from the ground so it is standing on the anchor point
static void move_wheel_out_of_ground(rigidbody* rb, vekt2* point) {
    double length = abs(rb->r - *point);
    vekt2 n = (rb->r - *point) * (1.0 / length);
    if (length < rb->radius - WheelDeformationLength) {
        rb->r = rb->r + n * (rb->radius - WheelDeformationLength - length);
    }
}

// Minimum speed loss to trigger a bump sound effect
static double BumpThreshold = 1.5;

// Handle collision between a wheel and one anchor point
// Return 1 if there is collision, or 0 if no collision
// Delete all of the velocity towards the point and keep velocity perpendicular to the point
static int simulate_anchor_point_collision(rigidbody* rb, vekt2* point, vekt2 force) {
    // Return 0 if no collision
    double length = abs(rb->r - *point);
    vekt2 n = (rb->r - *point) * (1.0 / length);
    if (n * rb->v > -GroundEscapeVelocity && n * force > 0) {
        return 0;
    }
    // Remove the velocity component that is parallel to the axis between wheel and point
    vekt2 deleted_velocity = n * rb->v * n;
    rb->v = rb->v - deleted_velocity;
    // Make a "bump" sound effect if enough velocity deleted
    double bump_magnitude = abs(deleted_velocity);
    if (bump_magnitude > BumpThreshold) {
        bump_magnitude = bump_magnitude / 0.8 * 0.1;
        if (bump_magnitude >= 0.99) {
            bump_magnitude = 0.99;
        }
        startwavegyujto(WAV_UTODES, bump_magnitude, -1);
    }
    return 1;
}

// Check to see whether the wheel is stuck
// Assuming that the wheel hits point point2
// Assuming that the wheel is rolling on point2, do the forces on the wheel push towards point1 or
// away?
// If the wheel is pushed towards point1, return 1 (stuck wheel).
// The reason why this causes stuck in Across is we ignore existing velocity and only consider
// acceleration/torque
static int valid_anchor_points_old(vekt2 point1, vekt2 point2, rigidbody* rb, vekt2 force,
                                   double torque) {
    // Get the unit vectors
    double length = abs(rb->r - point2);
    vekt2 n = (rb->r - point2) * (1.0 / length);
    vekt2 n90 = forgatas90fokkal(n);

    // Convert linear force into torque and get the net torque (gas/brake torque + linear force
    // torque)
    double total_torque = torque + length * n90 * force;
    // Check if torque is in same direction as point1 or not
    if ((point1 - point2) * n90 * total_torque < 0) {
        return 0;
    } else {
        return 1;
    }
}

// Check to see whether the wheel is stuck, Elma-exclusive
// Assuming that the wheel hits point point2
// Assuming that the wheel is rolling on point2, does the wheel slide towards point1 or away?
static int valid_anchor_points_new(vekt2 point1, vekt2 point2, rigidbody* rb, vekt2 force,
                                   double torque) {
    // Get the unit vectors
    double length = abs(rb->r - point2);
    vekt2 n = (rb->r - point2) * (1.0 / length);
    vekt2 n90 = forgatas90fokkal(n);

    // Get the velocity orthogonal to the point of collision + angular velocity
    // The units here don't really match (m/s + rad/s) so it's kind of flawed
    double speed_direction = rb->angular_velocity + length * n90 * rb->v;
    // Check if the velocity is in the same direction as point1
    if ((point1 - point2) * n90 * speed_direction < 0) {
        return 0;
    } else {
        return 1;
    }
}

// Handle wheel/bike movement
// do_collision = true if solid object (i.e. wheels and not bike)
void rigidbody_movement(rigidbody* rb, vekt2 force, double torque, double dt, int do_collision) {
    int anchor_point_count = 0;
    vekt2 point1, point2;
    vonal *line1 = NULL, *line2 = NULL;

    // Get up to two points of collision between wheel and polygons
    if (do_collision) {
        anchor_point_count = talppontkereses(rb->r, rb->radius, &point1, &point2, &line1, &line2);
#ifdef TEST
        switch (anchor_point_count) {
        case 0:
            if (line1 || line2) {
                hiba("rigidbody_movement: 0 anchor_point_count assertion fail!");
            }
            break;
        case 1:
            if (!line1 || line2) {
                hiba("rigidbody_movement: 1 anchor_point_count assertion fail!");
            }
            break;
        case 2:
            if (!line1 || !line2) {
                hiba("rigidbody_movement: 2 anchor_point_count assertion fail!");
            }
            break;
        }
#endif
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
    if (anchor_point_count == 2 && abs(rb->v) > 1.0) {
        // Elma-exclusive bike-stuck fixes:
        if (!valid_anchor_points_new(point1, point2, rb, force, torque)) {
            anchor_point_count = 1;
            point1 = point2;
        } else {
            if (!valid_anchor_points_new(point2, point1, rb, force, torque)) {
                anchor_point_count = 1;
            }
        }
    }
    // When the wheel is moving slow, try to discard one of the collision points if the force pushes
    // away from it
    if (anchor_point_count == 2 && abs(rb->v) < 1.0) {
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
        vekt2 a = force * (1.0 / rb->mass);
        rb->v = rb->v + a * dt;
        rb->r = rb->r + rb->v * dt;
        return;
    }
    // We collide with two points, so we are stuck
    if (anchor_point_count == 2) {
        rb->v = Vekt2null;
        rb->angular_velocity = 0;
        return;
    }

    // We collide with one point, so roll the wheel on the ground
    // We need to roll perpendicular to the point of contact, so get the corresponding unit vector
    double length = abs(rb->r - point1);
    vekt2 n = (rb->r - point1) * (1.0 / length);
    vekt2 n90 = forgatas90fokkal(n);
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
    return;
}

// Teleport the body to be within the specified boundaries
// See the image file body_boundaries.png in docs/
static void body_boundaries(motorst* mot, vekt2 i, vekt2 j) {
    // Flip the body temporarily if the bike is turned
    double body_x, body_y;
    if (mot->flipped_bike) {
        body_x = i * (mot->bike.r - mot->body_r);
        body_y = j * (mot->body_r - mot->bike.r);
    } else {
        body_x = i * (mot->body_r - mot->bike.r);
        body_y = j * (mot->body_r - mot->bike.r);
    }
    vekt2 body_r(body_x, body_y);

    // Restrict bottom with a diagonal line
    static vekt2 LINE_POINT(-0.35, 0.13);
    static vekt2 LINE_SLOPE(0.14 - (-0.35), 0.36 - (0.13));
    static vekt2 LINE_SLOPE_ORTHO(-LINE_SLOPE.y, LINE_SLOPE.x);
    static vekt2 LINE_SLOPE_ORTHO_UNIT = egys(LINE_SLOPE_ORTHO);
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
void body_movement(motorst* mot, vekt2 gravity, vekt2 i, vekt2 j, double dt) {
    // Teleport the body to be within acceptable bounds
    body_boundaries(mot, i, j);

    // Calculate the relative body offset compared to neutral position
    vekt2 neutral_body_r = mot->bike.r + j * BodyDY;
    vekt2 delta_body_r = neutral_body_r - mot->body_r;

    // Acting as if the body is a spring, calculate the spring force
    // Force = Stretch*Tension
    // where Tension is 5x the tension of the wheel spring
    double spring_length = abs(delta_body_r);
    if (spring_length < 0.0000001) {
        spring_length = 0.0000001;
    }
    vekt2 force_spring_unit = delta_body_r * (1.0 / spring_length);
    double force_spring_scalar = spring_length * SpringTensionCoefficient * 5.0;
    vekt2 force_spring = force_spring_unit * force_spring_scalar;

    // Acting as if the body is a spring, calculate the dampening force
    // Dampening force is proportional to velocity
    // Force = Velocity*Resistance
    // Where Resistance is 3x the resistance of the wheel resistance
    // It's a bit more complicated to calculate the relative velocity of the body
    // RelativeVelocity = BodyVel - BikeVel
    // However, the bike velocity has linear motion as well as angular motion
    // BikeVelocity = AngularVel(body_length) - LinearVel
    // AngularVel = radius(m)*rotational_velocity(rad/s)
    vekt2 body_length_ortho = forgatas90fokkal(mot->body_r - mot->bike.r);
    vekt2 neutral_v = body_length_ortho * mot->bike.angular_velocity + mot->bike.v;
    vekt2 relative_v = mot->body_v - neutral_v;
    vekt2 force_damping = relative_v * SpringResistanceCoefficient * 3.0;

    // Calculate gravity and then get the net force
    vekt2 force_total = force_spring - force_damping + gravity * mot->bike.mass * Gravity;

    // Update velocity and position
    vekt2 a = force_total * (1.0 / mot->bike.mass);
    mot->body_v = mot->body_v + a * dt;
    mot->body_r = mot->body_r + mot->body_v * dt;
}
