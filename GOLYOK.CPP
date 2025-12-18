#include "ALL.H"

double NO_COLLISION_TIME = 500000000.0;

double Maxgolyoseb = 0;

int BallCount = 0;
ball* Balls = NULL;
int BallsInitialized = 0;
double** CollisionTimeGrid = NULL;

static double NextCollisionTime = -2, ElapsedTimeSinceKeyframe = 0;

typedef double* duplaramutato;

static void create_balls(void) {
    BallCount = 9;
    Balls = new ball[BallCount];
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            int i = y * 3 + x;
            if (x == 0) {
                Balls[i].radius = 24;
            }
            if (x == 1) {
                Balls[i].radius = 30;
            }
            if (x == 2) {
                Balls[i].radius = 50;
            }
            Balls[i].keyframe_time = 0;
            Balls[i].v.x = 0;
            Balls[i].v.y = 0;
            Balls[i].keyframe_r.x = 320 + (x - 1) * 120;
            Balls[i].keyframe_r.y = 240 + (y - 1) * 120;
            Balls[i].keyframe_rotation = 0.0;
            Balls[i].current_rotation = 0.0;
            Balls[i].angular_velocity = 0.0;
        }
    }

    // Choose a random starting angle for the top-left ball
    srand(time(NULL));
    double angle = s_random(1000);
    angle *= K_pip2 / 1000.0;
    angle *= 0.999;
    angle += 0.0005;

    Balls[0].v.x = -sin(angle);
    Balls[0].v.y = -cos(angle);
}

// Initialize the balls and collision system
void balls_init(void) {
    create_balls();

    // Create collision grid
    // Allocate a 2D grid of width and height BallCount+4
    // The last 4 elements represent the 4 walls
    int allocation_success;
    NextCollisionTime = -2;
    ElapsedTimeSinceKeyframe = 0;
    if (BallsInitialized) {
        hiba("Balls already initialized!");
    }
    allocation_success = 1;
    CollisionTimeGrid = new duplaramutato[BallCount + 4];
    if (!CollisionTimeGrid) {
        allocation_success = 0;
    }
    if (allocation_success) {
        for (int i = 0; i < BallCount + 4; i++) {
            CollisionTimeGrid[i] = NULL;
        }
    }
    if (allocation_success) {
        for (int i = 0; i < BallCount + 4 && allocation_success; i++) {
            CollisionTimeGrid[i] = new double[BallCount + 4];
            if (CollisionTimeGrid[i]) {
                for (int j = 0; j < BallCount + 4; j++) {
                    CollisionTimeGrid[i][j] = -2;
                }
            }
            if (!CollisionTimeGrid[i]) {
                allocation_success = 0;
            }
        }
    }
    if (!allocation_success) {
        if (CollisionTimeGrid) {
            for (int i = 0; i < BallCount + 4; i++) {
                if (CollisionTimeGrid[i]) {
                    delete CollisionTimeGrid[i];
                }
            }
            delete CollisionTimeGrid;
            CollisionTimeGrid = NULL;
        }
        hiba("Out of memory!");
    }

    BallsInitialized = 1;
}

static double FirstTimeDelay = 0.0;

// Update ball positions
void balls_simulate(double dt) {
    // A minimal time delay before we actually start the ball animation.
    // This amount of time is inconsequential - it used to be 400.0
    if (FirstTimeDelay < 1.0) {
        FirstTimeDelay += dt;
        dt = 0.00000000001;
    }
    if (!BallsInitialized) {
        hiba("balls_simulate - balls not initialized!");
        return;
    }

    // If no collision occurs during dt, then just update position
    if (NextCollisionTime > -1 && NextCollisionTime > ElapsedTimeSinceKeyframe + dt) {
        ElapsedTimeSinceKeyframe += dt;
        for (int i = 0; i < BallCount; i++) {
            Balls[i].current_r = Balls[i].keyframe_r + (ElapsedTimeSinceKeyframe * Balls[i].v);
            Balls[i].current_rotation =
                Balls[i].keyframe_rotation + (ElapsedTimeSinceKeyframe * Balls[i].angular_velocity);
        }
        return;
    }

    // At least one collision will occur during dt
    // Or alternatively, some collision times are unknown
    dt += ElapsedTimeSinceKeyframe;

    // Handle all the collisions
    int done = 0;
    while (!done) {
        // Update all the collision times and get the next collision time
        int collider1, collider2;
        NextCollisionTime = megkeresidot(&collider1, &collider2);
        // Handle one collision
        if (NextCollisionTime > dt) {
            done = 1;
        } else {
            utkoztet(collider1, collider2);
        }
    }
    // Update keyframe position time of all the balls to the current time
    for (int i = 0; i < BallCount; i++) {
        Balls[i].keyframe_r = Balls[i].keyframe_r + ((dt - Balls[i].keyframe_time) * Balls[i].v);
        Balls[i].current_r = Balls[i].keyframe_r;
        Balls[i].keyframe_rotation = Balls[i].keyframe_rotation +
                                     ((dt - Balls[i].keyframe_time) * Balls[i].angular_velocity);
        Balls[i].current_rotation = Balls[i].keyframe_rotation;
        Balls[i].keyframe_time = 0;
    }
    // Update collision time to be relative to current time, skipping unused parts of the grid
    for (int i = 0; i < BallCount; i++) {
        for (int j = i + 1; j < BallCount + 4; j++) {
            CollisionTimeGrid[i][j] -= dt;
        }
    }

    // Reset keyframe time to current time
    ElapsedTimeSinceKeyframe = 0;
    NextCollisionTime -= dt;
}

/*void ujraszamol( void ) {
    for( int i = 0; i < Ngolyok; i++ )
        for( int j = i+1; j < Ngolyok+4; j++ )
            Idok[i][j] = -2;
    Legkozelebbiutkozesideje = -2;
    Pillanatnyiido = 0;
}

void idoreigazitmindet( void ) {
    for( int i = 0; i < Ngolyok; i++ ) {
        //dsvlet( &Golyok[i].r, dsvossz( &Golyok[i].r,
        //		dsvmul( Pillanatnyiido-Golyok[i].t, &Golyok[i].v ) ) );
        Golyok[i].r = Golyok[i].r +
                        ((Pillanatnyiido-Golyok[i].t) * Golyok[i].v);
        Golyok[i].t = 0;
    }
} */
