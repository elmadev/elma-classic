#include "ALL.H"
#include "physics_init.h"

int FlagTagAHasFlag = 0, FlagTagImmunity = 0;
int FlagTagAStarts = 1; // Legkozelebbi jateknal
double FlagTimeA = 0.0, FlagTimeB = 0.0;

// Ez nem publikus:
double Ucsoido = -1.0;

void flagtag_reset(void) {
    FlagTagAHasFlag = FlagTagAStarts;
    FlagTagAStarts = !FlagTagAStarts;
    FlagTagImmunity = 1;
    FlagTimeA = 0.0, FlagTimeB = 0.0;
    Ucsoido = 0.0;
}

static int kozelvan(vect2* pa, vect2* pb, double tavnegyzet) {
    double dx = pa->x - pb->x;
    double dy = pa->y - pb->y;
    if (dx * dx + dy * dy < tavnegyzet) {
        return 1;
    } else {
        return 0;
    }
}

static double Fogokistavnegyzet = 0.64; // 0.8 a negyzeten
// static double Fogonagytavnegyzet = 3.2; // 1.8 a negyzeten
static double Fogonagytavnegyzet = 1.44; // 1.2 a negyzeten

void flagtag(double time) {
    if (FlagTagImmunity) {
        // Eddig kozel voltak:
        if (!kozelvan(&Motor1->left_wheel.r, &Motor2->left_wheel.r, Fogonagytavnegyzet) &&
            !kozelvan(&Motor1->left_wheel.r, &Motor2->right_wheel.r, Fogonagytavnegyzet) &&
            !kozelvan(&Motor1->right_wheel.r, &Motor2->left_wheel.r, Fogonagytavnegyzet) &&
            !kozelvan(&Motor1->right_wheel.r, &Motor2->right_wheel.r, Fogonagytavnegyzet)) {
            // Most mar elegendoen tavol vannak:
            FlagTagImmunity = 0;
        }
    } else {
        // Eddig tavol voltak:
        if (kozelvan(&Motor1->left_wheel.r, &Motor2->left_wheel.r, Fogokistavnegyzet) ||
            kozelvan(&Motor1->left_wheel.r, &Motor2->right_wheel.r, Fogokistavnegyzet) ||
            kozelvan(&Motor1->right_wheel.r, &Motor2->left_wheel.r, Fogokistavnegyzet) ||
            kozelvan(&Motor1->right_wheel.r, &Motor2->right_wheel.r, Fogokistavnegyzet)) {
            // Megerintettek egymast:
            FlagTagImmunity = 1;
            FlagTagAHasFlag = !FlagTagAHasFlag;
        }
    }

    // Idot noveljuk:
    if (FlagTagAHasFlag) {
        FlagTimeA += time - Ucsoido;
    } else {
        FlagTimeB += time - Ucsoido;
    }

    Ucsoido = time;
}

// FlagTagAHasFlag alapjan csak idoket allitja:
void flagtag_replay(double time) {
    // Idot noveljuk:
    if (FlagTagAHasFlag) {
        FlagTimeA += time - Ucsoido;
    } else {
        FlagTimeB += time - Ucsoido;
    }

    Ucsoido = time;
}
