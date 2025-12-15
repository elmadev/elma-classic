#include "eol_settings.h"
#include "physics_init.h"

static motorst Motorst1;
static motorst Motorst2;
motorst* Motor1 = &Motorst1;
motorst* Motor2 = &Motorst2;

// Perpendicular velocity required to detach wheel from ground.
double GroundEscapeVelocity;
// Amount by which the wheel is sunk into the ground
double WheelDeformationLength;
double Gravity;

// Minimum distance between two vertices to be considered separate points
double TwoPointDiscriminationDistance;

double VoltDelay;
double LevelEndDelay;

double SpringTensionCoefficient;
double SpringResistanceCoefficient;

double HeadRadius;

double ObjectRadius = 0.4;

// Render size of the wheel when not turning, in meters, to avoid the wheel appearing to be inside
// the ground
double WheelBackgroundRenderRadius = 0.395;

double LeftWheelDX;
double LeftWheelDY;
double RightWheelDX;
double RightWheelDY;
double BodyDY;

double MetersToPixels;
double PixelsToMeters;

int MinimapScaleFactor = 10;

void init_motor(motorst* motor) {
    motor->flipped_bike = 0;
    motor->flipped_camera = 0;
    motor->gravity_direction = MotorGravity::Down;
    motor->prev_brake = 0;

    motor->bike.rotation = 0.0;
    motor->bike.angular_velocity = 0.0;
    motor->bike.radius = 0.3;
    motor->bike.mass = 200;
    // inertia = mass * radius * radius
    // although radius = 0.55 is used instead of 0.3, as set above.
    motor->bike.inertia = 200.0 * 0.55 * 0.55;
    motor->bike.r = vect2(2.75, 3.6);
    motor->bike.v = vect2(0, 0);

    motor->left_wheel.rotation = 0.0;
    motor->left_wheel.angular_velocity = 0.0;
    motor->left_wheel.radius = 0.4;
    motor->left_wheel.mass = 10;
    motor->left_wheel.inertia = 0.32;
    motor->left_wheel.r = vect2(1.9, 3.0);
    motor->left_wheel.v = vect2(0, 0);

    motor->right_wheel.rotation = 0.0;
    motor->right_wheel.angular_velocity = 0.0;
    motor->right_wheel.radius = 0.4;
    motor->right_wheel.mass = 10;
    motor->right_wheel.inertia = 0.32;
    motor->right_wheel.r = vect2(3.6, 3.0);
    motor->right_wheel.v = vect2(0, 0);

    motor->body_r = vect2(2.75, 4.04);
    motor->body_v = vect2(0.0, 0.0);
}

void set_zoom_factor() {
    double zoom_factor = 0.48 * EolSettings->zoom();
    MetersToPixels = 100.0 * zoom_factor;
    PixelsToMeters = 1.0 / MetersToPixels;

    MinimapScaleFactor = (int)(0.42 * MetersToPixels * 0.5);
}

void init_physics_data(void) {
    init_motor(Motor1);
    init_motor(Motor2);

    set_zoom_factor();

    GroundEscapeVelocity = 0.01;          // m/s
    WheelDeformationLength = 0.005;       // m
    Gravity = 10.0;                       // m/s^2
    TwoPointDiscriminationDistance = 0.1; // m

    VoltDelay = 0.4;
    LevelEndDelay = 1.0;

    SpringTensionCoefficient = 10000.0;   // N/m
    SpringResistanceCoefficient = 1000.0; // N/(m/s)

    HeadRadius = 0.238; // m

    // Relative positions
    vect2 vtmp = Motor1->left_wheel.r - Motor1->bike.r;
    LeftWheelDX = vtmp.x;
    LeftWheelDY = vtmp.y;
    vtmp = Motor1->right_wheel.r - Motor1->bike.r;
    RightWheelDX = vtmp.x;
    RightWheelDY = vtmp.y;

    BodyDY = Motor1->body_r.y - Motor1->bike.r.y;
}
