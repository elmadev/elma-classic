#include "ED_CHECK.H"
#include "editor_canvas.h"
#include "editor_dialog.h"
#include "EDITUJ.H"
#include "level.h"
#include "object.h"
#include "polygon.h"

// Visszaadja, hogy volt-e hiba:
int check_topology(int kelldialog) {
    dialog("Checking Topology, please wait!", DIALOG_BUTTONS, DIALOG_RETURN);

    // Eloszor egymason fekvo szomszedos pontokat valasztjuk szet:
    for (int i = 0; i < MAX_POLYGONS; i++) {
        polygon* pgy = Ptop->polygons[i];
        if (pgy) {
            pgy->separate_stacked_vertices();
        }
    }

    // Most megszuntetjuk hegyes szogeket:
    for (int i = 0; i < MAX_POLYGONS; i++) {
        polygon* pgy = Ptop->polygons[i];
        if (pgy && !pgy->is_grass) {
            pgy->is_clockwise(); // Ez automatikusan megcsinalja:
        }
    }

    // Most megnezzuk, hogy nincs-e tul sok pont osszesen:
    int pontszam = 0;
    for (int i = 0; i < MAX_POLYGONS; i++) {
        polygon* pgy = Ptop->polygons[i];
        if (pgy) {
            pontszam += pgy->vertex_count;
        }
    }
    if (pontszam > MAX_VERTICES) {
        if (kelldialog) {
            char tmp1[100];
            char tmp2[100];
            sprintf(tmp1, "Error: The number of vertices must be less than %d!", MAX_VERTICES);
            sprintf(tmp2, "There are %d vertices now in the level.", pontszam);
            dialog(tmp1, tmp2, "Please delete some vertices or polygons from this level!");
            invalidateegesz();
            return 1;
        } else {
            return 1;
        }
    }

    // Most megnezzuk, hogy nincsenek-e egymast metszo vonalak:
    for (int i = 0; i < MAX_POLYGONS; i++) {
        polygon* pgy = Ptop->polygons[i];
        if (pgy && !pgy->is_grass) {
            for (int j = 0; j < pgy->vertex_count; j++) {
                vect2 r = pgy->vertices[j];
                vect2 v;
                if (j == pgy->vertex_count - 1) {
                    v = pgy->vertices[0] - r;
                } else {
                    v = pgy->vertices[j + 1] - r;
                }
                // Megvan r, v szakasz, amirol el kell donteni, hogy van-e
                // metszespontja akarmi massal:
                for (int k = 0; k < MAX_POLYGONS; k++) {
                    polygon* pgy2 = Ptop->polygons[k];
                    if (pgy2 && !pgy2->is_grass) {
                        bool vanmetszes = false;
                        vect2 m;
                        if (pgy == pgy2) {
                            vanmetszes = pgy2->intersection_point(r, v, j, &m);
                        } else {
                            vanmetszes = pgy2->intersection_point(r, v, -1, &m);
                        }
                        if (vanmetszes) {
                            if (kelldialog) {
                                zoom(m, 0.0000001);
                                dialog("Error: Two lines are intersecting each others!",
                                       "After this dialog you will see the intersection.",
                                       "Use Zoomout to see where it is located!");
                                invalidateegesz();
                                return 1;
                            } else {
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }

    // Most megnezzuk, hogy nincs-e kintlevo objektum:
    double x1, y1, x2, y2;
    Ptop->get_boundaries(&x1, &y1, &x2, &y2, false);
    x1 -= 1.0;
    y1 -= 1.0;
    x2 += 1.0;
    y2 += 1.0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* pker = Ptop->objects[i];
        if (pker) {
            if (pker->r.x < x1 || pker->r.x > x2 || pker->r.y < y1 || pker->r.y > y2) {
                if (kelldialog) {
                    zoom(pker->r, 1.5);
                    dialog("Error: An object is outside the level borders!",
                           "After this dialog you will see the object.",
                           "Use Zoomout to see where it is located!");
                    invalidateegesz();
                    return 1;
                } else {
                    return 1;
                }
            }
        }
    }

    // Most megnezzuk, hogy bal kerek nincs-e kint:
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* pker = Ptop->objects[i];
        if (pker && pker->type == object::Type::Start) {
            if (!Ptop->is_sky(NULL, &pker->r)) {
                if (kelldialog) {
                    dialog("Error: The start object, which determines the place of the left wheel "
                           "when you start to",
                           "play on a particular level, is inside ground!",
                           "If you cannot resolve this error message, you should try to move this "
                           "object inside",
                           "ground, because it is possible that you have mixed up which is the "
                           "ground and",
                           "which is the air.", "After this dialog you will see the object.",
                           "Use Zoomout to see where it is located!");
                    zoom(pker->r, 1.5);
                }
                return 1;
            }
        }
    }

    if (kelldialog) {
        dialog("Everything seems to be all right.");
    }
    return 0;
}
