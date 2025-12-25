#ifndef DISPLAY_H
#define DISPLAY_H

// int koordinatak mindig kepernyo bal-felso sarkahoz vannak viszonyitva,
// tehat le kell takarni Sav-okat.

// Konvertalasok:
double konvx_i2d(int x);
double konvy_i2d(int y);
int konvx_d2i(double x);
int konvy_d2i(double y);

vect2 konv(int x, int y);
int konvx(vect2 v);
int konvy(vect2 v);

// Minden zoom-ot ez allit (csak indokolt esetben szabad hasznalni):
void zoom(vect2 kozep, double xsize);
void zoomout(void);
void zoomin(int x1, int y1, int x2, int y2);
void kitolt(void); // Palya pont beleferjen kepbe

double getzoomvalue(void);
// Visszaadja kepernyon tiz pixel tavolsagat double leptekben:
double display_double_kozeltav(void);

extern int Vonalcsakeditbe;
void vonalrajz(vect2 p1, vect2 p2, int szaggatott);

extern int Savx;
extern int Savy;

#endif
