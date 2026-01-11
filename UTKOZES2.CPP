#include "UTKOZES2.H"
#include "ECSET.H"
#include "EDITUJ.H"
#include "LEJATSZO.H"
#include "main.h"
#include "object.h"
#include "physics_init.h"
#include "TOPOL.H"
#include "vect2.h"

int utkozikesprite(vect2 r, double sugar) {
    int balalsox, balalsoy;
    if (!Pecsetalso) {
        internal_error("uovruor");
    }
    Pecsetalso->getbalalso_int(r, &balalsox, &balalsoy);
    int objminx = balalsox - 200;
    int objminy = balalsoy - 200;
    int objmaxx = balalsox + 200;
    int objmaxy = balalsoy + 200;

    for (int i = 0; i < MAXKEREK; i++) {
        object* pker = Ptop->kerektomb[i];
        if (!pker) {
            break;
        }

        if (!pker->active) {
            continue;
        }

        if (pker->type == object::Type::Exit && !Single && Tag) {
            continue; // Flag Tag-ben nincsen cel
        }

        if (pker->canvas_x < objminx || pker->canvas_y < objminy || pker->canvas_x > objmaxx ||
            pker->canvas_y > objmaxy) {
            continue;
        }

        vect2 diff = r - pker->r;
        double maxtav = sugar + ObjectRadius;
        if (diff.x * diff.x + diff.y * diff.y < maxtav * maxtav) {
            return i;
        }
    }
    return -1;
}
