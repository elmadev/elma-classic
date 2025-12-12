
#define NEVHOSSZ (30)

struct rigidbody {
    char name[NEVHOSSZ];
    double rotation;
    double angular_velocity;
    double radius;
    double mass;
    double inertia; // Moment of inertia
    vekt2 r;
    vekt2 v;
};
