#include "ball.h"
#include "ball_collision.h"
#include "ball_handler.h"

static WallId wall_id(int collider) { return static_cast<WallId>(collider - BallCount); }

// Returns the time of the next expected collision, as well as the ids of the two culprits
double next_collision_time(int* collider1, int* collider2) {
    double next_collision_time = NO_COLLISION_TIME;
    *collider1 = 0;
    *collider2 = 1;
    for (int i = 0; i < BallCount; i++) {
        // Check collision between two balls
        for (int j = i + 1; j < BallCount; j++) {
            // Update collision time if unknown
            if (CollisionTimeGrid[i][j] < -1) {
                CollisionTimeGrid[i][j] = ball_ball_collision_time(&Balls[i], &Balls[j]);
            }
            // Grab the earliest collision
            if (CollisionTimeGrid[i][j] < next_collision_time) {
                next_collision_time = CollisionTimeGrid[i][j];
                *collider1 = i;
                *collider2 = j;
            }
        }
        // Check collisions between ball and walls
        for (int j = BallCount; j < BallCount + 4; j++) {
            // Update collision time if unknown
            if (CollisionTimeGrid[i][j] < -1) {
                CollisionTimeGrid[i][j] = ball_wall_collision_time(&Balls[i], wall_id(j));
            }
            // Grab the earliest collision
            if (CollisionTimeGrid[i][j] < next_collision_time) {
                next_collision_time = CollisionTimeGrid[i][j];
                *collider1 = i;
                *collider2 = j;
            }
        }
    }
    // Return the time of the earliest collision
    return CollisionTimeGrid[*collider1][*collider2];
}

void simulate_collision(int collider1, int collider2) {
    // Ball-Ball collision
    if (collider2 < BallCount) {
        simulate_ball_ball_collision(&Balls[collider1], &Balls[collider2],
                                     CollisionTimeGrid[collider1][collider2]);
        // Reset all collision times of collider1 and collider2 to unknown
        for (int i = 0; i < BallCount + 4; i++) {
            CollisionTimeGrid[collider1][i] = UNKNOWN_COLLISION_TIME;
            CollisionTimeGrid[collider2][i] = UNKNOWN_COLLISION_TIME;
            if (i < BallCount) {
                // (if i is not a wall)
                CollisionTimeGrid[i][collider1] = UNKNOWN_COLLISION_TIME;
                CollisionTimeGrid[i][collider2] = UNKNOWN_COLLISION_TIME;
            }
        }
        return;
    }
    // Ball-Wall collision
    simulate_ball_wall_collision(&Balls[collider1], wall_id(collider2),
                                 CollisionTimeGrid[collider1][collider2]);
    // Reset all collision times of collider1 to unknown
    for (int i = 0; i < BallCount + 4; i++) {
        CollisionTimeGrid[collider1][i] = UNKNOWN_COLLISION_TIME;
        if (i < BallCount) {
            // (if i is not a wall)
            CollisionTimeGrid[i][collider1] = UNKNOWN_COLLISION_TIME;
        }
    }
}
