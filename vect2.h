#ifndef VECT2_H
#define VECT2_H

class vect2 {
  public:
    double x;
    double y;
    vect2();
    vect2(double x, double y);
    vect2 operator+(vect2);
    vect2 operator-(vect2);
    double operator*(vect2);
    void normalize();
    void rotate(double rotation);
    double length();
};

vect2 operator*(double x, vect2 a);
vect2 operator*(vect2 a, double x);
vect2 unit_vector(vect2 a);
vect2 rotate_90deg(vect2 in);
vect2 rotate_minus90deg(vect2 in);

// The intersection point of two infinite lines.
vect2 intersection(vect2 r1, vect2 v1, vect2 r2, vect2 v2);
// The distance between a point and a line segment.
double point_segment_distance(vect2 point_r, vect2 segment_r, vect2 segment_v);
// The distance between a point and an infinite line.
double point_line_distance(vect2 point_r, vect2 segment_r, vect2 segment_v);
// One intersection point of two circles.
vect2 circles_intersection(vect2 r1, vect2 r2, double l1, double l2);

// Returns true if two segments intersect.
bool segments_intersect(vect2 r1, vect2 v1, vect2 r2, vect2 v2);
// Returns true if two segments intersect or if they are close.
bool segments_intersect_inexact(vect2 r1, vect2 v1, vect2 r2, vect2 v2);

// Returns true if an infinite line and a circle intersect.
// The intersection point is copied to `intersection_point`.
bool line_circle_intersection(vect2 line_r, vect2 line_v, vect2 circle_r, double radius,
                              vect2* intersection_point);

extern vect2 Vekt2i, Vekt2j, Vekt2null;

#endif
