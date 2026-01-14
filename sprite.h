#ifndef SPRITE_H
#define SPRITE_H

#include "vect2.h"
#include <cstdio>

constexpr int DEFAULT_SPRITE_WIREFRAME = 20;

enum class Clipping {
    Unknown = -1,
    Unclipped = 0,
    Ground = 1,
    Sky = 2,
};

class sprite {
  public:
    char picture_name[10];
    char texture_name[10];
    char mask_name[10];
    vect2 r;
    double wireframe_width;
    double wireframe_height;
    int distance;
    Clipping clipping;

    sprite(double x, double y, const char* pic_name, const char* text_name, const char* mask_nam);
    sprite(FILE* h);
    // Render sprite in editor.
    void render();
    void save(FILE* h);
    double checksum();
};

#endif
