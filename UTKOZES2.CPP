#include "all.h"

int utkozikesprite(vekt2 r, double sugar) {
    int balalsox, balalsoy;
    if (!Pecsetalso) {
        hiba("uovruor");
    }
    Pecsetalso->getbalalso_int(r, &balalsox, &balalsoy);
    int objminx = balalsox - 200;
    int objminy = balalsoy - 200;
    int objmaxx = balalsox + 200;
    int objmaxy = balalsoy + 200;

    for (int i = 0; i < MAXKEREK; i++) {
        kerek* pker = Ptop->kerektomb[i];
        if (!pker) {
            break;
        }

        if (!pker->aktiv) {
            continue;
        }

        if (pker->tipus == T_CEL && !Single && Tag) {
            continue; // Flag Tag-ben nincsen cel
        }

        if (pker->rxint < objminx || pker->ryint < objminy || pker->rxint > objmaxx ||
            pker->ryint > objmaxy) {
            continue;
        }

        vekt2 diff = r - pker->r;
        double maxtav = sugar + Objektumsugar;
        if (diff.x * diff.x + diff.y * diff.y < maxtav * maxtav) {
            return i;
        }
    }
    return -1;
}
