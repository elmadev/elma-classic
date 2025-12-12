

extern double GroundEscapeVelocity, WheelDeformationLength, Gravity;
extern double TwoPointDiscriminationDistance;

extern double VoltDelay;
extern double LevelEndDelay;

extern double SpringTensionCoefficient, SpringResistanceCoefficient;

void init_physics_data(void);

extern double HeadRadius;

extern double ObjectRadius;

extern double WheelBackgroundRenderRadius;

extern double MetersToPixels, PixelsToMeters;

extern double LeftWheelDX, LeftWheelDY, RightWheelDX, RightWheelDY, BodyDY;

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
    vekt2 head_r;
    int flipped_bike;
    int flipped_camera;
    int gravity_direction; // (0:UP 1:DOWN 2:LEFT 3:RIGHT)

    vekt2 body_r, body_v;

    int apple_count;

    int prev_brake;
    double left_wheel_brake_rotation, right_wheel_brake_rotation;

    int volting_right, volting_left;
    double right_volt_time, left_volt_time;
    double angular_velocity_pre_right_volt, angular_velocity_pre_left_volt;
};

extern motorst *Motor1, *Motor2;

class recorder;
void encode_frame_count(recorder* rec);
int frame_count_integrity(recorder* rec);
