#ifndef VEKT2_H
#define VEKT2_H

class vekt2 {
  public:
    double x;
    double y;
    vekt2();
    vekt2(double x, double y);
    vekt2 operator+(vekt2);
    vekt2 operator-(vekt2);
    double operator*(vekt2);
    void normalize();
    void rotate(double rotation);
};

vekt2 operator*(double x, vekt2 a);
vekt2 operator*(vekt2 a, double x);
double abs(vekt2);
vekt2 unit_vector(vekt2 a);
vekt2 rotate_90deg(vekt2 in);
vekt2 rotate_minus90deg(vekt2 in);

double square_root(double x);

// The intersection point of two infinite lines.
vekt2 intersection(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2);
// The distance between a point and a line segment.
double point_segment_distance(vekt2 point_r, vekt2 segment_r, vekt2 segment_v);
// The distance between a point and an infinite line.
double point_line_distance(vekt2 point_r, vekt2 segment_r, vekt2 segment_v);
// One intersection point of two circles.
vekt2 circles_intersection(vekt2 r1, vekt2 r2, double l1, double l2);

// Returns true if two segments intersect.
bool segments_intersect(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2);
// Returns true if two segments intersect or if they are close.
bool segments_intersect_inexact(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2);

// Returns true if an infinite line and a circle intersect.
// The intersection point is copied to `intersection_point`.
bool line_circle_intersection(vekt2 line_r, vekt2 line_v, vekt2 circle_r, double radius,
                              vekt2* intersection_point);

extern vekt2 Vekt2i, Vekt2j, Vekt2null;

#endif
