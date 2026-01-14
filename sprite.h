#ifndef SPRITE_H
#define SPRITE_H

#include "vect2.h"
#include <cstdio>

#define DEFAULTSIZE (20.0)

#define HATAROL_G (1)
#define HATAROL_S (2)

class sprite {
  public:
    // Ha nev nem ervenyes, akkor [0] == 0:
    // Ha kepnev ervenyes, akkor masik kettonek uresnek kell lennie:
    char kepnev[10];
    char texturanev[10];
    char maszknev[10];
    vect2 r;
    double xsize, ysize;
    // Ha override, akkor tavolsagot es hatarolast nem irja felul LGR alapjan:
    int tavolsag;
    int hatarol; // 0, HATAROL_F, HATAROL_B

    sprite(double x, double y, char* kepnev_p, char* texturanev_p, char* maszknev_p);
    sprite(FILE* h);
    void kirajzol(void);
    void save(FILE* h);
    double belyegszamitas(void);
};

#endif
