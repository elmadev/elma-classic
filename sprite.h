#ifndef SPRITE_H
#define SPRITE_H

#include "vect2.h"
#include <cstdio>

#define DEFAULT_SPRITE_WIREFRAME (20.0)

#define HATAROL_G (1)
#define HATAROL_S (2)

class sprite {
  public:
    // Ha nev nem ervenyes, akkor [0] == 0:
    // Ha kepnev ervenyes, akkor masik kettonek uresnek kell lennie:
    char picture_name[10];
    char texture_name[10];
    char mask_name[10];
    vect2 r;
    double wireframe_width, wireframe_height;
    // Ha override, akkor tavolsagot es hatarolast nem irja felul LGR alapjan:
    int distance;
    int clipping; // 0, HATAROL_F, HATAROL_B

    sprite(double x, double y, char* pic_name, char* text_name, char* mask_nam);
    sprite(FILE* h);
    void render(void);
    void save(FILE* h);
    double checksum(void);
};

#endif
