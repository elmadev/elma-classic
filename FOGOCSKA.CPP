#include "all.h"

int Aafogo = 0, Kozelvannak = 0;
int Aleszfogo = 1; // Legkozelebbi jateknal
double Afogoido = 0.0, Bfogoido = 0.0;

// Ez nem publikus:
double Ucsoido = -1.0;

void fogocskareset(void) {
    Aafogo = Aleszfogo;
    Aleszfogo = !Aleszfogo;
    Kozelvannak = 1;
    Afogoido = 0.0, Bfogoido = 0.0;
    Ucsoido = 0.0;
}

static int kozelvan(vekt2* pa, vekt2* pb, double tavnegyzet) {
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

void fogocska(double t) {
    if (Kozelvannak) {
        // Eddig kozel voltak:
        if (!kozelvan(&Pmot1->kor2.r, &Pmot2->kor2.r, Fogonagytavnegyzet) &&
            !kozelvan(&Pmot1->kor2.r, &Pmot2->kor4.r, Fogonagytavnegyzet) &&
            !kozelvan(&Pmot1->kor4.r, &Pmot2->kor2.r, Fogonagytavnegyzet) &&
            !kozelvan(&Pmot1->kor4.r, &Pmot2->kor4.r, Fogonagytavnegyzet)) {
            // Most mar elegendoen tavol vannak:
            Kozelvannak = 0;
        }
    } else {
        // Eddig tavol voltak:
        if (kozelvan(&Pmot1->kor2.r, &Pmot2->kor2.r, Fogokistavnegyzet) ||
            kozelvan(&Pmot1->kor2.r, &Pmot2->kor4.r, Fogokistavnegyzet) ||
            kozelvan(&Pmot1->kor4.r, &Pmot2->kor2.r, Fogokistavnegyzet) ||
            kozelvan(&Pmot1->kor4.r, &Pmot2->kor4.r, Fogokistavnegyzet)) {
            // Megerintettek egymast:
            Kozelvannak = 1;
            Aafogo = !Aafogo;
        }
    }

    // Idot noveljuk:
    if (Aafogo) {
        Afogoido += t - Ucsoido;
    } else {
        Bfogoido += t - Ucsoido;
    }

    Ucsoido = t;
}

// Aafogo alapjan csak idoket allitja:
void fogocska_r(double t) {
    // Idot noveljuk:
    if (Aafogo) {
        Afogoido += t - Ucsoido;
    } else {
        Bfogoido += t - Ucsoido;
    }

    Ucsoido = t;
}
