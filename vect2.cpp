#include "vect2.h"
#include "main.h"
#include <cmath>

vect2 Vect2i(1.0, 0.0), Vect2j(0.0, 1.0), Vect2null(0.0, 0.0);

vect2 operator*(double x, const vect2& a) { return vect2(a.x * x, a.y * x); }

vect2 operator*(const vect2& a, double x) { return vect2(a.x * x, a.y * x); }

vect2::vect2(void) { x = y = 0; }

vect2::vect2(double xp, double yp) {
    x = xp;
    y = yp;
}

vect2 vect2::operator+(const vect2& a) const { return vect2(x + a.x, y + a.y); }

vect2 vect2::operator-(const vect2& a) const { return vect2(x - a.x, y - a.y); }

double vect2::operator*(const vect2& a) const { return x * a.x + y * a.y; }

void vect2::rotate(double rotation) {
    double a = sin(rotation);
    double b = cos(rotation);
    double xo = x;
    x = b * x - a * y;
    y = a * xo + b * y;
}

static double square_root(double a) {
    if (a < 0) {
        internal_error("square_root() of a negative number!");
        return 1;
    }
    double x1 = sqrt(a);
    if (x1 == 0) {
        return 0;
    }
    // Apply Newton's method... even though we have a pretty perfect
    // value from sqrt() function.
    return .5 * (x1 + a / x1);
}

double vect2::length() const { return square_root(x * x + y * y); }

vect2 unit_vector(const vect2 a) { return a * (1 / a.length()); }

void vect2::normalize() {
    double recip = 1 / length();
    x *= recip;
    y *= recip;
}

vect2 rotate_90deg(const vect2& in) { return vect2(-in.y, in.x); }

vect2 rotate_minus90deg(const vect2& in) { return vect2(in.y, -in.x); }

vect2 intersection(const vect2& r1, vect2 v1, vect2 r2, vect2 v2) {
    vect2 n = rotate_90deg(v1);
    double nv2 = n * v2;
    if (fabs(nv2) < 0.00000001) {
        // Parallel lines, no true intersection point
        if (v1 * v2 < 0) {
            r2 = r2 + v2;
        }
        if ((r2 - r1) * v1 > 0) {
            return r2;
        } else {
            return r1;
        }
    }
    v1.normalize();
    v2.normalize();
    nv2 = n * v2;
    double nr21 = n * (r2 - r1);
    return r2 - v2 * (nr21 / nv2);
}

double point_segment_distance(const vect2& point_r, const vect2& segment_r,
                              const vect2& segment_v) {
    vect2 rr = point_r - segment_r;
    double scalar = segment_r * rr;
    if (scalar <= 0) {
        // Distance to the first point.
        return rr.length();
    }
    if (scalar >= segment_v * segment_v) {
        // Distance to the second point.
        return (rr - segment_v).length();
    }
    // Distance from the segment.
    vect2 n = rotate_90deg(unit_vector(segment_v));
    return fabs(n * rr);
}

double point_line_distance(const vect2& point_r, const vect2& segment_r, const vect2& segment_v) {
    vect2 rr = point_r - segment_r;
    vect2 n = rotate_90deg(unit_vector(segment_v));
    return fabs(n * rr);
}

vect2 circles_intersection(const vect2& r1, const vect2& r2, double l1, double l2) {
    vect2 v = r2 - r1;
    double l = v.length();
    // If the circles don't intersect, resize the circles so they intersect.
    if (l >= l1 + l2) {
        l = l1 + l2 - 0.000001;
    }
    if (l1 >= l + l2) {
        l1 = l + l2 - 0.00001;
    }
    if (l2 >= l + l1) {
        l2 = l + l1 - 0.00001;
    }
    vect2 v_unit = v * (1 / l);
    vect2 normal = rotate_90deg(v_unit);

    double x = (l1 * l1 - l2 * l2 + l * l) / (2.0 * l);
    double m = square_root(l1 * l1 - x * x);

    vect2 r = r1 + x * v_unit + m * normal;
    return r;
}

// Returns true if the infinite line `v1` and segment `r2`, `v2` intersect.
static bool line_segment_intersects(const vect2& v1, const vect2& r2, const vect2& v2) {
    vect2 norm = rotate_90deg(v1);
    int first_side = r2 * norm > 0;
    int second_side = (r2 + v2) * norm > 0;
    return (first_side && !second_side) || (!first_side && second_side);
}

// Returns true if the infinite line `v1` and segment `r2`, `v2` intersect.
// This is inexact and will return true if the line and segment are close.
static bool line_segment_intersects_inexact(const vect2& v1, const vect2& r2, const vect2& v2) {
    static double epsilon = 0.00000001;
    vect2 norm = rotate_90deg(v1);
    int first_side = 0;
    double first_side_dist = r2 * norm;
    if (first_side_dist > epsilon) {
        first_side = 1;
    }
    if (first_side_dist < -epsilon) {
        first_side = -1;
    }
    int second_side = 0;
    double second_side_dist = (r2 + v2) * norm;
    if (second_side_dist > epsilon) {
        second_side = 1;
    }
    if (second_side_dist < -epsilon) {
        second_side = -1;
    }
    if ((first_side == -1 && second_side == -1) || (first_side == 1 && second_side == 1)) {
        return false;
    }
    return true;
}

bool segments_intersect(const vect2& r1, const vect2& v1, const vect2& r2, const vect2& v2) {
    return line_segment_intersects(v1, r2 - r1, v2) && line_segment_intersects(v2, r1 - r2, v1);
}

bool segments_intersect_inexact(const vect2& r1, const vect2& v1, const vect2& r2,
                                const vect2& v2) {
    return line_segment_intersects_inexact(v1, r2 - r1, v2) &&
           line_segment_intersects_inexact(v2, r1 - r2, v1);
}

bool line_circle_intersection(const vect2& line_r, vect2 line_v, const vect2& circle_r,
                              double radius, vect2* intersection_point) {
    vect2 r = circle_r - line_r;
    line_v.normalize();
    vect2 k = line_v * (line_v * r);
    double distance = point_line_distance(circle_r, line_r, line_v);
    double squared = radius * radius - distance * distance;
    if (squared < 0.0) {
        return false;
    }
    double t = sqrt(squared);
    *intersection_point = line_r + k - line_v * t;
    return true;
}
