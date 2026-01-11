#ifndef OBJECT_H
#define OBJECT_H

#include "vect2.h"
#include <cstdio>

enum { T_CEL = 1, T_KAJA, T_HALALOS, T_KEZDO };
enum { KT_NORMAL, KT_UP, KT_DOWN, KT_LEFT, KT_RIGHT };

class object {
  public:
    vect2 r;
    int canvas_x, canvas_y;                 // Kaja lejatszas kozbeni kirajzolasahoz
    int minimap_canvas_x, minimap_canvas_y; // Ugyanez view-ra
    int type;
    int property; // Ha kaja, akkor lehet gravitacios is
    int animation;
    int active;            // Lejatszas soran ha kaja, megvan-e meg
    unsigned char szin;    // Csak lejatszasnal kell
    int sinfazisint;       // 0-tol 999-ig mehet
    double floating_phase; // 0-tol 2Pi-ig megy

    object(double x, double y, int typ);
    object(FILE* h, int version);
    void render(void);
    void save(FILE* h);
    double checksum(void);
};

#endif
