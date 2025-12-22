#ifndef VEKT2_H
#define VEKT2_H

class vekt2 {
  public:
    double x, y;
    vekt2(void);
    vekt2(double, double);
    vekt2 operator+(vekt2);
    vekt2 operator-(vekt2);
    double operator*(vekt2);
    void normalize(void);
    void rotate(double rotation);
};

vekt2 operator*(double, vekt2);
vekt2 operator*(vekt2, double);
double abs(vekt2);
vekt2 unit_vector(vekt2);
vekt2 rotate_90deg(vekt2);
vekt2 rotate_minus90deg(vekt2);

// Sajat negyzetgyok:
double square_root(double x);

// KULONBOZO SZAMITASOK:

// Az r1-bol indulo v1 vektor altal es az r2-bol indulo v2 altal
// meghatarozott ket egyenes metszespontjanak visszaadasa:
// Parhuzamosok eseten hiba( char* ); fv.-t hivja meg.
vekt2 intersection(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2);
// Pont szakasztol valo tavolsagat szamolja ki:
// (Szakasz hossza vsz hossza).
double point_segment_distance(vekt2 point_r, vekt2 segment_r, vekt2 segment_v);
// Pont egyenestol valo tavolsagat szamolja ki:
double point_line_distance(vekt2 point_r, vekt2 segment_r, vekt2 segment_v);
// Visszaadja ket kor metszetet, r1-bol r2-be bal oldalon (Ganyolt):
vekt2 circles_intersection(vekt2 r1, vekt2 r2, double l1, double l2);

// Igaz, ha az r1-bol indulo v1 vektor es az r2-bol indulo v2 vektor
// metszi egymast:
bool segments_intersect(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2);
// Ez mar akkor is metszesnek tekinti, ha kozel vannak egymashoz:
bool segments_intersect_inexact(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2);

// Igazzal ter vissza, ha metszi kort egyenes:
// Ket metszespont kozul a v iranyaba haladva az elsot adja vissza:
bool line_circle_intersection(vekt2 line_r, vekt2 line_v, vekt2 circle_r, double radius,
                              vekt2* intersection_point);

extern vekt2 Vekt2i, Vekt2j, Vekt2null;

// Ezt meg kell irni hozza:
// Uzenetet kuld peldaul 0-val valo osztas eseten (inverz matrix).
// void hiba( char* message, char* text2 = NULL );

#endif
