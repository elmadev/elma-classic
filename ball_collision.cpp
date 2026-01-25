#include "ball_collision.h"
#include "ball.h"
#include "main.h"
#include "M_PIC.H"
#include <cmath>

bool WallsDisabled = false;

// Update ball keyframe time to current time
static void update_keyframe(ball* ball, double time) {
    ball->keyframe_r = ball->keyframe_r + ((time - ball->keyframe_time) * ball->v);
    ball->keyframe_rotation =
        ball->keyframe_rotation + ((time - ball->keyframe_time) * ball->angular_velocity);
    ball->keyframe_time = time;
}

// Update keyframe time of one of the two balls so that the two balls are synchronized
static void synchronize_keyframes(ball* ball1, ball* ball2) {
    if (ball1->keyframe_time < ball2->keyframe_time) {
        update_keyframe(ball1, ball2->keyframe_time);
    } else {
        update_keyframe(ball2, ball1->keyframe_time);
    }
}

// Get collision time between two balls
double ball_ball_collision_time(ball* ball1, ball* ball2) {
    synchronize_keyframes(ball1, ball2);

    // clang-format off
    // Get the collision time between two balls:
    // abs(ball1.keyframe_r + ball1.v*t - ball2.keyframe_r - ball2.v*t) = (ball1.radius + ball2.radius)
    // abs((ball1.keyframe_r - ball2.keyframe_r) + t*(ball1.v - ball2.v)) = (ball1.radius + ball2.radius)
    // abs(r + t*v) = radius
    // (r + t*v)^2 = radius^2
    // v^2*t^2 + 2*r*v*t + r^2 - radius^2 = 0
    // Then we solve the quadratic equation
    // (-b +- sqrt(b^2 - 4ac))/2a
    // One last caveat: r is ball2 - ball1 instead of ball1 - ball2
    // To compensate for this mistake, we multiply b by -2.0 instead of 2.0
    // clang-format on
    vect2 v = ball1->v - ball2->v;
    vect2 r = ball2->keyframe_r - ball1->keyframe_r;
    double a = v.x * v.x + v.y * v.y;
    if (a == 0) {
        // No relative movement, so no collision
        return NO_COLLISION_TIME;
    }
    double b = (-2) * (v.x * r.x + v.y * r.y);
    if (b >= 0) {
        // Balls moving away from each other, so no collision
        return NO_COLLISION_TIME;
    }
    double c =
        r.x * r.x + r.y * r.y - (ball1->radius + ball2->radius) * (ball1->radius + ball2->radius);
    double discriminant = b * b - 4 * a * c;
    if (discriminant <= 0) {
        // Balls will miss each other and not collide
        return NO_COLLISION_TIME;
    }
    double t = (-b - sqrt(discriminant)) / (2 * a);
    return (t + ball1->keyframe_time);
}

