#ifndef BALL_H
#define BALL_H

#include "vect2.h"

constexpr double UNKNOWN_COLLISION_TIME = -2.0;
constexpr double NO_COLLISION_TIME = 500000000.0;

struct ball {
    vect2 keyframe_r;
    vect2 current_r;
    vect2 v;
    double keyframe_rotation;
    double current_rotation;
    double angular_velocity;
    double radius;
    double keyframe_time;
};

void balls_init();
void balls_simulate(double dt);

extern int BallCount;
extern ball* Balls;
extern double** CollisionTimeGrid;

#endif
