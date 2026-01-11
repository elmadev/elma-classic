#ifndef OBJECT_H
#define OBJECT_H

#include "vect2.h"
#include <cstdio>

enum { T_CEL = 1, T_KAJA, T_HALALOS, T_KEZDO };
enum { KT_NORMAL, KT_UP, KT_DOWN, KT_LEFT, KT_RIGHT };

class kerek {
  public:
    vect2 r;
    int rxint, ryint;     // Kaja lejatszas kozbeni kirajzolasahoz
    int rxint_v, ryint_v; // Ugyanez view-ra
    int tipus;
    int kajatipus; // Ha kaja, akkor lehet gravitacios is
    int foodsorszam;
    int aktiv;          // Lejatszas soran ha kaja, megvan-e meg
    unsigned char szin; // Csak lejatszasnal kell
    int sinfazisint;    // 0-tol 999-ig mehet
    double sinfazis;    // 0-tol 2Pi-ig megy

    kerek(double x, double y, int tipus);
    kerek(FILE* h, int verzio);
    void kirajzol(void);
    void save(FILE* h);
    double belyegszamitas(void);
};

#endif
