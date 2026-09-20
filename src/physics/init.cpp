#include "physics/init.h"
#include "eol/settings.h"
#include "physics/forces.h"

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

double LeftWheelDX;
double LeftWheelDY;
double RightWheelDX;
double RightWheelDY;
double BodyDY;

double MetersToPixels;
double PixelsToMeters;

int MinimapScaleFactor = 10;
double MetersToMinimapPixels;

void motorst::init() {
    flipped_bike = 0;
    gravity_direction = MotorGravity::Down;
    prev_brake = false;
    one_wheel_failed = false;

    bike.rotation = 0.0;
    bike.angular_velocity = 0.0;
    bike.radius = 0.3;
    bike.mass = 200;
    // inertia = mass * radius * radius
    // although radius = 0.55 is used instead of 0.3, as set above.
    bike.inertia = 200.0 * 0.55 * 0.55;
    bike.r = vect2(2.75, 3.6);
    bike.v = vect2(0, 0);

    left_wheel.rotation = 0.0;
    left_wheel.angular_velocity = 0.0;
    left_wheel.radius = 0.4;
    left_wheel.mass = 10;
    left_wheel.inertia = 0.32;
    left_wheel.r = vect2(1.9, 3.0);
    left_wheel.v = vect2(0, 0);
    left_wheel.touching_edge = false;

    right_wheel.rotation = 0.0;
    right_wheel.angular_velocity = 0.0;
    right_wheel.radius = 0.4;
    right_wheel.mass = 10;
    right_wheel.inertia = 0.32;
    right_wheel.r = vect2(3.6, 3.0);
    right_wheel.v = vect2(0, 0);
    right_wheel.touching_edge = false;

    body_r = vect2(2.75, 4.04);
    body_v = vect2(0.0, 0.0);

    set_head_position(this);
}

void set_zoom_factor() {
    double zoom_factor = 0.48 * EolSettings->zoom();
    MetersToPixels = 100.0 * zoom_factor;
    PixelsToMeters = 1.0 / MetersToPixels;

    set_minimap_zoom_factor();
}

void set_minimap_zoom_factor() {
    MinimapScaleFactor = (int)(0.42 * MetersToPixels * 0.5 / EolSettings->minimap_zoom());
    MetersToMinimapPixels = MetersToPixels / MinimapScaleFactor;
}

void init_physics_data() {
    Motor1->init();
    Motor2->init();

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
