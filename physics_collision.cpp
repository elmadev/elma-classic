#include "physics_collision.h"
#include "EDITUJ.H"
#include "LEJATSZO.H"
#include "level.h"
#include "main.h"
#include "object.h"
#include "physics_init.h"
#include "segments.h"
#include "vect2.h"

// Check for collision between a wheel/head of a certain radius, and a provided segment.
// If there is a collision, return true and get the point of collision along the segment.
static bool get_anchor_point(vect2 r, double radius, segment* seg, vect2* point) {
    // Get relative position
    vect2 rel = r - seg->r;
    // The closest point of collision between a point and line is always perpendicular to the line
    // Figure out where along the line is the closest point of collision
    // 0 is the start of the segment, and seg->length is the end of the segment
    double position_along_line = rel * seg->unit_vector;
    if (position_along_line < 0) {
        // We are behind the segment
        if ((r - seg->r).length() < radius) {
            // Return the very start of the segment
            *point = seg->r;
            return true;
        } else {
            // Too far, no collision
            return false;
        }
    }
    if (position_along_line > seg->length) {
        // We are in front of the segment
        if ((r - (seg->r + seg->unit_vector * seg->length)).length() < radius) {
            // Return the very end of the segment
            *point = seg->r + seg->unit_vector * seg->length;
            return true;
        } else {
            // Too far, no collision
            return false;
        }
    }
    // We are neither behind nor in front of the segment, so we will collide somewhere in the middle
    // First, make sure we aren't too far away from the line to touch
    vect2 n = rotate_90deg(seg->unit_vector);
    double distance = rel * n;
    if (distance < -radius || distance > radius) {
        return false;
    }
    // Calculate where along the segment we will touch
    *point = seg->r + seg->unit_vector * position_along_line;
    return true;
}

int get_two_anchor_points(vect2 r, double radius, vect2* point1, vect2* point2) {
    // Iterate through all the lines in one collision cell
    Segments->iterate_collision_grid_cell_segments(r);
    int anchor_point_count = 0;
    segment* seg = nullptr;
    while ((seg = Segments->next_collision_grid_segment())) {
        // Find the point of collision between the wheel/head and the line
        vect2 point;
        if (get_anchor_point(r, radius, seg, &point)) {
            if (anchor_point_count == 2) {
                internal_error("anchor_point_count == 2");
            }
            // Verify our second collision
            if (anchor_point_count == 1) {
                *point2 = point;
                anchor_point_count++;
                // If the first and second collision are too close together,
                // average out the two points and treat it as a single point, then keep searching.
                // This should trigger when you touch a vertex, as two segments touch at a vertex.
                // This is also affects vsync bugs when many vertices are super close together.
                if ((*point1 - *point2).length() < TwoPointDiscriminationDistance) {
                    *point1 = (*point1 + *point2) * 0.5;
                    anchor_point_count = 1;
                } else {
                    // We found a valid second point, we're done!
                    return anchor_point_count;
                }
            }
            // Always accept our first collision
            if (anchor_point_count == 0) {
                *point1 = point;
                anchor_point_count++;
            }
        }
    }
    return anchor_point_count;
}

int get_touching_object(vect2 r, double radius) {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* obj = Ptop->objects[i];
        if (!obj) {
            break;
        }

        // Skip Start object and eaten Food
        if (!obj->active) {
            continue;
        }

        // Skip Exit in flagtag mode
        if (obj->type == object::Type::Exit && !Single && Tag) {
            continue;
        }

        vect2 diff = r - obj->r;
        double max_distance = radius + ObjectRadius;
        if (diff.x * diff.x + diff.y * diff.y < max_distance * max_distance) {
            return i;
        }
    }

    return -1;
}
