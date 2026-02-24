#include "grass.h"
#include "main.h"
#include "polygon.h"
#include "physics_init.h"
#include "pic8.h"
#include <algorithm>
#include <cmath>
#include <format>

grass::~grass() {
    for (updown& qupdown : elements) {
        delete[] qupdown.msk.data;
    }
}

static void generate_grass_mask(mask& msk, pic8* pic, int target_height) {
    // Source pic dimensions
    int src_width = pic->get_width();
    int src_height = pic->get_height();

    msk.name[0] = 0;
    msk.width = 0;
    msk.height = 0;
    msk.data = nullptr;

    // No mask data if qupdown image is too large
    int mask_height = std::max((int)(target_height), 1);
    double scale = (double)(mask_height) / (double)(src_height);
    int mask_width = std::max((int)(src_width * scale), 1);
    if (mask_height > 640.0 || mask_width > 480.0) {
        return;
    }

    // Generate heightmap
    unsigned char transparency = pic->gpixel(0, 0);
    int* heightmap = new int[mask_width];
    for (int j = 0; j < mask_width; j++) {
        heightmap[j] = mask_height; // for transparent columns
        for (int i = 0; i < src_height; i++) {
            if (pic->gpixel(j, i) != transparency) {
                heightmap[j] = (int)(i * scale);
                break;
            }
        }
    }

    // Create mask
    msk.width = mask_width;
    msk.height = *std::max_element(heightmap, heightmap + mask_width);
    create_grass_mask(msk, heightmap);
}

void grass::add(pic8* pic, bool up, int target_height) {
    if (elements.size() >= MAX_GRASS_PICS) {
        external_error("Too many grass pictures in lgr file!");
    }

    constexpr int SLOPE_PADDING = 2 * GRASS_MARGIN + 1;
    int slope = target_height - SLOPE_PADDING;
    if (slope < 0) {
        external_error(
            std::format("QUP/QDOWN picture's height is less than {}!", SLOPE_PADDING).c_str());
    }
    if (!up) {
        slope *= -1;
    }

    mask msk;
    generate_grass_mask(msk, pic, target_height);

    elements.emplace_back(std::unique_ptr<pic8>(pic), up, slope, msk);
}

// Calculate the heightmap for the line segment of `poly`,
// between vertices `v1` and `v2`.
//
// Populates `x0` with the first x value, if `x0` is < 0.
//
// Returns the x value of the last pixel the height was
// calculated for.
static int grass_line_heightmap(polygon* poly, int v1, int v2, int* x0, int cur, int* heightmap,
                                int max_heightmap_length, vect2* origin) {
    if (v1 < 0 || v1 >= poly->vertex_count || v2 < 0 || v2 >= poly->vertex_count) {
        internal_error("grass_line_heightmap vertex out of bounds!");
    }

    vect2 r1 = poly->vertices[v1];
    vect2 r2 = poly->vertices[v2];
    // If the line goes towards the left, don't draw anything.
    if (r1.x > r2.x) {
        return cur;
    }

    // Convert coordinates into pixel positions
    int x1 = (int)((r1.x - origin->x) * MetersToPixels);
    double y1 = (-r1.y - origin->y) * MetersToPixels;
    int x2 = (int)((r2.x - origin->x) * MetersToPixels);
    double y2 = (-r2.y - origin->y) * MetersToPixels;

    if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0) {
        internal_error("grass_line_heightmap coordinate out of bounds!");
    }

    if (cur < 0) {
        // First line segment, initialise `x0`.
        cur = x1;
        if (*x0 >= 0) {
            internal_error("grass_line_heightmap x0 already initialised!");
        }
        *x0 = x1;
        heightmap[0] = (int)(y1);
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
        internal_error("grass_line_heightmap skipped forwards!");
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
        heightmap[x - *x0] = (int)(y);
        cur = x;
    }
    return cur;
}

// Create a heightmap for `poly`.
bool create_grass_polygon_heightmap(polygon* poly, int* heightmap, int* heightmap_length, int* x0,
                                    int max_heightmap_length, vect2* origin) {
    *heightmap_length = 0;
    double max_vertex_length = 0.0;
    int v1 = 0;
    for (int i = 0; i < poly->vertex_count; i++) {
        int j = i + 1;
        if (j == poly->vertex_count) {
            j = 0;
        }
        double length = fabs(poly->vertices[i].x - poly->vertices[j].x);
        if (length > max_vertex_length) {
            v1 = i;
            max_vertex_length = length;
        }
    }
    if (max_vertex_length < 0.0001) {
        return false;
    }

    bool polygon_is_counterclockwise = true;
    int v2 = v1 + 1;
    if (v2 == poly->vertex_count) {
        v2 = 0;
    }
    if (poly->vertices[v1].x < poly->vertices[v2].x) {
        polygon_is_counterclockwise = false;
    }

    *x0 = -1;
    int cur = -1;
    // Starting from the longest line, evaluate every line a
    // counterclockwise direction (left to right).
    for (int i = 0; i < poly->vertex_count - 1; i++) {
        if (polygon_is_counterclockwise) {
            v1++;
            if (v1 == poly->vertex_count) {
                v1 = 0;
            }
            v2++;
            if (v2 == poly->vertex_count) {
                v2 = 0;
            }
        } else {
            v1--;
            if (v1 < 0) {
                v1 = poly->vertex_count - 1;
            }
            v2--;
            if (v2 < 0) {
                v2 = poly->vertex_count - 1;
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
        return false;
    }

    *heightmap_length = cur - *x0 + 1;
    return true;
}
