#include "ALL.H"
#include "physics_init.h"

int FlagTagAHasFlag = 0, FlagTagImmunity = 0;
int FlagTagAStarts = 1; // Legkozelebbi jateknal
double FlagTimeA = 0.0, FlagTimeB = 0.0;

// Ez nem publikus:
double FlagTagElapsedTime = -1.0;

void flagtag_reset(void) {
    FlagTagAHasFlag = FlagTagAStarts;
    FlagTagAStarts = !FlagTagAStarts;
    FlagTagImmunity = 1;
    FlagTimeA = 0.0, FlagTimeB = 0.0;
    FlagTagElapsedTime = 0.0;
}

static int points_within_distance(vect2* v1, vect2* v2, double distance_squared) {
    double dx = v1->x - v2->x;
    double dy = v1->y - v2->y;
    if (dx * dx + dy * dy < distance_squared) {
        return 1;
    } else {
        return 0;
    }
}

static double WHEEL_DIST_SQURAED = 0.64; // 0.8 a negyzeten
// static double IMMUNITY_DIST_SQUARED = 3.2; // 1.8 a negyzeten
static double IMMUNITY_DIST_SQUARED = 1.44; // 1.2 a negyzeten

void flagtag(double time) {
    if (FlagTagImmunity) {
        // Eddig kozel voltak:
        if (!points_within_distance(&Motor1->left_wheel.r, &Motor2->left_wheel.r,
                                    IMMUNITY_DIST_SQUARED) &&
            !points_within_distance(&Motor1->left_wheel.r, &Motor2->right_wheel.r,
                                    IMMUNITY_DIST_SQUARED) &&
            !points_within_distance(&Motor1->right_wheel.r, &Motor2->left_wheel.r,
                                    IMMUNITY_DIST_SQUARED) &&
            !points_within_distance(&Motor1->right_wheel.r, &Motor2->right_wheel.r,
                                    IMMUNITY_DIST_SQUARED)) {
            // Most mar elegendoen tavol vannak:
            FlagTagImmunity = 0;
        }
    } else {
        // Eddig tavol voltak:
        if (points_within_distance(&Motor1->left_wheel.r, &Motor2->left_wheel.r,
                                   WHEEL_DIST_SQURAED) ||
            points_within_distance(&Motor1->left_wheel.r, &Motor2->right_wheel.r,
                                   WHEEL_DIST_SQURAED) ||
            points_within_distance(&Motor1->right_wheel.r, &Motor2->left_wheel.r,
                                   WHEEL_DIST_SQURAED) ||
            points_within_distance(&Motor1->right_wheel.r, &Motor2->right_wheel.r,
                                   WHEEL_DIST_SQURAED)) {
            // Megerintettek egymast:
            FlagTagImmunity = 1;
            FlagTagAHasFlag = !FlagTagAHasFlag;
        }
    }

    // Idot noveljuk:
    if (FlagTagAHasFlag) {
        FlagTimeA += time - FlagTagElapsedTime;
    } else {
        FlagTimeB += time - FlagTagElapsedTime;
    }

    FlagTagElapsedTime = time;
}

// FlagTagAHasFlag alapjan csak idoket allitja:
void flagtag_replay(double time) {
    // Idot noveljuk:
    if (FlagTagAHasFlag) {
        FlagTimeA += time - FlagTagElapsedTime;
    } else {
        FlagTimeB += time - FlagTagElapsedTime;
    }

    FlagTagElapsedTime = time;
}
