#include "all.h"

vekt2 Vekt2i(1.0, 0.0), Vekt2j(0.0, 1.0), Vekt2null(0.0, 0.0);
double Pi = 3.141592;

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

double vekt2::operator%(vekt2 b) { return x * b.y - y * b.x; }

void vekt2::forgatas(double alfa) {
    double a = sin(alfa);
    double b = cos(alfa);
    double xo = x;
    x = b * x - a * y;
    y = a * xo + b * y;
}

double abs(vekt2 a) { return gyok(a.x * a.x + a.y * a.y); }

double absnegyzet(vekt2 a) { return a.x * a.x + a.y * a.y; }

vekt2 egys(vekt2 a) { return a * (1 / abs(a)); }

void vekt2::normal(void) {
    double recabs = 1 / abs(*this);
    x *= recabs;
    y *= recabs;
}

vekt2 forgatas90fokkal(vekt2 in) { return vekt2(-in.y, in.x); }

vekt2 forgatasminusz90fokkal(vekt2 in) { return vekt2(in.y, -in.x); }

// personal square root function:
double gyok(double a) {
    if (a < 0) {
        hiba("Square root of negative number!");
        return 1;
    }
    double x1 = sqrt(a);
    if (x1 == 0) {
        return 0;
    }
    return .5 * (x1 + a / x1);
}

// returns the intersection point of lines
// determined by the v1 vector with r1 origin and v2 vector with r2 origin
// calls hiba( char* ) in case of parallel lines
// the formula is as follows: p = r2-v2*(n*(r2-r1))/(n*v2), where n is perpendicular to v1
vekt2 intersection(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2) {
    vekt2 n = forgatas90fokkal(v1);
    double nv2 = n * v2;
    if (fabs(nv2) < 0.00000001) {
        // we return one of two endpoints of the two parallel vectors
        if (v1 * v2 < 0) {
            r2 = r2 + v2;
            v2 = Vekt2null - v2; // this will no longer be used
        }
        // now v1 and v2 have the same orientation
        if ((r2 - r1) * v1 > 0) {
            return r2;
        } else {
            return r1;
        }
    }
    v1.normal();
    v2.normal();
    nv2 = n * v2;
    double nr21 = n * (r2 - r1);
    return r2 - v2 * (nr21 / nv2);
}

// calculates the distance of a point from a line segment (vsz = length of segment) 
double pontszakasztav(vekt2 rpont, vekt2 rsz, vekt2 vsz) {
    vekt2 rr = rpont - rsz;
    double skalered = vsz * rr;
    if (skalered <= 0) {
        // returns the distance of the first point:
        return abs(rr);
    }
    if (skalered >= vsz * vsz) {
        // returns the distance of the second point
        return abs(rr - vsz);
    }
    // returns the distance from the line
    vekt2 n = forgatas90fokkal(egys(vsz));
    return fabs(n * rr);
}

// calculates the distance of a point from a line
double pontegyenestav(vekt2 rpont, vekt2 rsz, vekt2 vsz) {
    vekt2 rr = rpont - rsz;
    vekt2 n = forgatas90fokkal(egys(vsz));
    return fabs(n * rr);
}

// returns the intersection of 2 circles, r1 and r2 on the left side (sloppy):
vekt2 ketkormetszete(vekt2 r1, vekt2 r2, double l1, double l2) {
    vekt2 v = r2 - r1;
    double l = abs(v);
    // a bit of slop:
    if (l >= l1 + l2) {
        l = l1 + l2 - 0.000001;
    }
    if (l1 >= l + l2) {
        l1 = l + l2 - 0.00001;
    }
    if (l2 >= l + l1) {
        l2 = l + l1 - 0.00001;
    }
    vekt2 vegys = v * (1 / l);
    vekt2 normal = forgatas90fokkal(vegys);

    double x = (l1 * l1 - l2 * l2 + l * l) / (2.0 * l);
    double m = gyok(l1 * l1 - x * x);

    vekt2 r = r1 + x * vegys + m * normal;
    return r;
}

