#include "ALL.H"

double next_collision_time(int* collider1, int* collider2) {
    double eddigilegjobbt = NO_COLLISION_TIME;
    int x, y;

    *collider1 = 0;
    *collider2 = 1;

    for (x = 0; x < BallCount; x++) {
        // Golyo-Golyo:
        for (y = x + 1; y < BallCount; y++) {
            if (CollisionTimeGrid[x][y] < -1) {
                CollisionTimeGrid[x][y] = ggutkozesideje(&Balls[x], &Balls[y]);
            }
            if (CollisionTimeGrid[x][y] < eddigilegjobbt) {
                eddigilegjobbt = CollisionTimeGrid[x][y];
                *collider1 = x;
                *collider2 = y;
            }
        }
        // Golyo-Oldal:
        for (y = BallCount; y < BallCount + 4; y++) {
            if (CollisionTimeGrid[x][y] < -1) {
                CollisionTimeGrid[x][y] = goutkozesideje(&Balls[x], y - BallCount);
            }
            if (CollisionTimeGrid[x][y] < eddigilegjobbt) {
                eddigilegjobbt = CollisionTimeGrid[x][y];
                *collider1 = x;
                *collider2 = y;
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
