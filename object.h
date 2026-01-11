#ifndef OBJECT_H
#define OBJECT_H

#include "vect2.h"
#include <cstdio>

enum { T_CEL = 1, T_KAJA, T_HALALOS, T_KEZDO };
enum { KT_NORMAL, KT_UP, KT_DOWN, KT_LEFT, KT_RIGHT };

class object {
  public:
    vect2 r;                                // Position in meters
    int canvas_x, canvas_y;                 // Position in canvas pixels
    int minimap_canvas_x, minimap_canvas_y; // Position in canvas pixels
    int type;
    int property;          // Food gravity
    int animation;         // Food animation (0-8)
    bool active;           // true if visible/interactable
    double floating_phase; // Floating up/down phase, -Pi to Pi

    object(double x, double y, int typ);
    object(FILE* h, int version);
    // Render object in editor.
    void render();
    void save(FILE* h);
    double checksum();
};

#endif
