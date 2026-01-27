#include "ball.h"
#include "ball_handler.h"
#include "main.h"
#include "M_PIC.H"
#include "platform_utils.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

int BallCount = 0;
ball* Balls = nullptr;
bool BallsInitialized = false;
double** CollisionTimeGrid = nullptr;

static double NextCollisionTime = UNKNOWN_COLLISION_TIME;
static double ElapsedTimeSinceKeyframe = 0.0;

constexpr int InitColumns = 3;
constexpr int InitRows = 3;

static void create_balls() {
    BallCount = InitColumns * InitRows;
    Balls = new ball[BallCount];
    for (int x = 0; x < InitColumns; x++) {
        for (int y = 0; y < InitRows; y++) {
            int i = y * InitRows + x;
            if (x == 0) {
                Balls[i].radius = 24.0;
            }
            if (x == 1) {
                Balls[i].radius = 30.0;
            }
            if (x == 2) {
                Balls[i].radius = 50.0;
            }
            Balls[i].keyframe_time = 0.0;
            Balls[i].v.x = 0.0;
            Balls[i].v.y = 0.0;
            Balls[i].keyframe_r.x = SCREEN_WIDTH / 2 + (x - 1) * 120.0;
            Balls[i].keyframe_r.y = SCREEN_HEIGHT / 2 + (y - 1) * 120.0;
            Balls[i].keyframe_rotation = 0.0;
            Balls[i].current_rotation = 0.0;
            Balls[i].angular_velocity = 0.0;
        }
    }
    // Choose a random starting angle for the top-left ball
    srand((unsigned int)time(nullptr));
    double angle = random_range(1000);
    angle *= HALF_PI / 1000.0;
    angle *= 0.999;
    angle += 0.0005;
    Balls[0].v.x = -sin(angle);
    Balls[0].v.y = -cos(angle);
}

// Initialize the balls and collision system
void balls_init() {
    create_balls();

    // Create collision grid
    // Allocate a 2D grid of width and height BallCount+4
    // The last 4 elements represent the 4 walls
    NextCollisionTime = UNKNOWN_COLLISION_TIME;
    ElapsedTimeSinceKeyframe = 0.0;
    if (BallsInitialized) {
        internal_error("Balls already initialized!");
    }
    bool allocation_success = true;
    CollisionTimeGrid = new double*[BallCount + 4];
    if (!CollisionTimeGrid) {
        allocation_success = false;
    }
    if (allocation_success) {
        for (int i = 0; i < BallCount + 4; i++) {
            CollisionTimeGrid[i] = nullptr;
        }
    }
    if (allocation_success) {
        for (int i = 0; i < BallCount + 4 && allocation_success; i++) {
            CollisionTimeGrid[i] = new double[BallCount + 4];
            if (CollisionTimeGrid[i]) {
                for (int j = 0; j < BallCount + 4; j++) {
                    CollisionTimeGrid[i][j] = UNKNOWN_COLLISION_TIME;
                }
            }
            if (!CollisionTimeGrid[i]) {
                allocation_success = false;
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
            CollisionTimeGrid = nullptr;
        }
        internal_error("Out of memory!");
    }
    BallsInitialized = true;
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
        internal_error("balls_simulate - balls not initialized!");
        return;
    }

    // If no collision occurs during dt, then just update position
    double current_time = ElapsedTimeSinceKeyframe + dt;
    if (NextCollisionTime > -1 && NextCollisionTime > current_time) {
        ElapsedTimeSinceKeyframe = current_time;
        for (int i = 0; i < BallCount; i++) {
            Balls[i].current_r = Balls[i].keyframe_r + (ElapsedTimeSinceKeyframe * Balls[i].v);
            Balls[i].current_rotation =
                Balls[i].keyframe_rotation + (ElapsedTimeSinceKeyframe * Balls[i].angular_velocity);
        }
        return;
    }

    // At least one collision will occur during dt
    // Or alternatively, some collision times are unknown
    // Handle all the collisions
    while (true) {
        // Update all the collision times and get the next collision time
        int collider1, collider2;
        NextCollisionTime = next_collision_time(&collider1, &collider2);
        // Handle one collision
        if (NextCollisionTime > current_time) {
            break;
        }
        simulate_collision(collider1, collider2);
    }
    // Update keyframe position time of all the balls to the current time
    for (int i = 0; i < BallCount; i++) {
        Balls[i].keyframe_r =
            Balls[i].keyframe_r + ((current_time - Balls[i].keyframe_time) * Balls[i].v);
        Balls[i].current_r = Balls[i].keyframe_r;
        Balls[i].keyframe_rotation =
            Balls[i].keyframe_rotation +
            ((current_time - Balls[i].keyframe_time) * Balls[i].angular_velocity);
        Balls[i].current_rotation = Balls[i].keyframe_rotation;
        Balls[i].keyframe_time = 0;
    }
    // Update collision time to be relative to current time, skipping unused parts of the grid
    for (int i = 0; i < BallCount; i++) {
        for (int j = i + 1; j < BallCount + 4; j++) {
            CollisionTimeGrid[i][j] -= current_time;
        }
    }

    // Reset keyframe time to current time
    ElapsedTimeSinceKeyframe = 0.0;
    NextCollisionTime -= current_time;
}
