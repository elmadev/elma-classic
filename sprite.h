#ifndef SPRITE_H
#define SPRITE_H

#include "vect2.h"
#include <cstdio>

#define DEFAULT_SPRITE_WIREFRAME (20.0)

#define HATAROL_G (1)
#define HATAROL_S (2)

class sprite {
  public:
    char picture_name[10];
    char texture_name[10];
    char mask_name[10];
    vect2 r;
    double wireframe_width, wireframe_height;
    int distance;
    int clipping;

    sprite(double x, double y, char* pic_name, char* text_name, char* mask_nam);
    sprite(FILE* h);
    // Render sprite in editor.
    void render(void);
    void save(FILE* h);
    double checksum(void);
};

#endif