void simulate_ball_ball_collision(ball* ball1, ball* ball2, double time) {
    // Synchronize the two balls
    update_keyframe(ball1, time);
    update_keyframe(ball2, time);

    // Inertial calculations
    // Simulate as 2D disks (moment of inertia = 1/2 * m * r^2)
    double mass1 = ball1->radius * ball1->radius;
    double mass2 = ball2->radius * ball2->radius;
    double inertia1 = 0.5 * mass1 * ball1->radius * ball1->radius;
    double inertia2 = 0.5 * mass2 * ball2->radius * ball2->radius;

    // Center of mass frame of reference's velocity before collision
    double mass = mass1 + mass2;
    vect2 center_of_mass_velocity = ((mass1 / mass) * ball1->v) + ((mass2 / mass) * ball2->v);

    // Energy = AngularEnergy + KineticEnergy
    // AngularEnergy = 0.5*I*angular_velocity^2
    // KineticEnergy = 0.5*m*v^2
    // We ignore the 0.5 factor
    double initial_energy = mass1 * ball1->v * ball1->v + mass2 * ball2->v * ball2->v +
                            inertia1 * ball1->angular_velocity * ball1->angular_velocity +
                            inertia2 * ball2->angular_velocity * ball2->angular_velocity;

    // Collision direction (from ball2 to ball1) as a normal vector
    vect2 dr_normal =
        (1 / (ball1->radius + ball2->radius)) * (ball1->keyframe_r - ball2->keyframe_r);

    // Velocity component towards collision, multiplied by 2.
    // We use this as a reference value when updating velocities below
    vect2 vperp1 = 2 * (dr_normal * (ball1->v - center_of_mass_velocity)) * dr_normal;
    vect2 vperp2 = 2 * (dr_normal * (ball2->v - center_of_mass_velocity)) * dr_normal;

    // Plane of reflection as a normal vector
    vect2 dr = ball2->keyframe_r - ball1->keyframe_r;
    dr.normalize();
    vect2 collision_plane = rotate_90deg(dr);

    // Velocity component perpendicular to collision
    double vtang1 = collision_plane * ball1->v;
    double vtang2 = collision_plane * ball2->v;

    // Simulate a no-slip collision
    // When we calculate our effective mass, there is a bug here:
    // We use the effective mass of the system, but we accidentally use the first inertial factor
    // twice instead of using mass_I1 and mass_I2
    double angular_v1 = ball1->angular_velocity;
    double angular_v2 = ball2->angular_velocity;
    double rad1 = ball1->radius;
    double rad2 = ball2->radius;
    double mass_I1 = rad1 * rad1 / inertia1;
    double effective_mass = 1.0 / mass1 + 1.0 / mass2 + mass_I1 + mass_I1; // Bug
    double tangential_impulse =
        (vtang2 - vtang1 - angular_v1 * rad1 - angular_v2 * rad2) / effective_mass;
    tangential_impulse /= 2.0;

    // Update angular velocities
    ball1->angular_velocity += tangential_impulse * rad1 / inertia1;
    ball2->angular_velocity += tangential_impulse * rad2 / inertia2;

    // Alright, we did a relatively accurate simulation of the no-slip tangential impulse
    // (except we used the wrong inertial factor, whoops)
    // But since this steals energy from the system,
    // we don't know how to properly calculate the linear velocity!
    // Brute-force new velocity values that conserve energy
    int interation = 0;
    double step = 1.0;
    double adjust = 0.5;
    bool increase_energy = true;
    while (true) {
        // Keep adjusting velocity by a factor of "step" until we achieve conservation of energy
        interation++;
        vect2 new_v1 = ball1->v + tangential_impulse / mass1 * collision_plane - vperp1 * step;
        vect2 new_v2 = ball2->v - tangential_impulse / mass2 * collision_plane - vperp2 * step;

        double new_energy = mass1 * new_v1 * new_v1 + mass2 * new_v2 * new_v2 +
                            inertia1 * ball1->angular_velocity * ball1->angular_velocity +
                            inertia2 * ball2->angular_velocity * ball2->angular_velocity;

        if (increase_energy) {
            // Step 1: We initially increase velocity until we have too much energy
            if (new_energy > initial_energy) {
                increase_energy = false;
            } else {
                step *= 2.0;
                adjust *= 2.0;
            }
        } else {
            // Step 2: We fine-tune our energy to try and get a good approximation
            if (interation == 30) {
                ball1->v = new_v1;
                ball2->v = new_v2;
                if (fabs(new_energy - initial_energy) > 0.001) {
                    internal_error("fabs( new_energy - initial_energy ) > 0.001!");
                }
                break;
            }

            if (new_energy > initial_energy) {
                step -= adjust;
            } else {
                step += adjust;
            }
            adjust *= 0.5001;
        }
    }
}

static int wall_left() { return 0; }
static int wall_right() { return SCREEN_WIDTH; }
static int wall_top() { return 0; }
static int wall_bottom() { return SCREEN_HEIGHT; }

// Get collision time between a ball and wall
double ball_wall_collision_time(ball* ball, WallId wall) {
    if (WallsDisabled) {
        return NO_COLLISION_TIME;
    }
    switch (wall) {
    case WallId::Top:
        if (ball->v.y >= 0) {
            return NO_COLLISION_TIME;
        }
        if (ball->keyframe_r.y <= wall_top() + ball->radius) {
            return NO_COLLISION_TIME;
        }
        return ball->keyframe_time - ((ball->keyframe_r.y - wall_top() - ball->radius) / ball->v.y);
    case WallId::Bottom:
        if (ball->v.y <= 0) {
            return NO_COLLISION_TIME;
        }
        if (ball->keyframe_r.y >= wall_bottom() - ball->radius) {
            return NO_COLLISION_TIME;
        }
        return ball->keyframe_time +
               ((wall_bottom() - ball->radius - ball->keyframe_r.y) / ball->v.y);
    case WallId::Right:
        if (ball->v.x <= 0) {
            return NO_COLLISION_TIME;
        }
        if (ball->keyframe_r.x >= wall_right() - ball->radius) {
            return NO_COLLISION_TIME;
        }
        return ball->keyframe_time +
               ((wall_right() - ball->radius - ball->keyframe_r.x) / ball->v.x);
    case WallId::Left:
        if (ball->v.x >= 0) {
            return NO_COLLISION_TIME;
        }
        if (ball->keyframe_r.x <= wall_left() + ball->radius) {
            return NO_COLLISION_TIME;
        }
        return ball->keyframe_time -
               ((ball->keyframe_r.x - wall_left() - ball->radius) / ball->v.x);
    }
    return NO_COLLISION_TIME;
}

// Simulate a collision between a ball and wall
void simulate_ball_wall_collision(ball* ball, WallId wall, double time) {
    if (WallsDisabled) {
        return;
    }
    update_keyframe(ball, time);
    switch (wall) {
    case WallId::Top:
    case WallId::Bottom:
        ball->v.y = -ball->v.y;
        break;
    case WallId::Right:
    case WallId::Left:
        ball->v.x = -ball->v.x;
        break;
    }
}
