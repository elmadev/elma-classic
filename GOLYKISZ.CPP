#include "ALL.H"

double megkeresidot(int* xlegjobb, int* ylegjobb) {
    double eddigilegjobbt = NO_COLLISION_TIME;
    int x, y;

    *xlegjobb = 0;
    *ylegjobb = 1;

    for (x = 0; x < BallCount; x++) {
        // Golyo-Golyo:
        for (y = x + 1; y < BallCount; y++) {
            if (CollisionTimeGrid[x][y] < -1) {
                CollisionTimeGrid[x][y] = ggutkozesideje(&Balls[x], &Balls[y]);
            }
            if (CollisionTimeGrid[x][y] < eddigilegjobbt) {
                eddigilegjobbt = CollisionTimeGrid[x][y];
                *xlegjobb = x;
                *ylegjobb = y;
            }
        }
        // Golyo-Oldal:
        for (y = BallCount; y < BallCount + 4; y++) {
            if (CollisionTimeGrid[x][y] < -1) {
                CollisionTimeGrid[x][y] = goutkozesideje(&Balls[x], y - BallCount);
            }
            if (CollisionTimeGrid[x][y] < eddigilegjobbt) {
                eddigilegjobbt = CollisionTimeGrid[x][y];
                *xlegjobb = x;
                *ylegjobb = y;
            }
        }
    }
    return CollisionTimeGrid[*xlegjobb][*ylegjobb];
}

void utkoztet(int x, int y) {
    int i;

    // Golyo-Golyo:
    if (y < BallCount) {
        ggutkozes(&Balls[x], &Balls[y], CollisionTimeGrid[x][y]);
        // Oszlopokat es sorokat ervenytelenit:
        for (i = 0; i < BallCount + 4; i++) {
            CollisionTimeGrid[x][i] = -2;
            CollisionTimeGrid[y][i] = -2;
            if (i < BallCount) {
                CollisionTimeGrid[i][x] = -2;
                CollisionTimeGrid[i][y] = -2;
            }
        }
        return;
    }
    // Golyo-Oldal:
    goutkozes(&Balls[x], y - BallCount, CollisionTimeGrid[x][y]);
    // Oszlopot ervenytelenit:
    for (i = 0; i < BallCount + 4; i++) {
        CollisionTimeGrid[x][i] = -2;
        if (i < BallCount) {
            CollisionTimeGrid[i][x] = -2;
        }
    }
}
