
#define NEVHOSSZ (30)

struct rigidbody {
    char name[NEVHOSSZ];
    double rotation, angular_velocity, radius, mass, inertia;
    vekt2 r, v;
    // int    utoljaraleert;
};
