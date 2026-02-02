#include "flagtag.h"
#include "physics_init.h"

constexpr double WHEEL_DIST_SQUARED = 0.64;
constexpr double IMMUNITY_DIST_SQUARED = 1.44;

bool FlagTagAHasFlag = false;
bool FlagTagImmunity = false;
bool FlagTagAStarts = true;
double FlagTimeA = 0.0;
double FlagTimeB = 0.0;

double FlagTagElapsedTime = -1.0;

void flagtag_reset() {
    FlagTagAHasFlag = FlagTagAStarts;
    FlagTagAStarts = !FlagTagAStarts;
    FlagTagImmunity = true;
    FlagTimeA = 0.0;
    FlagTimeB = 0.0;
    FlagTagElapsedTime = 0.0;
}

static bool points_within_distance(vect2* v1, vect2* v2, double distance_squared) {
    double dx = v1->x - v2->x;
    double dy = v1->y - v2->y;
    return dx * dx + dy * dy < distance_squared;
}

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
            FlagTagImmunity = false;
        }
    } else {
        if (points_within_distance(&Motor1->left_wheel.r, &Motor2->left_wheel.r,
                                   WHEEL_DIST_SQUARED) ||
            points_within_distance(&Motor1->left_wheel.r, &Motor2->right_wheel.r,
                                   WHEEL_DIST_SQUARED) ||
            points_within_distance(&Motor1->right_wheel.r, &Motor2->left_wheel.r,
                                   WHEEL_DIST_SQUARED) ||
            points_within_distance(&Motor1->right_wheel.r, &Motor2->right_wheel.r,
                                   WHEEL_DIST_SQUARED)) {
            // Wheels touching
            FlagTagImmunity = true;
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
