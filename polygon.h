#ifndef GYURU_H
#define GYURU_H

#include "vect2.h"
#include <cstdio>

class level;

class polygon {
    int allocated_vertex_count;

  public:
    int vertex_count;
    vect2* vertices;
    int is_grass; // int for read/write compatibility

    // Create a default polygon (New level polygon).
    polygon();
    polygon(FILE* h, int version);
    ~polygon();

    void save(FILE* h, level* lev);
    void set_vertex(int v, double x, double y) const;
    void render_one_line(int v, bool forward, bool dotted) const;
    void render_outline();
    // Inserts a new vertex at `v`, duplicating the vertex at `v`.
    // Returns true if successful.
    bool insert_vertex(int v);
    void delete_vertex(int v);
    // Return the distance of the closest vertex to
    // coordinates `x`, `y` and store the vertex number in
    // `v`.
    double get_closest_vertex(double x, double y, int* v) const;
    // Count the number of times the line r1->v1 intersects with the polygon.
    int count_intersections(vect2 r1, vect2 v1) const;
    // Return true if line r1->v1 intersects with the polygon
    // and stores in the intersection point in
    // `intersect_point`. Ignore the lines that connect to
    // index skip_v.
    bool intersection_point(vect2 r1, vect2 v1, int skip_v, vect2* intersect_point) const;
    // Return true if a polygon is clockwise, or false if clockwise.
    // Returns false if polygon is self-intersecting.
    // This will also move vertices to fix angles that are too small.
    bool is_clockwise() const;
    // Return the centerpoint of a polygon.
    vect2 center() const;
    // Update the bounding box [x1, y1, x2, y2] so that it
    // contains all the points of this polygon.
    void update_boundaries(double* x1, double* y1, double* x2, double* y2) const;
    double checksum() const;
    // Ensure that vertices are not stacked upon each other too closely.
    void separate_stacked_vertices() const;
};

#endif
