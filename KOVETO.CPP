#include "ALL.H"

koveto::koveto(void) {
    length = 0;
    for (int i = 0; i < MAX_GRASS_PICS; i++) {
        pics[i] = NULL;
        is_up[i] = 0;
    }
}

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

// Calculate the heightmap for the line segment of `poly`,
// between vertices `v1` and `v2`.
//
// Populates `x0` with the first x value, if `x0` is < 0.
//
// Returns the x value of the last pixel the height was
// calculated for.
static int grass_line_heightmap(gyuru* poly, int v1, int v2, int* x0, int cur, int* heightmap,
                                int max_heightmap_length, vect2* origin) {
    if (v1 < 0 || v1 >= poly->pontszam || v2 < 0 || v2 >= poly->pontszam) {
        hiba("8u9tg4");
    }

    vect2 r1 = poly->ponttomb[v1];
    vect2 r2 = poly->ponttomb[v2];
    // If the line goes towards the left, don't draw anything.
    if (r1.x > r2.x) {
        return cur;
    }

    // Convert coordinates into pixel positions
    int x1 = (r1.x - origin->x) * MetersToPixels;
    double y1 = (-r1.y - origin->y) * MetersToPixels;
    int x2 = (r2.x - origin->x) * MetersToPixels;
    double y2 = (-r2.y - origin->y) * MetersToPixels;

    if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0) {
        hiba("7yces");
    }

    if (cur < 0) {
        // First line segment, initialise `x0`.
        cur = x1;
        if (*x0 >= 0) {
            hiba("kjiut67");
        }
        *x0 = x1;
        heightmap[0] = y1;
    }

    // Skip lines of length 0.
    if (x1 >= x2) {
        return cur;
    }

    // No more room in the heightmap.
    if (x1 - *x0 >= max_heightmap_length) {
        return cur;
    }

    // Skip if we've somehow jumped forward to the right.
    if (cur < x1 - 1) {
#ifdef DEBUG
        hiba("o96ytuyg");
#endif
        return cur;
    }

    // Calculate the slope of the line between the two
    // vertices, the y value at each step is recorded in the
    // heightmap.
    for (int x = x1; x <= x2; x++) {
        // We've doubled back to the left on a previous line, don't overwrite
        // existing line data.
        if (x < cur) {
            continue;
        }

        // We've reached the end of the heightmap.
        if (x - *x0 >= max_heightmap_length) {
            return cur;
        }

        // This is linear interpolation:
        // y = y1 + t(y2 - y1)
        // With:
        // t = (x - x1) / (x2 - x1)
        double y = y1 + (y2 - y1) * ((double)x - x1) / (x2 - x1);
        heightmap[x - *x0] = y;
        cur = x;
    }
    return cur;
}

// Create a heightmap for `poly`.
int create_grass_polygon_heightmap(gyuru* poly, int* heightmap, int* heightmap_length, int* x0,
                                   int max_heightmap_length, vect2* origin) {
    *heightmap_length = 0;
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
    if (poly->ponttomb[v1].x < poly->ponttomb[v2].x) {
        polygon_is_counterclockwise = 0;
    }

    *x0 = -1;
    int cur = -1;
    // Starting from the longest line, evaluate every line a
    // counterclockwise direction (left to right).
    for (int i = 0; i < poly->pontszam - 1; i++) {
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

        int left_v = v1;
        int right_v = v2;
        if (!polygon_is_counterclockwise) {
            left_v = v2;
            right_v = v1;
        }

        cur = grass_line_heightmap(poly, left_v, right_v, x0, cur, heightmap, max_heightmap_length,
                                   origin);
    }

    if (*x0 < 0) {
        return 0;
    }

    *heightmap_length = cur - *x0 + 1;
    return 1;
}
