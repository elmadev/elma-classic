#ifndef VECT2_H
#define VECT2_H

class vect2 {
  public:
    double x;
    double y;
    vect2();
    vect2(double x, double y);
    vect2 operator+(const vect2&) const;
    vect2 operator-(const vect2&) const;
    double operator*(const vect2&) const;
    void normalize();
    void rotate(double rotation);
    double length() const;
};

vect2 operator*(double x, const vect2& a);
vect2 operator*(const vect2& a, double x);
vect2 unit_vector(const vect2 a);
vect2 rotate_90deg(const vect2& in);
vect2 rotate_minus90deg(const vect2& in);

// The intersection point of two infinite lines.
vect2 intersection(const vect2& r1, vect2 v1, vect2 r2, vect2 v2);
// The distance between a point and a line segment.
double point_segment_distance(const vect2& point_r, const vect2& segment_r, const vect2& segment_v);
// The distance between a point and an infinite line.
double point_line_distance(const vect2& point_r, const vect2& segment_r, const vect2& segment_v);
// One intersection point of two circles.
vect2 circles_intersection(const vect2& r1, const vect2& r2, double l1, double l2);

// Returns true if two segments intersect.
bool segments_intersect(const vect2& r1, const vect2& v1, const vect2& r2, const vect2& v2);
// Returns true if two segments intersect or if they are close.
bool segments_intersect_inexact(const vect2& r1, const vect2& v1, const vect2& r2, const vect2& v2);

// Returns true if an infinite line and a circle intersect.
// The intersection point is copied to `intersection_point`.
bool line_circle_intersection(const vect2& line_r, vect2 line_v, const vect2& circle_r,
                              double radius, vect2* intersection_point);

extern vect2 Vect2i, Vect2j, Vect2null;

#endif
