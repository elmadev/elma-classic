#include "UTKOZES.H"
#include "main.h"
#include "physics_init.h"
#include "segments.h"
#include "vect2.h"

static int get_anchor_point(vect2 r, double radius, segment* seg, vect2* point) {
    vect2 rel = r - seg->r;
    double position_along_line = rel * seg->unit_vector;
    if (position_along_line < 0) {
        if ((r - seg->r).length() < radius) {
            *point = seg->r;
            return 1;
        } else {
            return 0;
        }
    }
    if (position_along_line > seg->length) {
        if ((r - (seg->r + seg->unit_vector * seg->length)).length() < radius) {
            *point = seg->r + seg->unit_vector * seg->length;
            return 1;
        } else {
            return 0;
        }
    }
    vect2 n = rotate_90deg(seg->unit_vector);
    double distance = rel * n;
    if (distance < -radius || distance > radius) {
        return 0;
    }
    *point = seg->r + seg->unit_vector * position_along_line;
    return 1;
}

int get_two_anchor_points(vect2 r, double radius, vect2* point1, vect2* point2, segment** seg1,
                          segment** seg2) {
    *seg1 = *seg2 = nullptr;

    Segments->iterate_collision_grid_cell_segments(r);
    int anchor_point_count = 0;
    segment* seg = nullptr;
    // int vegignezett = 0; // tesztelesre!
    while ((seg = Segments->next_collision_grid_segment()) != nullptr) {
        // vegignezett++;
        vect2 point;
        if (get_anchor_point(r, radius, seg, &point)) {
            if (anchor_point_count == 2) {
                internal_error("anchor_point_count == 2");
            }
            if (anchor_point_count == 1) {
                *point2 = point;
                *seg2 = seg;
                anchor_point_count++;
                // Ellenorzes, hogy nincs-e tul kozel ketpont,
                // ha ket pontrol van szo:
                if ((*point1 - *point2).length() < TwoPointDiscriminationDistance) {
                    *point1 = (*point1 + *point2) * 0.5; // atlagol
                    anchor_point_count = 1;
                    *seg2 = nullptr;
                } else {
                    // Kettonel tobb talppont mar ugy sem lehet!
                    return anchor_point_count;
                }
            }
            if (anchor_point_count == 0) {
                *point1 = point;
                *seg1 = seg;
                anchor_point_count++;
            }
        }
    }
    // kinyom( vegignezett );
    return anchor_point_count;
}
