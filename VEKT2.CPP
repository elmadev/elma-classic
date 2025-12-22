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

void vekt2::normalize(void) {
    double recabs = 1 / abs(*this);
    x *= recabs;
    y *= recabs;
}

vekt2 rotate_90deg(vekt2 in) { return vekt2(-in.y, in.x); }

vekt2 rotate_minus90deg(vekt2 in) { return vekt2(in.y, -in.x); }

// Sajat negyzetgyok:
double square_root(double a) {
    if (a < 0) {
        hiba("Gyokvonas negativ szammal!");
        return 1;
    }
    double x1 = sqrt(a);
    if (x1 == 0) {
        return 0;
    }
    return .5 * (x1 + a / x1);
}

// Az r1-bol indulo v1 vektor altal es az r2-bol indulo v2 altal
// meghatarozott ket egyenes metszespontjanak visszaadasa:
// Parhuzamosok eseten hiba( char* ); fv.-t hivja meg.
// A keplet a kovetkezo: p = r2-v2*(n*(r2-r1))/(n*v2), ahol n meroleges v1-re.
vekt2 intersection(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2) {
    vekt2 n = rotate_90deg(v1);
    double nv2 = n * v2;
    if (fabs(nv2) < 0.00000001) {
        // Ket parhuzamos egyenesrol van szo, visszaadjuk egyiknek a
        // vegpontjat, ami masikon fekszik:
        if (v1 * v2 < 0) {
            r2 = r2 + v2;
        }
        // Most egyiranyba mutat v1 es v2:
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

// Pont szakasztol valo tavolsagat szamolja ki:
// (Szakasz hossza vsz hossza).
double point_segment_distance(vekt2 point_r, vekt2 segment_r, vekt2 segment_v) {
    vekt2 rr = point_r - segment_r;
    double skalered = segment_r * rr;
    if (skalered <= 0) {
        // Elso pont tavolsagat adja vissza:
        return abs(rr);
    }
    if (skalered >= segment_v * segment_v) {
        // Masodik pont tavolsagat adja vissza:
        return abs(rr - segment_v);
    }
    // Egyenestol valo tavolsagat adja vissza:
    vekt2 n = rotate_90deg(unit_vector(segment_v));
    return fabs(n * rr);
}

// Pont egyenestol valo tavolsagat szamolja ki:
double point_line_distance(vekt2 point_r, vekt2 segment_r, vekt2 segment_v) {
    vekt2 rr = point_r - segment_r;
    vekt2 n = rotate_90deg(unit_vector(segment_v));
    return fabs(n * rr);
}

// Visszaadja ket kor metszetet, r1-bol r2-be bal oldalon (Ganyolt):
vekt2 circles_intersection(vekt2 r1, vekt2 r2, double l1, double l2) {
    vekt2 v = r2 - r1;
    double l = abs(v);
    // Egy kis ganyolas:
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
    vekt2 normal = rotate_90deg(vegys);

    double x = (l1 * l1 - l2 * l2 + l * l) / (2.0 * l);
    double m = square_root(l1 * l1 - x * x);

    vekt2 r = r1 + x * vegys + m * normal;
    return r;
}

// Igaz, ha v1 altal meghatarozott egyenest metszi r2,v2,
// vagyis ha v2 kezdo es vegpontja egyenes ket oldalan fekszik:
static bool ketoldalon(vekt2 v1, vekt2 r2, vekt2 v2) {
    vekt2 norm = rotate_90deg(v1);
    int elso = r2 * norm > 0;
    int masodik = (r2 + v2) * norm > 0;
    return (elso && !masodik) || (!elso && masodik);
}

// Igaz, ha v1 altal meghatarozott egyenest metszi r2,v2,
// vagyis ha v2 kezdo es vegpontja egyenes ket oldalan fekszik:
// Ez mar akkor is metszesnek tekinti, ha kozel vannak egymashoz:
static bool ketoldalon3(vekt2 v1, vekt2 r2, vekt2 v2) {
    static double kishatar = 0.00000001;
    vekt2 norm = rotate_90deg(v1);
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
        return false;
    }
    return true;
}

// Igaz, ha az r1-bol indulo v1 vektor es az r2-bol indulo v2 vektor
// metszi egymast:
bool segments_intersect(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2) {
    return ketoldalon(v1, r2 - r1, v2) && ketoldalon(v2, r1 - r2, v1);
}

// Ez mar akkor is metszesnek tekinti, ha kozel vannak egymashoz:
bool segments_intersect_inexact(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2) {
    return ketoldalon3(v1, r2 - r1, v2) && ketoldalon3(v2, r1 - r2, v1);
}

// Igazzal ter vissza, ha metszi kort egyenes:
// Ket metszespont kozul a v iranyaba haladva az elsot adja vissza:
bool line_circle_intersection(vekt2 line_r, vekt2 line_v, vekt2 circle_r, double radius,
                              vekt2* intersection_point) {
    vekt2 r = circle_r - line_r;
    line_v.normalize();
    vekt2 k = line_v * (line_v * r); // line_r-bol mutat egyenes legkozelebbi pontjaba
    double tav = point_line_distance(circle_r, line_r, line_v);
    double gyokalatt = radius * radius - tav * tav;
    if (gyokalatt < 0.0) {
        return false;
    }
    double t = sqrt(gyokalatt);
    *intersection_point = line_r + k - line_v * t;
    return true;
}
