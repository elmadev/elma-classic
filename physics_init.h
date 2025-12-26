#ifndef PHYSICS_INIT_H
#define PHYSICS_INIT_H

#include "vect2.h"

extern double GroundEscapeVelocity;
extern double WheelDeformationLength;

extern double Gravity;

extern double TwoPointDiscriminationDistance;

extern double VoltDelay;
extern double LevelEndDelay;

extern double SpringTensionCoefficient;
extern double SpringResistanceCoefficient;

extern double HeadRadius;

extern double ObjectRadius;

extern double WheelBackgroundRenderRadius;

extern double MetersToPixels, PixelsToMeters;

extern double LeftWheelDX, LeftWheelDY, RightWheelDX, RightWheelDY, BodyDY;

extern int MinimapScaleFactor;

enum class MotorGravity {
    Up = 0,
    Down = 1,
    Left = 2,
    Right = 3,
};

struct rigidbody {
    double rotation;
    double angular_velocity;
    double radius;
    double mass;
    double inertia; // Moment of inertia
    vect2 r;
    vect2 v;
};

struct motorst {
    rigidbody bike;
    rigidbody left_wheel;
    rigidbody right_wheel;
    vect2 head_r;
    int flipped_bike;
    int flipped_camera;
    MotorGravity gravity_direction;

    vect2 body_r;
    vect2 body_v;

    int apple_count;

    int prev_brake;
    double left_wheel_brake_rotation;
    double right_wheel_brake_rotation;

    int volting_right;
    int volting_left;
    double right_volt_time;
    double left_volt_time;
    double angular_velocity_pre_right_volt;
    double angular_velocity_pre_left_volt;
};

extern motorst *Motor1, *Motor2;

void set_zoom_factor();
void init_physics_data();
void init_motor(motorst* motor);

#endif
