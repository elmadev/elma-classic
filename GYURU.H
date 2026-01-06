#ifndef GYURU_H
#define GYURU_H

#include "vect2.h"
#include <cstdio>

class topol;

#define MAXPONT (1000) // Ennyit csak akkor foglal le, ha kell

class gyuru {
    int lefoglaltmaxpont;

  public:
    int pontszam;
    vect2* ponttomb;

    // Koveto valtozok:
    int koveto;

    // Fuggvenyek:
    gyuru(void);
    gyuru(FILE* h, int kovetos);
    ~gyuru(void);

    int getpontszam(void);
    void save(FILE* h, topol* ptop);
    void ujkoord(int k, double x, double y);
    void kirajz(int k, int fel, int szaggatott = 0);
    void osszrajz(void);
    // Sikeres-e azt adja vissza:
    int beszur(int k); // k pont utan beszur egyet ugyanolyan koordokkal
    void torol(int k);
    double legkozpont(double x, double y, int* pk);
    int metszesszam(vect2 r1, vect2 v1);
    // Ez csak akkor tekinti metszesnek, ha nem csak eppen erintik egymast:
    int metszeshely(vect2 r1, vect2 v1, int kihagy, vect2* pmetszes);
    // akkor ad igazat, ha korbejaraskor poligon belseje bal oldalon van;
    int pozitivkorjaras(void);
    vect2 sulypont(void);
    // Beadott px1, stb-bol indul ki:
    void getminmax(double* px1, double* py1, double* px2, double* py2);
    double belyegszamitas(void);
    void szetvalaszt(void);
};

#endif
