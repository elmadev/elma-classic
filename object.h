#ifndef OBJECT_H
#define OBJECT_H

#include "vect2.h"
#include <cstdio>

class object {
  public:
    enum class Type { Exit = 1, Food = 2, Killer = 3, Start = 4 };

    enum class Property {
        None = 0,
        GravityUp = 1,
        GravityDown = 2,
        GravityLeft = 3,
        GravityRight = 4
    };

    vect2 r;                                // Position in meters
    int canvas_x, canvas_y;                 // Position in canvas pixels
    int minimap_canvas_x, minimap_canvas_y; // Position in canvas pixels
    Type type;
    Property property;     // Food gravity
    int animation;         // Food animation (0-8)
    bool active;           // true if visible/interactable
    double floating_phase; // Floating up/down phase, -Pi to Pi

    object(double x, double y, Type typ);
    object(FILE* h, int version);
    // Render object in editor.
    void render();
    void save(FILE* h);
    double checksum();
};

#endif
