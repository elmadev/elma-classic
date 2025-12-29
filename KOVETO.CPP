#include "ALL.H"

koveto::koveto(void) {
    // Nullazas:
    length = 0;
    for (int i = 0; i < MAX_GRASS_PICS; i++) {
        pics[i] = NULL;
        is_up[i] = 0;
    }
}

// nev kiterjesztes nelkuli:
void koveto::add(pic8* pic, int up) {
    if (length >= MAX_GRASS_PICS) {
        uzenet("Too many grass pictures in lgr file!");
    }
    pics[length] = pic;
    is_up[length] = up;

    length++;
}

koveto::~koveto(void) {
    for (int i = 0; i < MAX_GRASS_PICS; i++) {
        if (pics[i]) {
            delete pics[i];
            pics[i] = NULL;
        }
    }
}

// cur-t adja vissza (ami mindig utolso jol kitoltott hely):
// Elso alkalommal kitolti *px0-at:
static int grass_line_heightmap(gyuru* poly, int v1, int v2, int* x0, int cur, int* heightmap,
                                int max_heightmap_length, vect2* origin) {
    if (v1 < 0 || v1 >= poly->pontszam || v2 < 0 || v2 >= poly->pontszam) {
        hiba("8u9tg4");
    }

    vect2 r1 = poly->ponttomb[v1];
    vect2 r2 = poly->ponttomb[v2];
    if (r1.x > r2.x) {
        return cur;
    }

    int x1 = (r1.x - origin->x) * MetersToPixels;
    double y1 = (-r1.y - origin->y) * MetersToPixels; // Mivel forditva vannak:
    int x2 = (r2.x - origin->x) * MetersToPixels;
    double y2 = (-r2.y - origin->y) * MetersToPixels;

    if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0) {
        hiba("7yces");
    }

    if (cur < 0) {
        // Elso jo hivas:
        cur = x1;
        if (*x0 >= 0) {
            hiba("kjiut67");
        }
        *x0 = x1;
        heightmap[0] = y1;
    }

    if (x1 >= x2) {
        return cur;
    }
    if (x1 - *x0 >= max_heightmap_length) {
        return cur;
    }

    if (cur < x1 - 1) {
#ifdef DEBUG
        hiba("o96ytuyg");
#endif
        return cur;
    }

    for (int x = x1; x <= x2; x++) {
        if (x < cur) {
            continue;
        }
        if (x - *x0 >= max_heightmap_length) {
            return cur;
        }
        double y = y1 + (y2 - y1) * ((double)x - x1) / (x2 - x1);
        heightmap[x - *x0] = y;
        cur = x;
    }
    return cur;
}

// Ez nem hasznal semmit koveto adatokbol:
int create_grass_polygon_heightmap(gyuru* poly, int* heightmap, int* heightmap_length, int* x0,
                                   int max_heightmap_length, vect2* origin) {
    *heightmap_length = 0;
    // Most megkeressuk osszekoto szakaszt:
    double max_vertex_length = 0.0;
    int v1 = 0;
    for (int i = 0; i < poly->pontszam; i++) {
        int j = i + 1;
        if (j == poly->pontszam) {
            j = 0;
        }
        double length = fabs(poly->ponttomb[i].x - poly->ponttomb[j].x);
        if (length > max_vertex_length) {
            v1 = i;
            max_vertex_length = length;
        }
    }
    if (max_vertex_length < 0.0001) {
        return 0;
    }

    int polygon_is_counterclockwise = 1;
    int v2 = v1 + 1;
    if (v2 == poly->pontszam) {
        v2 = 0;
    }
    // Forditva kell nezni, mivel osszekoto szakasz ellentetes:
    if (poly->ponttomb[v1].x < poly->ponttomb[v2].x) {
        polygon_is_counterclockwise = 0;
    }

    // Vegigmegyunk szakaszokon:
    *x0 = -1;
    int cur = -1;
    for (int i = 0; i < poly->pontszam - 1; i++) {
        // Eloszor lepunk mert osszekoto szakaszrol indultunk:
        if (polygon_is_counterclockwise) {
            v1++;
            if (v1 == poly->pontszam) {
                v1 = 0;
            }
            v2++;
            if (v2 == poly->pontszam) {
                v2 = 0;
            }
        } else {
            v1--;
            if (v1 < 0) {
                v1 = poly->pontszam - 1;
            }
            v2--;
            if (v2 < 0) {
                v2 = poly->pontszam - 1;
            }
        }
        // Most pedig beetetjuk vonalba szakaszt:
        int left_v = v1;
        int right_v = v2;
        if (!polygon_is_counterclockwise) {
            left_v = v2;
            right_v = v1;
        }

        // Elso alkalommal kitolti *x0-at:
        cur = grass_line_heightmap(poly, left_v, right_v, x0, cur, heightmap, max_heightmap_length,
                                   origin);
    }

    if (*x0 < 0) {
        return 0;
    }

    *heightmap_length = cur - *x0 + 1;
    return 1;
}
