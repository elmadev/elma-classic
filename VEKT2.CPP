#include "ALL.H"

vekt2 Vekt2i(1.0, 0.0), Vekt2j(0.0, 1.0), Vekt2null(0.0, 0.0);

vekt2 operator*(double x, vekt2 a) { return vekt2(a.x * x, a.y * x); }

vekt2 operator*(vekt2 a, double x) { return vekt2(a.x * x, a.y * x); }

vekt2::vekt2(void) { x = y = 0; }

vekt2::vekt2(double xp, double yp) {
    x = xp;
    y = yp;
}

vekt2 vekt2::operator+(vekt2 a) { return vekt2(x + a.x, y + a.y); }

vekt2 vekt2::operator-(vekt2 a) { return vekt2(x - a.x, y - a.y); }

double vekt2::operator*(vekt2 a) { return x * a.x + y * a.y; }

void vekt2::rotate(double rotation) {
    double a = sin(rotation);
    double b = cos(rotation);
    double xo = x;
    x = b * x - a * y;
    y = a * xo + b * y;
}

double abs(vekt2 a) { return square_root(a.x * a.x + a.y * a.y); }

vekt2 unit_vector(vekt2 a) { return a * (1 / abs(a)); }

void vekt2::normalize() {
    double recip = 1 / abs(*this);
    x *= recip;
    y *= recip;
}

vekt2 rotate_90deg(vekt2 in) { return vekt2(-in.y, in.x); }

vekt2 rotate_minus90deg(vekt2 in) { return vekt2(in.y, -in.x); }

double square_root(double a) {
    if (a < 0) {
        hiba("square_root() of a negative number!");
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

vekt2 intersection(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2) {
    vekt2 n = rotate_90deg(v1);
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

double point_segment_distance(vekt2 point_r, vekt2 segment_r, vekt2 segment_v) {
    vekt2 rr = point_r - segment_r;
    double scalar = segment_r * rr;
    if (scalar <= 0) {
        // Distance to the first point.
        return abs(rr);
    }
    if (scalar >= segment_v * segment_v) {
        // Distance to the second point.
        return abs(rr - segment_v);
    }
    // Distance from the segment.
    vekt2 n = rotate_90deg(unit_vector(segment_v));
    return fabs(n * rr);
}

double point_line_distance(vekt2 point_r, vekt2 segment_r, vekt2 segment_v) {
    vekt2 rr = point_r - segment_r;
    vekt2 n = rotate_90deg(unit_vector(segment_v));
    return fabs(n * rr);
}

vekt2 circles_intersection(vekt2 r1, vekt2 r2, double l1, double l2) {
    vekt2 v = r2 - r1;
    double l = abs(v);
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
    vekt2 v_unit = v * (1 / l);
    vekt2 normal = rotate_90deg(v_unit);

    double x = (l1 * l1 - l2 * l2 + l * l) / (2.0 * l);
    double m = square_root(l1 * l1 - x * x);

    vekt2 r = r1 + x * v_unit + m * normal;
    return r;
}

// Returns true if the infinite line `v1` and segment `r2`, `v2` intersect.
static bool line_segment_intersects(vekt2 v1, vekt2 r2, vekt2 v2) {
    vekt2 norm = rotate_90deg(v1);
    int first_side = r2 * norm > 0;
    int second_side = (r2 + v2) * norm > 0;
    return (first_side && !second_side) || (!first_side && second_side);
}

// Returns true if the infinite line `v1` and segment `r2`, `v2` intersect.
// This is inexact and will return true if the line and segment are close.
static bool line_segment_intersects_inexact(vekt2 v1, vekt2 r2, vekt2 v2) {
    static double epsilon = 0.00000001;
    vekt2 norm = rotate_90deg(v1);
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

bool segments_intersect(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2) {
    return line_segment_intersects(v1, r2 - r1, v2) && line_segment_intersects(v2, r1 - r2, v1);
}

bool segments_intersect_inexact(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2) {
    return line_segment_intersects_inexact(v1, r2 - r1, v2) &&
           line_segment_intersects_inexact(v2, r1 - r2, v1);
}

bool line_circle_intersection(vekt2 line_r, vekt2 line_v, vekt2 circle_r, double radius,
                              vekt2* intersection_point) {
    vekt2 r = circle_r - line_r;
    line_v.normalize();
    vekt2 k = line_v * (line_v * r);
    double distance = point_line_distance(circle_r, line_r, line_v);
    double squared = radius * radius - distance * distance;
    if (squared < 0.0) {
        return false;
    }
    double t = sqrt(squared);
    *intersection_point = line_r + k - line_v * t;
    return true;
}
