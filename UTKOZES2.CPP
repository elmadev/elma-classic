#include "UTKOZES2.H"
#include "ECSET.H"
#include "EDITUJ.H"
#include "LEJATSZO.H"
#include "main.h"
#include "object.h"
#include "physics_init.h"
#include "TOPOL.H"
#include "vect2.h"

int get_touching_object(vect2 r, double radius) {
    int balalsox, balalsoy;
    if (!Pecsetalso) {
        internal_error("uovruor");
    }
    Pecsetalso->getbalalso_int(r, &balalsox, &balalsoy);
    int objminx = balalsox - 200;
    int objminy = balalsoy - 200;
    int objmaxx = balalsox + 200;
    int objmaxy = balalsoy + 200;

    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* obj = Ptop->objects[i];
        if (!obj) {
            break;
        }

        // Skip Start object and eaten Food
        if (!obj->active) {
            continue;
        }

        // Skip Exit in flagtag mode
        if (obj->type == object::Type::Exit && !Single && Tag) {
            continue;
        }

        if (obj->canvas_x < objminx || obj->canvas_y < objminy || obj->canvas_x > objmaxx ||
            obj->canvas_y > objmaxy) {
            continue;
        }

        vect2 diff = r - obj->r;
        double max_distance = radius + ObjectRadius;
        if (diff.x * diff.x + diff.y * diff.y < max_distance * max_distance) {
            return i;
        }
    }

    return -1;
}