// returns true if r2,v2 intersects the line defined by v1,
// so if the origin and end points of v2 lie on different sides of the line
static int ketoldalon(vekt2 v1, vekt2 r2, vekt2 v2) {
    vekt2 norm = forgatas90fokkal(v1);
    int elso = r2 * norm > 0;
    int masodik = (r2 + v2) * norm > 0;
    if ((elso && !masodik) || (!elso && masodik)) {
        return 1;
    }
    return 0;
}

// returns true if r2,v2 intersects the line defined by v1,
// so if the origin and end points of v2 lie on different sides of the line
// this function doesn't consider intersections where the two vectors are barely touching(?)
static int ketoldalon2(vekt2 v1, vekt2 r2, vekt2 v2) {
    static double kishatar = 0.0000000001;
    vekt2 norm = forgatas90fokkal(v1);
    int elso = 0;
    double elsoertek = r2 * norm;
    if (elsoertek > kishatar) {
        elso = 1;
    }
    if (elsoertek < -kishatar) {
        elso = -1;
    }
    int masodik = 0;
    double masodikertek = (r2 + v2) * norm;
    if (masodikertek > kishatar) {
        masodik = 1;
    }
    if (masodikertek < -kishatar) {
        masodik = -1;
    }
    if ((elso == -1 && masodik == 1) || (elso == 1 && masodik == -1)) {
        return 1;
    }
    return 0;
}

// returns true if r2,v2 intersects the line defined by v1,
// so if the origin and end points of v2 lie on different sides of the line
// this function is more lenient on intersections
static int ketoldalon3(vekt2 v1, vekt2 r2, vekt2 v2) {
    static double kishatar = 0.00000001;
    vekt2 norm = forgatas90fokkal(v1);
    int elso = 0;
    double elsoertek = r2 * norm;
    if (elsoertek > kishatar) {
        elso = 1;
    }
    if (elsoertek < -kishatar) {
        elso = -1;
    }
    int masodik = 0;
    double masodikertek = (r2 + v2) * norm;
    if (masodikertek > kishatar) {
        masodik = 1;
    }
    if (masodikertek < -kishatar) {
        masodik = -1;
    }
    if ((elso == -1 && masodik == -1) || (elso == 1 && masodik == 1)) {
        return 0;
    }
    return 1;
}

// returns true if v1 vector with r1 origin and v2 vector with r2 origin intersect
int vektormetszes(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2) {
    if (ketoldalon(v1, r2 - r1, v2) && ketoldalon(v2, r1 - r2, v1)) {
        return 1;
    }
    return 0;
}

// returns true if v1 vector with r1 origin and v2 vector with r2 origin intersect
// this function doesn't consider intersections where the two vectors are barely touching(?)
int vektormetszes2(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2) {
    if (ketoldalon2(v1, r2 - r1, v2) && ketoldalon2(v2, r1 - r2, v1)) {
        return 1;
    }
    return 0;
}

// this function is more lenient on intersections
int vektormetszes3(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2) {
    if (ketoldalon3(v1, r2 - r1, v2) && ketoldalon3(v2, r1 - r2, v1)) {
        return 1;
    }
    return 0;
}

// returns true if a line intersects a circle
// of the two intersection points, it returns the first one when moving towards v:
int egyeneskormetszes(vekt2 p1, vekt2 v, vekt2 r_kor, double sugar, vekt2* p) {
    vekt2 r = r_kor - p1;
    v.normal();
    vekt2 k = v * (v * r); // from p1, it points towards the closest point on the line
    double tav = pontegyenestav(r_kor, p1, v);
    double gyokalatt = sugar * sugar - tav * tav;
    if (gyokalatt < 0.0) {
        return 0;
    }
    double t = sqrt(gyokalatt);
    *p = p1 + k - v * t;
    return 1;
}
