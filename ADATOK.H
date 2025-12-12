

extern double GroundEscapeVelocity, WheelDeformationLength, Gravity;
extern double TwoPointDiscriminationDistance;

extern double VoltDelay;
extern double LevelEndDelay;

extern double SpringTensionCoefficient, SpringResistanceCoefficient;

void init_physics_data(void);

extern double HeadRadius;

extern double ObjectRadius;

// Hajszalnyival kisebb rigidbody.sugarnal, hogy kirajzolasnal ne logjon foldbe:
extern double WheelBackgroundRenderRadius;

// MCGA kirajzolasos:

extern double MetersToPixels, PixelsToMeters;

extern double LeftWheelDX, LeftWheelDY, RightWheelDX, RightWheelDY,
    BodyDY; // Ez az utolso vezeto helye

#define OBJECT_PIXEL_RADIUS (20)

extern int MinimapScaleFactor;

struct rigidbody {
    double rotation;
    double angular_velocity;
    double radius;
    double mass;
    double inertia; // Moment of inertia
    vekt2 r;
    vekt2 v;
};

struct motorst {
    rigidbody bike, left_wheel, right_wheel;
    vekt2 head_r; // bike-bol Kor12Fejr fuggveny allitja elo (leptet.h)!
    int flipped_bike;
    int flipped_camera;
    int gravity_direction; // 1 a normalis (0:UP 1:DOWN 2:LEFT 3:RIGHT)

    vekt2 body_r, body_v; // Motoros

    int apple_count; // hanyat evett ez a motoros

    // Leptet ezeket is hasznalja:
    int prev_brake;
    double left_wheel_brake_rotation, right_wheel_brake_rotation;

    int volting_right, volting_left;
    double right_volt_time, left_volt_time;
    double angular_velocity_pre_right_volt, angular_velocity_pre_left_volt;
};

extern motorst *Motor1, *Motor2;

// reorder.h-ban friend-kent mar deklaralva van:
// pgazhatra-ba beepiti betoltvet:
class recorder;
void encode_frame_count(recorder* rec);
// pgazhatra alapjan ellenorzi hosszt:
int frame_count_integrity(recorder* rec);
