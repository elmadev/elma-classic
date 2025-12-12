#include "ALL.H"

motorst Motorst1, Motorst2; // Ez nem hasznalando
motorst *Motor1 = &Motorst1, *Motor2 = &Motorst2;

double GroundEscapeVelocity, WheelDeformationLength, Gravity;
double TwoPointDiscriminationDistance;

double VoltDelay;
double LevelEndDelay;

double SpringTensionCoefficient, SpringResistanceCoefficient;

double HeadRadius;

double ObjectRadius = 0.4;

// Hajszalnyival kisebb rigidbody.sugarnal, hogy kirajzolasnal ne logjon foldbe:
double WheelBackgroundRenderRadius = 0.395;

double LeftWheelDX, LeftWheelDY, RightWheelDX, RightWheelDY,
    BodyDY; // Ez az utolso vezeto helye

double MetersToPixels, PixelsToMeters;

/* Eredeti szamitas play.cpp-bol:
MetersToPixels = 100.0;
MetersToPixels *= Displaysizex/320.0;
MetersToPixels *= fakt;
Aranyx = MetersToPixels*320.0/640.0;
Aranyy = Aranyx*Displaysizey/Displaysizex*(640.0/480.0);*/

int MinimapScaleFactor = 10;

void init_motor(motorst* motor) {
    motor->flipped_bike = 0;
    motor->flipped_camera = 0;
    motor->gravity_direction = 1;
    motor->prev_brake = 0;

    // Kor1:
    motor->bike.rotation = 0.0;
    motor->bike.angular_velocity = 0.0;
    motor->bike.radius = 0.3;
    motor->bike.mass = 200; // 100;
    // inertia = mass * r*r;
    motor->bike.inertia = 200.0 * 0.55 * 0.55;
    motor->bike.r = vekt2(2.75, 3.6);
    motor->bike.v = vekt2(0, 0);

    // Kor2:
    motor->left_wheel.rotation = 0.0; // 30
    motor->left_wheel.angular_velocity = 0.0;
    motor->left_wheel.radius = 0.4;
    motor->left_wheel.mass = 10;
    motor->left_wheel.inertia = 0.32;
    motor->left_wheel.r = vekt2(1.9, 3.0);
    motor->left_wheel.v = vekt2(0, 0);

    // Kor4:
    motor->right_wheel.rotation = 0.0; // 40
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

    double fakt = 0.48; // 0.54; volt mar 28 is
    if (State->minibike) {
        fakt = 0.30;
    }

    MetersToPixels = 100.0 * fakt;
    PixelsToMeters = 1.0 / MetersToPixels;

    // MinimapScaleFactor = 0.28*MetersToPixels*0.5;
    MinimapScaleFactor = 0.42 * MetersToPixels * 0.5;

    GroundEscapeVelocity = 0.01;
    WheelDeformationLength = 0.005; // uj
    // WheelDeformationLength = 0.02; // regi
    Gravity = 10.0; // nehezsegi gyorsulas: a [m/s^2]
    TwoPointDiscriminationDistance = 0.1;

    // VoltDelay = 0.2;
    VoltDelay = 0.4;
    LevelEndDelay = 1.0;

    SpringTensionCoefficient = 10000.0;   // rudban fuggoleges modulus: F/l [N/m]
    SpringResistanceCoefficient = 1000.0; // rudban mozgasi ellenallas: F/v [N*s/m]

    HeadRadius = 0.238; // 0.26;

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

// reorder.h-ban friend-kent vannak deklaralva mar:
// pgazhatra-ba beepiti betoltvet:
void encode_frame_count(recorder* rec) {
    if (rec->betoltve < 80) {
        return;
    }
    unsigned int beirando = rec->betoltve;
    for (int i = 0; i < 32; i++) {
        rec->pgazhatra[40 + i] = rec->pgazhatra[40 + i] & 127;
        if (beirando & 1) {
            rec->pgazhatra[40 + i] += 128;
        }
        beirando = beirando >> 1;
    }
}

// pgazhatra alapjan ellenorzi hosszt:
// Ezt vegleges verziobol kiszedjuk, mert ezt csak mi ellenorizzuk Csabival:
/*int frame_count_integrity( recorder* rec ) {
    if( rec->betoltve < 80 )
        return 1;
    unsigned int szam = 0;
    for( int i = 0; i < 32; i++ ) {
        szam *= 2;
        if( rec->pgazhatra[40+31-i] & 128 )
            szam += 1;
    }
    if( szam == rec->betoltve )
        return 1;
    else
        return 0;
} */
