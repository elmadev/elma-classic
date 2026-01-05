#include "ALL.H"
#include "physics_init.h"

int FlagTagAHasFlag = 0, FlagTagImmunity = 0;
int FlagTagAStarts = 1;
double FlagTimeA = 0.0, FlagTimeB = 0.0;

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

static double WHEEL_DIST_SQURAED = 0.64;
static double IMMUNITY_DIST_SQUARED = 1.44;

void flagtag(double time) {
    if (FlagTagImmunity) {
        if (!points_within_distance(&Motor1->left_wheel.r, &Motor2->left_wheel.r,
                                    IMMUNITY_DIST_SQUARED) &&
            !points_within_distance(&Motor1->left_wheel.r, &Motor2->right_wheel.r,
                                    IMMUNITY_DIST_SQUARED) &&
            !points_within_distance(&Motor1->right_wheel.r, &Motor2->left_wheel.r,
                                    IMMUNITY_DIST_SQUARED) &&
            !points_within_distance(&Motor1->right_wheel.r, &Motor2->right_wheel.r,
                                    IMMUNITY_DIST_SQUARED)) {
            // Now the wheels are far enough apart
            FlagTagImmunity = 0;
        }
    } else {
        if (points_within_distance(&Motor1->left_wheel.r, &Motor2->left_wheel.r,
                                   WHEEL_DIST_SQURAED) ||
            points_within_distance(&Motor1->left_wheel.r, &Motor2->right_wheel.r,
                                   WHEEL_DIST_SQURAED) ||
            points_within_distance(&Motor1->right_wheel.r, &Motor2->left_wheel.r,
                                   WHEEL_DIST_SQURAED) ||
            points_within_distance(&Motor1->right_wheel.r, &Motor2->right_wheel.r,
                                   WHEEL_DIST_SQURAED)) {
            // Wheels touching
            FlagTagImmunity = 1;
            FlagTagAHasFlag = !FlagTagAHasFlag;
        }
    }

    if (FlagTagAHasFlag) {
        FlagTimeA += time - FlagTagElapsedTime;
    } else {
        FlagTimeB += time - FlagTagElapsedTime;
    }

    FlagTagElapsedTime = time;
}

void flagtag_replay(double time) {
    if (FlagTagAHasFlag) {
        FlagTimeA += time - FlagTagElapsedTime;
    } else {
        FlagTimeB += time - FlagTagElapsedTime;
    }

    FlagTagElapsedTime = time;
}
