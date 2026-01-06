#ifndef GYURU_H
#define GYURU_H

#include "vect2.h"
#include <cstdio>

class topol;

#define POLYGON_MAX_VERTICES (1000) // Ennyit csak akkor foglal le, ha kell

class polygon {
    int allocated_vertex_count;

  public:
    int vertex_count;
    vect2* vertices;

    // Koveto valtozok:
    int is_grass;

    // Fuggvenyek:
    polygon(void);
    polygon(FILE* h, int version);
    ~polygon(void);

    int get_vertex_count(void);
    void save(FILE* h, topol* lev);
    void set_vertex(int v, double x, double y);
    void render_one_line(int v, int forward, int dotted = 0);
    void render_outline(void);
    // Sikeres-e azt adja vissza:
    int insert_vertex(int v); // k pont utan beszur egyet ugyanolyan koordokkal
    void delete_vertex(int v);
    double get_closest_vertex(double x, double y, int* v);
    int count_intersections(vect2 r1, vect2 v1);
    // Ez csak akkor tekinti metszesnek, ha nem csak eppen erintik egymast:
    int intersection_point(vect2 r1, vect2 v1, int skip_v, vect2* intersect_point);
    // akkor ad igazat, ha korbejaraskor poligon belseje bal oldalon van;
    int is_clockwise(void);
    vect2 center(void);
    // Beadott px1, stb-bol indul ki:
    void update_boundaries(double* x1, double* y1, double* x2, double* y2);
    double checksum(void);
    void separate_stacked_vertices(void);
};

#endif
