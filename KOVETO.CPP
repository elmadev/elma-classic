#include "all.h"

koveto::koveto(void) {
    // Nullazas:
    kepszam = 0;
    for (int i = 0; i < MAXKOVETOKEPSZAM; i++) {
        pkeptomb[i] = NULL;
        feltomb[i] = 0;
    }
}

// nev kiterjesztes nelkuli:
void koveto::addkep(pic8* ppic, int up) {
    if (kepszam >= MAXKOVETOKEPSZAM) {
        uzenet("Too many grass pictures in lgr file!");
    }
    pkeptomb[kepszam] = ppic;
    feltomb[kepszam] = up;

    kepszam++;
}

koveto::~koveto(void) {
    for (int i = 0; i < MAXKOVETOKEPSZAM; i++) {
        if (pkeptomb[i]) {
            delete pkeptomb[i];
            pkeptomb[i] = NULL;
        }
    }
}

// cur-t adja vissza (ami mindig utolso jol kitoltott hely):
// Elso alkalommal kitolti *px0-at:
static int feldolgvonalszakasz(gyuru* pgy, int p1, int p2, int* px0, int cur, int* ytomb,
                               int maxhely, vekt2* porigo) {
    if (p1 < 0 || p1 >= pgy->pontszam || p2 < 0 || p2 >= pgy->pontszam) {
        hiba("8u9tg4");
    }

    vekt2 r1 = pgy->ponttomb[p1];
    vekt2 r2 = pgy->ponttomb[p2];
    if (r1.x > r2.x) {
        return cur;
    }

    int x1 = (r1.x - porigo->x) * Arany;
    double y1 = (-r1.y - porigo->y) * Arany; // Mivel forditva vannak:
    int x2 = (r2.x - porigo->x) * Arany;
    double y2 = (-r2.y - porigo->y) * Arany;

    if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0) {
        hiba("7yces");
    }

    if (cur < 0) {
        // Elso jo hivas:
        cur = x1;
        if (*px0 >= 0) {
            hiba("kjiut67");
        }
        *px0 = x1;
        ytomb[0] = y1;
    }

    if (x1 >= x2) {
        return cur;
    }
    if (x1 - *px0 >= maxhely) {
        return cur;
    }

    if (cur < x1 - 1) {
#ifdef TEST
        hiba("o96ytuyg");
#endif
        return cur;
    }

    for (int x = x1; x <= x2; x++) {
        if (x < cur) {
            continue;
        }
        if (x - *px0 >= maxhely) {
            return cur;
        }
        double y = y1 + (y2 - y1) * ((double)x - x1) / (x2 - x1);
        ytomb[x - *px0] = y;
        cur = x;
    }
    return cur;
}

// Ez nem hasznal semmit koveto adatokbol:
int makevonal(gyuru* pgy, int* ytomb, int* phossz, int* px0, int maxhely, vekt2* porigo) {
    *phossz = 0;
    // Most megkeressuk osszekoto szakaszt:
    double maxxhossz = 0.0;
    int p1 = 0;
    for (int i = 0; i < pgy->pontszam; i++) {
        int j = i + 1;
        if (j == pgy->pontszam) {
            j = 0;
        }
        double ujxhossz = fabs(pgy->ponttomb[i].x - pgy->ponttomb[j].x);
        if (ujxhossz > maxxhossz) {
            p1 = i;
            maxxhossz = ujxhossz;
        }
    }
    if (maxxhossz < 0.0001) {
        return 0;
    }

    int elore = 1;
    int p2 = p1 + 1;
    if (p2 == pgy->pontszam) {
        p2 = 0;
    }
    // Forditva kell nezni, mivel osszekoto szakasz ellentetes:
    if (pgy->ponttomb[p1].x < pgy->ponttomb[p2].x) {
        elore = 0;
    }

    // Vegigmegyunk szakaszokon:
    *px0 = -1;
    int cur = -1;
    for (int i = 0; i < pgy->pontszam - 1; i++) {
        // Eloszor lepunk mert osszekoto szakaszrol indultunk:
        if (elore) {
            p1++;
            if (p1 == pgy->pontszam) {
                p1 = 0;
            }
            p2++;
            if (p2 == pgy->pontszam) {
                p2 = 0;
            }
        } else {
            p1--;
            if (p1 < 0) {
                p1 = pgy->pontszam - 1;
            }
            p2--;
            if (p2 < 0) {
                p2 = pgy->pontszam - 1;
            }
        }
        // Most pedig beetetjuk vonalba szakaszt:
        int pkisebb = p1;
        int pnagyobb = p2;
        if (!elore) {
            pkisebb = p2;
            pnagyobb = p1;
        }

        // Elso alkalommal kitolti *px0-at:
        cur = feldolgvonalszakasz(pgy, pkisebb, pnagyobb, px0, cur, ytomb, maxhely, porigo);
    }

    if (*px0 < 0) {
        return 0;
    }

    *phossz = cur - *px0 + 1;
    return 1;
}
