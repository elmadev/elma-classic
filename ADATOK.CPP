#include "ALL.H"

motorst Motorst1, Motorst2;
motorst *Motor1 = &Motorst1, *Motor2 = &Motorst2;

// Perpendicular velocity required to detach wheel from ground.
// Amount by which the wheel is sunk into the ground
double GroundEscapeVelocity, WheelDeformationLength, Gravity;

// Minimum distance between two vertices to be considered separate points
double TwoPointDiscriminationDistance;

double VoltDelay;
double LevelEndDelay;

double SpringTensionCoefficient, SpringResistanceCoefficient;

double HeadRadius;

double ObjectRadius = 0.4;

// Render size of the wheel when not turning, in meters, to avoid the wheel appearing to be inside
// the ground
double WheelBackgroundRenderRadius = 0.395;

double LeftWheelDX, LeftWheelDY, RightWheelDX, RightWheelDY, BodyDY;

double MetersToPixels, PixelsToMeters;

int MinimapScaleFactor = 10;

void init_motor(motorst* motor) {
    motor->flipped_bike = 0;
    motor->flipped_camera = 0;
    motor->gravity_direction = 1;
    motor->prev_brake = 0;

    motor->bike.rotation = 0.0;
    motor->bike.angular_velocity = 0.0;
    motor->bike.radius = 0.3;
    motor->bike.mass = 200;
    // inertia = mass * radius * radius
    // although radius = 0.55 is used instead of 0.3, as set above.
    motor->bike.inertia = 200.0 * 0.55 * 0.55;
    motor->bike.r = vekt2(2.75, 3.6);
    motor->bike.v = vekt2(0, 0);

    motor->left_wheel.rotation = 0.0;
    motor->left_wheel.angular_velocity = 0.0;
    motor->left_wheel.radius = 0.4;
    motor->left_wheel.mass = 10;
    motor->left_wheel.inertia = 0.32;
    motor->left_wheel.r = vekt2(1.9, 3.0);
    motor->left_wheel.v = vekt2(0, 0);

    motor->right_wheel.rotation = 0.0;
    motor->right_wheel.angular_velocity = 0.0;
    motor->right_wheel.radius = 0.4;
    motor->right_wheel.mass = 10;
    motor->right_wheel.inertia = 0.32;
    motor->right_wheel.r = vekt2(3.6, 3.0);
    motor->right_wheel.v = vekt2(0, 0);

    motor->body_r = vekt2(2.75, 4.04);
    motor->body_v = vekt2(0.0, 0.0);
}

void init_physics_data(void) {
    init_motor(Motor1);
    init_motor(Motor2);

    double zoom_factor = 0.48;
    if (State->minibike) {
        zoom_factor = 0.30;
    }

    MetersToPixels = 100.0 * zoom_factor;
    PixelsToMeters = 1.0 / MetersToPixels;

    MinimapScaleFactor = 0.42 * MetersToPixels * 0.5;

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
    vekt2 vtmp = Motor1->left_wheel.r - Motor1->bike.r;
    LeftWheelDX = vtmp.x;
    LeftWheelDY = vtmp.y;
    vtmp = Motor1->right_wheel.r - Motor1->bike.r;
    RightWheelDX = vtmp.x;
    RightWheelDY = vtmp.y;

    BodyDY = Motor1->body_r.y - Motor1->bike.r.y;
}

/*static double atmeretez( void ) {
    FILE* h = fopen( "size.inf", "rt" );
    if( !h )
        return 1.0;
    long meret = 1000;
    if( fscanf( h, "%ld", &meret ) != 1 ) {
        fclose( h );
        return 1.0;
    }
    fclose( h );
    if( meret < 100 )
        meret = 100;
    if( meret > 10000 )
        meret = 10000;
    return (double)meret/1000.0;
} */

// Encode framecount into MSB of the flags
void encode_frame_count(recorder* rec) {
    if (rec->betoltve < 80) {
        return;
    }
    unsigned int frame_count = rec->betoltve;
    for (int i = 0; i < 32; i++) {
        rec->pgazhatra[40 + i] = rec->pgazhatra[40 + i] & 127;
        if (frame_count & 1) {
            rec->pgazhatra[40 + i] += 128;
        }
        frame_count = frame_count >> 1;
    }
}

// Check that the framecount matches with the MSB of the flags
/*int frame_count_integrity( recorder* rec ) {
    if( rec->betoltve < 80 )
        return 1;
    unsigned int frame_count = 0;
    for( int i = 0; i < 32; i++ ) {
        frame_count *= 2;
        if( rec->pgazhatra[40+31-i] & 128 )
            frame_count += 1;
    }
    if( frame_count == rec->betoltve )
        return 1;
    else
        return 0;
} */
