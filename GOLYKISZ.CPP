#include "all.h"

double megkeresidot(int* xlegjobb, int* ylegjobb) {
    double eddigilegjobbt = Hosszuido;
    int x, y;

    *xlegjobb = 0;
    *ylegjobb = 1;

    for (x = 0; x < Ngolyok; x++) {
        // Golyo-Golyo:
        for (y = x + 1; y < Ngolyok; y++) {
            if (Idok[x][y] < -1) {
                Idok[x][y] = ggutkozesideje(&Golyok[x], &Golyok[y]);
            }
            if (Idok[x][y] < eddigilegjobbt) {
                eddigilegjobbt = Idok[x][y];
                *xlegjobb = x;
                *ylegjobb = y;
            }
        }
        // Golyo-Oldal:
        for (y = Ngolyok; y < Ngolyok + 4; y++) {
            if (Idok[x][y] < -1) {
                Idok[x][y] = goutkozesideje(&Golyok[x], y - Ngolyok);
            }
            if (Idok[x][y] < eddigilegjobbt) {
                eddigilegjobbt = Idok[x][y];
                *xlegjobb = x;
                *ylegjobb = y;
            }
        }
    }
    return Idok[*xlegjobb][*ylegjobb];
}

void utkoztet(int x, int y) {
    int i;

    // Golyo-Golyo:
    if (y < Ngolyok) {
        ggutkozes(&Golyok[x], &Golyok[y], Idok[x][y]);
        // Oszlopokat es sorokat ervenytelenit:
        for (i = 0; i < Ngolyok + 4; i++) {
            Idok[x][i] = -2;
            Idok[y][i] = -2;
            if (i < Ngolyok) {
                Idok[i][x] = -2;
                Idok[i][y] = -2;
            }
        }
        return;
    }
    // Golyo-Oldal:
    goutkozes(&Golyok[x], y - Ngolyok, Idok[x][y]);
    // Oszlopot ervenytelenit:
    for (i = 0; i < Ngolyok + 4; i++) {
        Idok[x][i] = -2;
        if (i < Ngolyok) {
            Idok[i][x] = -2;
        }
    }
}
