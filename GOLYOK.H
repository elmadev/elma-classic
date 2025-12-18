#ifndef GOLYOK_H
#define GOLYOK_H

typedef struct {
    vekt2 keyframe_r,
        // relozo,
        current_r, v;
    double keyframe_rotation, current_rotation, angular_velocity;
    double radius;
    double keyframe_time;
} ball;

void balls_init(void);
void balls_simulate(double dt);
// void ujraszamol( void );
// void idoreigazitmindet( void );

extern int BallCount;
extern ball* Balls;

extern double NO_COLLISION_TIME;

extern int BallsInitialized;

extern double** CollisionTimeGrid;
extern double* T;

#endif
