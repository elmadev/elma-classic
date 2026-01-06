#ifndef GYURU_H
#define GYURU_H

#include "vect2.h"
#include <cstdio>

class topol;

#define POLYGON_MAX_VERTICES (1000)

class polygon {
    int allocated_vertex_count;

  public:
    int vertex_count;
    vect2* vertices;
    int is_grass; // int for read/write compatibility

    // Create a default polygon (New level polygon).
    polygon(void);
    polygon(FILE* h, int version);
    ~polygon(void);

    int get_vertex_count(void);
    void save(FILE* h, topol* lev);
    void set_vertex(int v, double x, double y);
    void render_one_line(int v, int forward, int dotted = 0);
    void render_outline(void);
    // Inserts a new vertex at `v`, duplicating the vertex at `v`.
    // Returns true if successful.
    int insert_vertex(int v);
    void delete_vertex(int v);
    // Return the distance of the closest vertex to
    // coordinates `x`, `y` and store the vertex number in
    // `v`.
    double get_closest_vertex(double x, double y, int* v);
    // Count the number of times the line r1->v1 intersects with the polygon.
    int count_intersections(vect2 r1, vect2 v1);
    // Return true if line r1->v1 intersects with the polygon
    // and stores in the intersection point in
    // `intersect_point`. Ignore the lines that connect to
    // index skip_v.
    int intersection_point(vect2 r1, vect2 v1, int skip_v, vect2* intersect_point);
    // Return true if a polygon is clockwise, or false if clockwise.
    // Returns false if polygon is self-intersecting.
    // This will also move vertices to fix angles that are too small.
    int is_clockwise(void);
    // Return the centerpoint of a polygon.
    vect2 center(void);
    // Update the bounding box [x1, y1, x2, y2] so that it
    // contains all the points of this polygon.
    void update_boundaries(double* x1, double* y1, double* x2, double* y2);
    double checksum(void);
    // Ensure that vertices are not stacked upon each other too closely.
    void separate_stacked_vertices(void);
};

#endif
