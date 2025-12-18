#include "ALL.H"

double next_collision_time(int* collider1, int* collider2) {
    double next_collision_time = NO_COLLISION_TIME;
    int i, j;

    *collider1 = 0;
    *collider2 = 1;

    for (i = 0; i < BallCount; i++) {
        // Golyo-Golyo:
        for (j = i + 1; j < BallCount; j++) {
            if (CollisionTimeGrid[i][j] < -1) {
                CollisionTimeGrid[i][j] = ggutkozesideje(&Balls[i], &Balls[j]);
            }
            if (CollisionTimeGrid[i][j] < next_collision_time) {
                next_collision_time = CollisionTimeGrid[i][j];
                *collider1 = i;
                *collider2 = j;
            }
        }
        // Golyo-Oldal:
        for (j = BallCount; j < BallCount + 4; j++) {
            if (CollisionTimeGrid[i][j] < -1) {
                CollisionTimeGrid[i][j] = goutkozesideje(&Balls[i], j - BallCount);
            }
            if (CollisionTimeGrid[i][j] < next_collision_time) {
                next_collision_time = CollisionTimeGrid[i][j];
                *collider1 = i;
                *collider2 = j;
            }
        }
    }
    return CollisionTimeGrid[*collider1][*collider2];
}

void simulate_collision(int collider1, int collider2) {
    int i;

    // Golyo-Golyo:
    if (collider2 < BallCount) {
        ggutkozes(&Balls[collider1], &Balls[collider2], CollisionTimeGrid[collider1][collider2]);
        // Oszlopokat es sorokat ervenytelenit:
        for (i = 0; i < BallCount + 4; i++) {
            CollisionTimeGrid[collider1][i] = -2;
            CollisionTimeGrid[collider2][i] = -2;
            if (i < BallCount) {
                CollisionTimeGrid[i][collider1] = -2;
                CollisionTimeGrid[i][collider2] = -2;
            }
        }
        return;
    }
    // Golyo-Oldal:
    goutkozes(&Balls[collider1], collider2 - BallCount, CollisionTimeGrid[collider1][collider2]);
    // Oszlopot ervenytelenit:
    for (i = 0; i < BallCount + 4; i++) {
        CollisionTimeGrid[collider1][i] = -2;
        if (i < BallCount) {
            CollisionTimeGrid[i][collider1] = -2;
        }
    }
}
