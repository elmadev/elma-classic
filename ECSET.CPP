#include "ECSET.H"
#include "anim.h"
#include "EDITUJ.H"
#include "eol_settings.h"
#include "grass.h"
#include "polygon.h"
#include "level.h"
#include "lgr.h"
#include "main.h"
#include "M_PIC.H"
#include "menu_pic.h"
#include "object.h"
#include "physics_init.h"
#include "pic8.h"
#include "segments.h"
#include "sprite.h"
#include <cstdint>
#include <cmath>
#include <cstring>

typedef uintptr_t pixelek_t;

canvas* CanvasBack = NULL;
canvas* CanvasFront = NULL;
canvas* CanvasMinimap = NULL;

#define PIXELMASZK (~((uintptr_t)0x7FF))

static void memory_error(void) {
    external_error("You do not have enough memory to load this level!",
                   "Try to set the graphic detail to low at the options.");
}

canvas_chunk_node* canvas::new_node(void) {
    // Check to see if we used up all the unused nodes in our memory struct
    if (node_array_index >= CHUNK_NODE_BLOCK_LENGTH) {
        // Create a new memory struct and reset the index to 0
        node_array_index = 0;
        node_array_last->next = new canvas_chunk_node_array;
        if (!node_array_last->next) {
            memory_error();
        }

        // Point to the new memory struct
        node_array_last = node_array_last->next;
        node_array_last->next = NULL;

        // Return the next unused node
        node_array_index++;
        canvas_chunk_node* new_node = &node_array_last->nodes[node_array_index - 1];
        new_node->distance = 0;
        return new_node;
    }

    // Return the next unused node
    node_array_index++;
    canvas_chunk_node* new_node = &node_array_last->nodes[node_array_index - 1];
    new_node->distance = 0;
    return new_node;
}

// Round a value in meters so it points to exactly the middle of a pixel
static void quantize(double* meters) {
    int pixels = (int)(*meters * MetersToPixels);
    *meters = (pixels + 0.5) / MetersToPixels;
}

void canvas::set_origin_and_dimensions(void) {
    // Iterate through all the segments and grab the min/max x/y values
    Segments->iterate_all_segments();
    segment* seg = Segments->next_segment();
    double minx = seg->r.x;
    double maxx = seg->r.x;
    double miny = seg->r.y;
    double maxy = seg->r.y;
    while (seg) {
        if (seg->r.x < minx) {
            minx = seg->r.x;
        }
        if (seg->r.x > maxx) {
            maxx = seg->r.x;
        }
        if (seg->r.y < miny) {
            miny = seg->r.y;
        }
        if (seg->r.y > maxy) {
            maxy = seg->r.y;
        }

        // This code is unnecessary:
        // We only need to check the end of the line for Verzio Across levels
        // since all polygons are closed loops in Elma.
        if (seg->r.x + seg->v.x < minx) {
            minx = seg->r.x + seg->v.x;
        }
        if (seg->r.x + seg->v.x > maxx) {
            maxx = seg->r.x + seg->v.x;
        }
        if (seg->r.y + seg->v.y < miny) {
            miny = seg->r.y + seg->v.y;
        }
        if (seg->r.y + seg->v.y > maxy) {
            maxy = seg->r.y + seg->v.y;
        }

        seg = Segments->next_segment();
    }

    if (is_minimap) {
        // Minimap, we don't need to quantize
        // Add a small buffer around the level to avoid crashing from out of bounds
        origin = vect2(minx - 10000 / MetersToPixels, miny - 4000 / MetersToPixels);
        width = (maxx + 10000 / MetersToPixels) - origin.x;
        height = (maxy + 4000 / MetersToPixels) - origin.y;
    } else {
        // Main view, we need to quantize
        // Add a small buffer around the level to avoid crashing from out of bounds
        origin = vect2(minx - 10000 / MetersToPixels, miny - 1000 / MetersToPixels);
        quantize(&origin.x);
        quantize(&origin.y);
        width = (maxx + 10000 / MetersToPixels) - origin.x;
        height = (maxy + 1000 / MetersToPixels) - origin.y;
    }
}

void canvas::draw_segment(segment* seg) {
    // Get the line position relative to the origin, converted to pixels
    vect2 r, v;
    r = seg->r - origin;
    r.x *= MetersToPixels;
    r.y *= MetersToPixels;
    v.x = seg->v.x * MetersToPixels;
    v.y = seg->v.y * MetersToPixels;
    if (is_minimap) {
        // Minimap adjustment
        double reciprocal = 1.0 / MinimapScaleFactor;
        r.x *= reciprocal;
        r.y *= reciprocal;
        v.x *= reciprocal;
        v.y *= reciprocal;
    }

    // Quantize to top-left corner of pixel
    r.x -= 0.5;
    r.y -= 0.5;

    // Determine whether the line will generate background or foreground
    int line_direction_negative = v.y < 0;

    // If the vector is going in a negative y direction, swap it to go in the positive y direction
    if (v.y < 0) {
        r = r + v;
        v = Vect2null - v;
    }

    // Don't draw perfectly horizontal lines
    if (v.y < 0.001) {
        return;
    }

    // Draw over the target rows
    int ymin = (int)(r.y + 1.0); // std::ceil(r.y)
    int ymax = (int)(r.y + v.y); // std::floor((r + v).y)

    // Calculate the slope of the line x = ym + b
    double m = v.x / v.y;
    // Calculate the x-intercept of the line b
    double y1 = r.y;
    double y2 = r.y + v.y;
    double x1 = r.x;
    double x2 = r.x + v.x;
    double b = (x2 * y1 - x1 * y2) / (y1 - y2);
    // Add a chunk at the appropriate x position for each y position
    for (int y = ymin; y <= ymax; y++) {
        int x = (int)(m * y + b + 1.0); // x = std::ceil(ym + b)

        // Make sure we aren't out of bounds.
        // (shouldn't happen due to margin in set_origin_and_dimensions())
        if (y < 10 || y >= pixel_height) {
            internal_error("canvas::draw_segment y < 10 || y >= pixel_height!");
        }
        if (x < 10 || x > pixel_width) {
            internal_error("canvas::draw_segment x < 10 || x > pixel_width!");
        }

        // Create a new node
        canvas_chunk_node* node = new_node();
        node->next = NULL;
        node->width = x;
        if (line_direction_negative) {
            node->pixels = (unsigned char*)PixelsDefaultForeground;
        } else {
            node->pixels = (unsigned char*)PixelsDefaultBackground;
        }

        // Append it to the end of the linked list (linked list is currently not in order)
        canvas_chunk_node* row_nodes = rows_linked[y];
        if (!row_nodes) {
            internal_error("canvas::draw_segment !row_nodes!");
        }
        while (row_nodes->next) {
            row_nodes = row_nodes->next;
        }
        row_nodes->next = node;
    }
}

// Initially, the chunk keeps track of x position instead of chunk width
// Convert x position values to chunk width values
static int xpos_to_width(canvas_chunk_node* row_nodes) {
    int xpos = row_nodes->width;
    canvas_chunk_node* cur_node = row_nodes;
    while (cur_node) {
        if (cur_node->next) {
            cur_node->width = cur_node->next->width - cur_node->width;
        } else {
            // Very last chunk has a infinite-ish width
            cur_node->width = 1000000;
        }

        cur_node = cur_node->next;
    }
    return xpos;
}

// Chunks are in an unsorted order when using draw_segment
// 1) Stable-sort the chunks in order by x position
// 2) Also merge multiple chunks at the same x position so we don't have 0-width chunks
//    - foreground/background destroy each other
//    - fg + fg or bg + bg destroy only 1
// 3) Merge two adjacent identical chunks (e.g. gr -> gr or bg -> bg)
static void sort_xpos_and_merge(canvas_chunk_node* row_nodes) {
    // Count the number of chunks in one row so we can bubble sort
    int count = 0;
    canvas_chunk_node* cur_node = row_nodes;
    while (cur_node) {
        cur_node = cur_node->next;
        count++;
    }
    if (count <= 0) {
        internal_error("(canvas) sort_xpos_and_merge-ben count <= 0!");
    }
    if (count < 2) {
        return;
    }

    // Bubble sort
    for (int j = 0; j < count + 2; j++) {
        cur_node = row_nodes;
        cur_node = cur_node->next;
        while (cur_node->next) {
            canvas_chunk_node* next_node = cur_node->next;
            if (cur_node->width > next_node->width) {
                int tmp_xpos = cur_node->width;
                cur_node->width = next_node->width;
                next_node->width = tmp_xpos;

                unsigned char* tmp_pixels = cur_node->pixels;
                cur_node->pixels = next_node->pixels;
                next_node->pixels = tmp_pixels;
            }
            cur_node = next_node;
        }
    }

    // Merge chunks at the same position
    canvas_chunk_node* prev_node = row_nodes;
    cur_node = prev_node->next;
    while (cur_node->next) {
        canvas_chunk_node* next_node = cur_node->next;
        // Check if two nodes with the same x position
        if (cur_node->width == next_node->width) {
            if (cur_node->pixels == next_node->pixels) {
                // Identical, delete next_node
                cur_node->next = next_node->next;
            } else {
                // Different, delete cur_node and next_node
                prev_node->next = next_node->next;
            }
            // Restart the row from the very beginning (not optimized)
            prev_node = row_nodes;
            cur_node = prev_node->next;
            if (!cur_node) {
                return;
            }
        } else {
            prev_node = cur_node;
            cur_node = next_node;
        }
    }

    // Merge identical adjacent chunks
    cur_node = row_nodes;
    while (cur_node->next) {
        canvas_chunk_node* next_node = cur_node->next;
        if (cur_node->pixels == next_node->pixels) {
            // Identical, delete next_node
            cur_node->next = next_node->next;
            // Restart the row from the very beginning (not optimized)
            cur_node = row_nodes;
        } else {
            cur_node = next_node;
        }
    }
}

void canvas::default_foreground_to_pointers(void) {
    for (int i = 0; i < pixel_height; i++) {
        canvas_chunk_node* cur_node = rows_linked[i];
        // Make sure canvas is unused so far
        int xpos = rows_x1[i];
        if (xpos > 100) {
            internal_error("canvas::default_foreground_to_pointers xpos > 100!");
        }
        while (cur_node) {
            // Make sure width doesn't exceed the width in generate_default_texture()
            if ((pixelek_t)cur_node->pixels == PixelsDefaultForeground &&
                cur_node->width < SCREEN_WIDTH) {
                int x = xpos % Lgr->foreground_original_width;
                int y = i % Lgr->foreground->get_height();
                cur_node->pixels = Lgr->foreground->get_row(y) + x;
            }
            xpos += cur_node->width;
            cur_node = cur_node->next;
        }
    }
}

void canvas::merge_redundant_chunks(void) {
    for (int i = 0; i < pixel_height; i++) {
        int again = 1;
        while (again) {
            again = 0;
            canvas_chunk_node* cur_node = rows_linked[i];
            while (cur_node && cur_node->next) {
                canvas_chunk_node* next_node = cur_node->next;
                // Merge enums and textures only
                if (((pixelek_t)cur_node->pixels & PIXELMASZK) == 0 &&
                    cur_node->pixels == next_node->pixels) {
                    cur_node->next = next_node->next;
                    cur_node->width += next_node->width;
                    again = 1;
                }
                cur_node = cur_node->next;
            }
        }
    }
}

void canvas::textures_to_pointers(void) {
    for (int y = 0; y < pixel_height; y++) {
        canvas_chunk_node* cur_node = rows_linked[y];
        // Make sure canvas is unused so far
        int xpos = rows_x1[y];
        if (xpos > 100) {
            internal_error("canvas::textures_to_pointers xpos > 100");
        }
        while (cur_node) {
            int original_width = cur_node->width;
            // Convert textures to raw pixel data
            pixelek_t texture_index = (pixelek_t)cur_node->pixels;
            if (((texture_index & PIXELMASZK) == 0) && (texture_index >= 10)) {
                texture_index -= 10;
#ifdef DEBUG
                if (texture_index >= Lgr->texture_count) {
                    internal_error(
                        "canvas::textures_to_pointers texture_index >= Lgr->picture_count!");
                }
                if (Lgr->textures[texture_index].original_width <= 0) {
                    internal_error("canvas::textures_to_pointers texture invalid width!");
                }
#endif
                // Split each chunk into multiple chunks based on the texture's width
                texture* text = &Lgr->textures[texture_index];
                pic8* pic = text->pic;
                int remaining_width = cur_node->width;
                int texture_x_offset = xpos;
                int first = 1;
                while (remaining_width > 0) {
                    if (!first) {
                        canvas_chunk_node* node = new_node();
                        node->next = cur_node->next;
                        cur_node->next = node;
                        cur_node = node;
                    }
                    first = 0;

                    texture_x_offset %= text->original_width;
                    int node_width = remaining_width;
                    if (texture_x_offset + node_width > pic->get_width()) {
                        node_width = pic->get_width() - texture_x_offset;
                    }

                    cur_node->width = node_width;
                    cur_node->pixels = pic->get_row(y % pic->get_height()) + texture_x_offset;
                    if (((pixelek_t)cur_node->pixels & PIXELMASZK) == 0) {
                        external_error("Memory address is less than 2048 (723)!");
                    }

                    texture_x_offset += node_width;
                    remaining_width -= node_width;
                }
            }
            xpos += original_width;
            cur_node = cur_node->next;
        }
    }
}

static int list_length(canvas_chunk_node* row_nodes) {
    int count = 0;
    canvas_chunk_node* cur_node = row_nodes;
    while (cur_node) {
        cur_node = cur_node->next;
        count++;
    }
    if (count <= 0) {
        internal_error("(canvas) list_length count <= 0!");
    }
    return count;
}

void canvas::linked_list_to_array(void) {
    // Total number of still-used chunks
    int count = 0;
    for (int i = 0; i < pixel_height; i++) {
        count += list_length(rows_linked[i]);
    }

    // Create more efficient canvas_chunk array
    chunk_array = new canvas_chunk[count + 10];
    if (!chunk_array) {
        memory_error();
    }

    // Copy data
    int offset = 0;
    for (int i = 0; i < pixel_height; i++) {
        canvas_chunk_node* cur_node = rows_linked[i];
        rows[i] = &chunk_array[offset];
        rows_position1[i] = rows[i];
        rows_position2[i] = rows[i];
        while (cur_node) {
            chunk_array[offset].width = cur_node->width;
            chunk_array[offset].pixels = cur_node->pixels;
            cur_node = cur_node->next;
            offset++;
        }
    }
}

void canvas::calculate_object_positions(void) {
    const double offset = ANIM_WIDTH / 2.0 * EolSettings->zoom();
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* obj = Ptop->objects[i];
        if (!obj) {
            continue;
        }
        if (is_minimap) {
            obj->minimap_canvas_x =
                (int)((obj->r.x - origin.x) * MetersToPixels / MinimapScaleFactor);
            obj->minimap_canvas_y =
                (int)((-obj->r.y - origin.y) * MetersToPixels / MinimapScaleFactor);
        } else {
            obj->canvas_x = (int)((obj->r.x - origin.x) * MetersToPixels - offset);
            obj->canvas_y = (int)((-obj->r.y - origin.y) * MetersToPixels - offset);
        }
    }
}

enum {
    CanvasClipping_Ground,
    CanvasClipping_Sky,
    CanvasClipping_Unclipped,
    CanvasClipping_Transparent
};

// source must be aligned to a single chunk.
canvas_chunk_node* canvas::draw_one_chunk(canvas_chunk_node* dest, unsigned char* source,
                                          int dest_x, int source_x_left, int source_x_right,
                                          int source_dist, canvas_chunk_node* dest_prev,
                                          int* prev_can_merge, int clipping) {

    if ((pixelek_t)source == PixelsTransparent) {
        internal_error("canvas::draw_one_chunk source is transparent!");
    }
    // Transparent pixels are only specifically used for the front canvas, so enforce this usage
    if ((pixelek_t)dest->pixels == PixelsTransparent && clipping != CanvasClipping_Transparent) {
        internal_error("canvas::draw_one_chunk",
                       "dest is transparent but clipping is not transparent!");
    }

    // Validate whether we want to draw based on clipping and distance
    // Default foreground/background and empty has the lowest-priority distance
    int dest_dist = dest->distance;
    int skip = 0;
    if (clipping == CanvasClipping_Ground) {
        if ((pixelek_t)dest->pixels == PixelsDefaultForeground) {
            dest_dist = 1000000;
        }
        if ((pixelek_t)dest->pixels == PixelsDefaultBackground) {
            skip = 1;
        }
    }
    if (clipping == CanvasClipping_Sky) {
        // Background distance needs to be temporarily modified as Sky clipping should never draw on
        // top of Ground clipping
        source_dist += 10000;
        if ((pixelek_t)dest->pixels == PixelsDefaultForeground) {
            skip = 1;
        }
        if ((pixelek_t)dest->pixels == PixelsDefaultBackground) {
            dest_dist = 1000000;
        }
    }
    if (clipping == CanvasClipping_Unclipped) {
        if ((pixelek_t)dest->pixels == PixelsDefaultForeground ||
            (pixelek_t)dest->pixels == PixelsDefaultBackground) {
            dest_dist = 1000000;
        }
    }

    if (clipping == CanvasClipping_Transparent) {
        // Transparent pixels are only specifically used for the front canvas, so enforce this usage
        if ((pixelek_t)dest->pixels == PixelsDefaultForeground ||
            (pixelek_t)dest->pixels == PixelsDefaultBackground) {
            internal_error("canvas::draw_one_chunk",
                           "dest is non-transparent but clipping is transparent!");
        }
        if ((pixelek_t)dest->pixels == PixelsTransparent) {
            dest_dist = 1000000;
        }
    }

    // Check to see if we actually want to draw it
    if (skip || source_dist >= dest_dist) {
        *prev_can_merge = 0;
        return dest;
    }

    // Sanity checks, which should already be handled by draw_pixels()
    int dest_x_right = dest_x + dest->width - 1;
    if (source_x_left < dest_x) {
        source_x_left = dest_x;
    }
    if (source_x_right > dest_x_right) {
        source_x_right = dest_x_right;
    }
    if (source_x_right < dest_x || source_x_left > dest_x_right) {
        internal_error(
            "canvas::draw_one_chunk source_x_right < dest_x || source_x_left > dest_x_right!");
    }

    // Case 1: Right-aligned
    //  ------------------------------------
    //  I              XXXXXXXXXXXXXXXXXXXXI
    //  ------------------------------------
    if (dest_x != source_x_left && dest_x_right == source_x_right) {
        // Split the chunk in two
        canvas_chunk_node* node = new_node();
        node->width = source_x_right - source_x_left + 1;
        node->pixels = source;
        node->distance = source_dist;
        node->next = dest->next;

        dest->width -= node->width;
        dest->next = node;

        *prev_can_merge = 1;
        return node;
    }

    // Case 2: Left-aligned
    //  ------------------------------------
    //  IXXXXXXXXXXXXXXXXXXXX              I
    //  ------------------------------------
    if (dest_x == source_x_left && dest_x_right != source_x_right) {
        if (!dest_prev) {
            internal_error("canvas::draw_one_chunk missing dest_prev!");
        }
        // Check if prev chunk is a matching texture, in which case we merge with prev chunk
        if (!(*prev_can_merge) && dest_prev->distance == source_dist &&
            !((pixelek_t)dest_prev->pixels & PIXELMASZK) && dest_prev->pixels == source) {
            *prev_can_merge = 1;
        }
        if (*prev_can_merge) {
            // Merge with previous chunk (which was right-aligned or matching texture)
            int insertion_width = source_x_right - source_x_left + 1;
            dest_prev->width += insertion_width;
            dest->width -= insertion_width;
            if ((pixelek_t)dest->pixels & PIXELMASZK) {
                dest->pixels += insertion_width;
            }

            *prev_can_merge = 0;
            return dest;
        } else {
            // Split the chunk in two
            canvas_chunk_node* node = new_node();
            dest_prev->next = node;

            node->width = source_x_right - source_x_left + 1;
            node->pixels = source;
            node->distance = source_dist;
            node->next = dest;

            dest->width -= node->width;
            if ((pixelek_t)dest->pixels & PIXELMASZK) {
                dest->pixels += node->width;
            }

            *prev_can_merge = 0;
            return dest;
        }
    }

    // Case 3: Left- and right-aligned
    //  ------------------------------------
    //  IXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXI
    //  ------------------------------------
    if (dest_x == source_x_left && dest_x_right == source_x_right) {
        if (!dest_prev) {
            internal_error("canvas::draw_one_chunk missing dest_prev!");
        }
        // Check if prev chunk is a matching texture, in which case we merge with prev chunk
        if (!(*prev_can_merge) && dest_prev->distance == source_dist &&
            !((pixelek_t)dest_prev->pixels & PIXELMASZK) && dest_prev->pixels == source) {
            *prev_can_merge = 1;
        }
        if (*prev_can_merge) {
            // Merge with previous chunk (which was right-aligned or matching texture)
            dest_prev->width += dest->width;
            dest_prev->next = dest->next;

            *prev_can_merge = 1;
            return dest_prev;
        } else {
            // Overwrite chunk with new data
            dest->pixels = source;
            dest->distance = source_dist;

            *prev_can_merge = 1;
            return dest;
        }
    }

    // Case 4: Neither left- nor right-aligned
    //  ------------------------------------
    //  I             XXXXXXXXXXXX         I
    //  ------------------------------------
    if (dest_x != source_x_left && dest_x_right != source_x_right) {
        // Split the chunk in three
        canvas_chunk_node* node_middle = new_node();
        canvas_chunk_node* node_right = new_node();

        node_middle->width = source_x_right - source_x_left + 1;
        node_middle->pixels = source;
        node_middle->distance = source_dist;
        node_middle->next = node_right;

        node_right->width = dest_x_right - source_x_right;
        if ((pixelek_t)dest->pixels & PIXELMASZK) {
            node_right->pixels = dest->pixels + (source_x_right + 1 - dest_x);
        } else {
            node_right->pixels = dest->pixels;
        }
        node_right->distance = dest->distance;
        node_right->next = dest->next;

        dest->width -= node_middle->width + node_right->width;
        dest->next = node_middle;

        *prev_can_merge = 0;
        return node_right;
    }

    internal_error("canvas::draw_one_chunk unknown case!");
    return NULL;
}

void canvas::draw_pixels(unsigned char* source, int source_dist, int x_left, int x_right, int y,
                         int clipping) {
    // Make sure canvas is unused so far
    canvas_chunk_node* cur_node = rows_linked[y];
    int xpos = rows_x1[y];
    if (xpos > 10) {
        internal_error("canvas::draw_pixels xpos > 10!");
    }
    // Find the first chunk onto which we will draw
    canvas_chunk_node* prev_node = NULL;
    while (xpos + cur_node->width - 1 < x_left) {
        xpos += cur_node->width;
        prev_node = cur_node;
        cur_node = cur_node->next;
        if (!cur_node) {
            internal_error("canvas::draw_pixels !cur_node!");
        }
    }
    // Draw one chunk at a time
    int prev_can_merge = 0;
    while (1) {
        if (!cur_node) {
            internal_error("canvas::draw_pixels !cur_node!");
        }
        int xpos_left = xpos;
        int xpos_right = xpos + cur_node->width - 1;
        if (x_right < xpos_left) {
            internal_error("canvas::draw_pixels x_right < xpos_left!");
        }
        int done = 0;
        if (xpos_right + 1 > x_right) {
            done = 1;
        }

        // Offset the source data if it is a pointer to pixel data
        unsigned char* source_next = source;
        if ((pixelek_t)source_next & PIXELMASZK) {
            int left_skip = 0;
            if (x_left > xpos_left) {
                left_skip = x_left - xpos_left;
            }
            source_next = source + (cur_node->width - left_skip);
        }

        // Draw
        int xpos_next = xpos + cur_node->width;
        canvas_chunk_node* next_node = cur_node->next;
        prev_node = draw_one_chunk(cur_node, source, xpos, x_left, x_right, source_dist, prev_node,
                                   &prev_can_merge, clipping);

        if (done) {
            return;
        }
        source = source_next;
        xpos = xpos_next;
        cur_node = next_node;
    }
}

void canvas::draw_texture(sprite* spr, int texture_index, int mask_index, int clipping) {
    // Calculate pixel position
    int x = (int)((spr->r.x - origin.x) * MetersToPixels);
    int y = (int)((-spr->r.y - origin.y) * MetersToPixels);

    mask* msk = &Lgr->masks[mask_index];
    int distance = spr->distance;
    int width = msk->width;
    int height = msk->height;

    // Check out of range textures (x position only)
    if (x < 120 || x + width >= pixel_width - 50) {
        return;
    }

    // Draw the mask
    int offset = 0;
    for (int i = 0; i < height; i++) {
        int j = 0;
        while (msk->data[offset].type != MaskEncoding::EndOfLine) {
            if (msk->data[offset].type == MaskEncoding::Solid) {
                draw_pixels((unsigned char*)(pixelek_t)(texture_index + 10), distance, x + j,
                            x + j + msk->data[offset].length - 1, y - i, clipping);
            }
            j += msk->data[offset].length;
            offset++;
        }
        offset++;
    }
}

void canvas::draw_sprites(int clipping) {
    for (int sprite_index = 0; sprite_index < MAX_SPRITES; sprite_index++) {
        sprite* spr = Ptop->sprites[sprite_index];
        if (!spr) {
            return;
        }
        // Only draw the selected clipping
        if (clipping == CanvasClipping_Ground && spr->clipping != Clipping::Ground) {
            continue;
        }
        if (clipping == CanvasClipping_Sky && spr->clipping != Clipping::Sky) {
            continue;
        }
        if (clipping == CanvasClipping_Unclipped && spr->clipping != Clipping::Unclipped) {
            continue;
        }

        if (spr->picture_name[0] == 0) {
            // Texture/Mask combo
            if (!spr->texture_name[0] || !spr->mask_name[0]) {
                continue;
            }
            int texture_index = Lgr->get_texture_index(spr->texture_name);
            if (texture_index < 0) {
                internal_error("draw_sprites texture_index < 0");
            }
            int mask_index = Lgr->get_mask_index(spr->mask_name);
            if (mask_index < 0) {
                internal_error("draw_sprites mask_index < 0");
            }
            draw_texture(spr, texture_index, mask_index, clipping);
            continue;
        }
        if (spr->texture_name[0] || spr->mask_name[0]) {
            internal_error("draw_sprites too many names!");
        }

        // Picture
        int picture_index = Lgr->get_picture_index(spr->picture_name);
        if (picture_index < 0) {
            internal_error("draw_sprites picture_index < 0");
        }
        picture* pict = &Lgr->pictures[picture_index];

        int x = (int)((spr->r.x - origin.x) * MetersToPixels);
        int y = (int)((-spr->r.y - origin.y) * MetersToPixels);
        int distance = spr->distance;
        int width = pict->width;
        int height = pict->height;

        // Check out of range pictures (x position only)
        if (x < 120 || x + width >= pixel_width - 50) {
            // Should continue instead of return here
            return;
        }

        unsigned char* pixeldata = pict->data;
        if (((pixelek_t)pixeldata & PIXELMASZK) == 0) {
            external_error("Memory address is less than 2048 (987)!");
        }

        // Draw the picture
        int offset = 0;
        for (int i = 0; i < height; i++) {
            int j = 0;
            while (1) {
                int skip = pixeldata[offset] * 256 + pixeldata[offset + 1];
                offset += 2;
                if (skip > 60000) {
                    break;
                }
                j += skip;

                int count = pixeldata[offset] * 256 + pixeldata[offset + 1];
                offset += 2;
                if (y - i >= 0 && y - i < pixel_height) {
                    draw_pixels(&pixeldata[offset], distance, x + j, x + j + count - 1, y - i,
                                clipping);
                }
                j += count;
                offset += count;
            }
        }
    }
}

static int consecutive_transparent_pixels(unsigned char* pic_row, int x, int width,
                                          unsigned char transparency) {
    int count = 0;
    for (int i = x; i < width; i++) {
        if (pic_row[i] == transparency) {
            count++;
        } else {
            return count;
        }
    }
    return count;
}

static int consecutive_solid_pixels(unsigned char* pic_row, int x, int width,
                                    unsigned char transparency) {
    int count = 0;
    for (int i = x; i < width; i++) {
        if (pic_row[i] != transparency) {
            count++;
        } else {
            return count;
        }
    }
    return count;
}

#define MAX_HEIGHTMAP_LENGTH (10000)
static int GrassHeightmap[MAX_HEIGHTMAP_LENGTH + 10];

constexpr int GRASS_MARGIN = 20;

void canvas::draw_qgrass_texture(grass* grass, int qupdown_index, int x, int y, int clipping) {
    if (!BufferMain) {
        internal_error("draw_qgrass_texture !BufferMain");
    }

    // Grab the QGRASS texture
    int distance = 600;
    char texturanev[20];
    strcpy(texturanev, "qgrass");
    int texture_index = Lgr->get_texture_index(texturanev);
    if (texture_index < 0) {
        internal_error("draw_qgrass_texture texture_index < 0");
    }

    // Grab the QUP/QDOWN image
    pic8* pic = grass->pics[qupdown_index];
    int width = pic->get_width();
    int height = pic->get_height();
    unsigned char transparency = pic->gpixel(0, 0);

    // Initialize picture buffer
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            BufferMain->ppixel(j, i, 0);
        }
    }
    // Calculate qupdown mask
    for (int j = 0; j < width; j++) {
        for (int i = 0; i < height; i++) {
            if (pic->gpixel(j, i) == transparency) {
                BufferMain->ppixel(j, i, 1);
            } else {
                break;
            }
        }
    }

    // Draw qupdown mask with qgrass texture
    for (int i = 0; i < height; i++) {
        int j = 0;
        unsigned char* row = BufferMain->get_row(height - 1 - i);
        while (j < width) {
            j += consecutive_transparent_pixels(row, j, width, 0);
            if (j >= width) {
                break;
            }
            int count = consecutive_solid_pixels(row, j, width, 0);
            if (count <= 0) {
                internal_error("draw_qgrass_texture count <= 0");
            }
            draw_pixels((unsigned char*)(pixelek_t)(texture_index + 10), distance, x + j,
                        x + j + count - 1, y + i, clipping);
            j += count;
        }
    }

    // Add 20 pixels of height to grass
    int grass_height_padding = QGRASS_EXTRA_HEIGHT * EolSettings->zoom();
    for (int i = 0; i < grass_height_padding; i++) {
        draw_pixels((unsigned char*)(pixelek_t)(texture_index + 10), distance, x, x + width - 1,
                    y + height + i, clipping);
    }
}

void canvas::draw_qupdown(grass* gr, int qupdown_index, int x, int y, int clipping) {
    int distance = 600;

    // Grab the QUP/QDOWN image
    pic8* pic = gr->pics[qupdown_index];
    int width = pic->get_width();
    int height = pic->get_height();
    unsigned char transparency = pic->gpixel(0, 0);

    if (gr->is_up[qupdown_index]) {
        // Slide image down by 20
        y -= GRASS_MARGIN;
    } else {
        // Slide image down by 20, then down again by the slope
        y -= height - 1 - GRASS_MARGIN;
    }

    // Draw upside down
    for (int i = 0; i < height; i++) {
        int j = 0;
        unsigned char* sor = pic->get_row(height - 1 - i);
        while (j < width) {
            j += consecutive_transparent_pixels(sor, j, width, transparency);
            if (j >= width) {
                break;
            }
            int count = consecutive_solid_pixels(sor, j, width, transparency);
            if (count <= 0) {
                internal_error("draw_qupdown count <= 0");
            }
            draw_pixels(&sor[j], distance, x + j, x + j + count - 1, y + i, clipping);
            j += count;
        }
    }

    // Draw the QGRASS image on top of the QUP/QDOWN
    if (width > 640 || height > 480) {
#ifdef DEBUG
        internal_error("7yfg4");
#endif
        return;
    }
    draw_qgrass_texture(gr, qupdown_index, x, y, clipping);
}

void canvas::draw_grass_polygon(grass* gr, int* heightmap, int heightmap_length, int x0,
                                int clipping) {
#ifdef DEBUG
    if (heightmap_length < 1) {
        internal_error("dd387d3e");
    }
#else
    if (heightmap_length < 1) {
        return;
    }
#endif
    int x = x0;
    int y = heightmap[0];
    while (1) {
        if (x >= x0 + heightmap_length) {
            return;
        }

        // Let's pick the best matching QUP/QDOWN
        int best_score = 10000;
        int best_index = -1;
        int best_slope = 0;
        for (int i = 0; i < gr->length; i++) {
            // Check each image and choose the one with the smallest y-offset from desired
            // height
            int slope = gr->pics[i]->get_height();
            if (slope < 2 * GRASS_MARGIN + 1) {
                char tmp[60];
                sprintf(tmp, "QUP/QDOWN picture's height is less than %d!", 2 * GRASS_MARGIN + 1);
                external_error(tmp);
            }
            slope -= 2 * GRASS_MARGIN + 1;
            if (!gr->is_up[i]) {
                slope = -slope;
            }

            int target_x = x + gr->pics[i]->get_width();
            int target_y;
            if (target_x >= x0 + heightmap_length) {
                target_y = heightmap[heightmap_length - 1];
            } else {
                target_y = heightmap[target_x - x0];
            }

            int score = abs(y + slope - target_y);
            if (score < best_score) {
                best_score = score;
                best_index = i;
                best_slope = slope;
            }
        }

        // Draw the best matching QUP/QDOWN
        if (best_index < 0) {
            internal_error("draw_grass_polygon best_index < 0");
        }

        draw_qupdown(gr, best_index, x, y, clipping);
        x += gr->pics[best_index]->get_width();
        y += best_slope;
    }
}

void canvas::draw_grass_polygons(int clipping) {
    // Only if the lgr has the right assets
    if (!Lgr->has_grass) {
        return;
    }

    for (int i = 0; i < MAX_POLYGONS; i++) {
        // Make sure we are drawing a grass polygon onto the ground
        polygon* poly = Ptop->polygons[i];
        if (!poly) {
            return;
        }
        if (!poly->is_grass) {
            continue;
        }

        Clipping valid_clipping = Clipping::Ground;
        if (clipping == CanvasClipping_Ground && valid_clipping != Clipping::Ground) {
            continue;
        }
        if (clipping == CanvasClipping_Sky && valid_clipping != Clipping::Sky) {
            continue;
        }
        if (clipping == CanvasClipping_Unclipped && valid_clipping != Clipping::Unclipped) {
            continue;
        }

        grass* gr = Lgr->grass_pics;

        // Calculate the grass heightmap
        int heightmap_length = 0, x0 = 0;
        if (!create_grass_polygon_heightmap(poly, GrassHeightmap, &heightmap_length, &x0,
                                            MAX_HEIGHTMAP_LENGTH, &origin)) {
            continue;
        }
        if (heightmap_length > MAX_HEIGHTMAP_LENGTH) {
            internal_error("draw_grass_polygons heightmap_length > MAX_HEIGHTMAP_LENGTH");
        }

        // Draw the grass polygon
        draw_grass_polygon(gr, GrassHeightmap, heightmap_length, x0, clipping);
    }
}

void canvas::draw_killers(void) {
    if (!is_minimap) {
        return;
    }

    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* obj = Ptop->objects[i];
        if (!obj) {
            return;
        }

        if (obj->type == object::Type::Food || obj->type == object::Type::Start ||
            obj->type == object::Type::Exit) {
            continue;
        }

        if (is_minimap) {
            // Draw a 3x3 square
            unsigned char* source = Lgr->minimap_killer_palette_id;
            int distance = 499;
            int x = (int)((obj->r.x - origin.x) * (MetersToPixels / MinimapScaleFactor));
            int y = (int)((-obj->r.y - origin.y) * (MetersToPixels / MinimapScaleFactor));
            draw_pixels(source, distance, x - 1, x + 1, y - 1, CanvasClipping_Unclipped);
            draw_pixels(source, distance, x - 1, x - 1, y, CanvasClipping_Unclipped);
            draw_pixels(source, distance, x + 1, x + 1, y, CanvasClipping_Unclipped);
            draw_pixels(source, distance, x - 1, x + 1, y + 1, CanvasClipping_Unclipped);
        }
    }
}

void canvas::adjust_background_distance(void) {
    for (int i = 0; i < pixel_height; i++) {
        canvas_chunk_node* cur_node = rows_linked[i];
        while (cur_node) {
            // In draw_one_chunk(), we temporarily added distance to properly layer assets
            // Remove that distance
            if (cur_node->distance >= 10000) {
                cur_node->distance -= 10000;
            }
            cur_node = cur_node->next;
        }
    }
};

canvas::canvas(int minimap) {
    is_minimap = minimap;

    for (int i = 0; i < CANVAS_MAX_HEIGHT; i++) {
        rows_linked[i] = NULL;
        rows[i] = NULL;
        rows_x1[i] = 0;
        rows_x2[i] = 0;
        rows_position1[i] = NULL;
        rows_position2[i] = NULL;
    }
    node_array = node_array_last = NULL;
    node_array_index = 0;
    chunk_array = NULL;
    width = height = 0.0;
    pixel_width = pixel_height = 0;

    if (!Ptop || !Segments) {
        internal_error("canvas::canvas !Ptop || !Segments!");
    }

    // Initialize node memory
    node_array = new canvas_chunk_node_array;
    if (!node_array) {
        internal_error("canvas::canvas out of memory!");
    }
    node_array->next = NULL;
    node_array_last = node_array;

    // Set canvas dimensions
    set_origin_and_dimensions();
    if (is_minimap) {
        pixel_width = (int)(width * (MetersToPixels / MinimapScaleFactor));
        pixel_height = (int)(height * (MetersToPixels / MinimapScaleFactor));
    } else {
        pixel_width = (int)(width * MetersToPixels);
        pixel_height = (int)(height * MetersToPixels);
    }

    if (pixel_height - 10 > CANVAS_MAX_HEIGHT) {
        internal_error("canvas::canvas pixel_height-10 > CANVAS_MAX_HEIGHT!");
    }

    // Initialize the canvas with a single chunk for every row
    for (int i = 0; i < pixel_height; i++) {
        rows_linked[i] = new_node();
        rows_linked[i]->next = NULL;
        rows_linked[i]->width = 1;
        rows_linked[i]->distance = 0;
        rows_linked[i]->pixels = (unsigned char*)PixelsDefaultForeground;
    }

    // Draw all the polygon data
    Segments->iterate_all_segments();
    segment* seg = Segments->next_segment();
    while (seg) {
        draw_segment(seg);
        seg = Segments->next_segment();
    }

    // Convert the polygon data into widths instead of x position
    for (int i = 0; i < pixel_height; i++) {
        sort_xpos_and_merge(rows_linked[i]);
        rows_x1[i] = rows_x2[i] = xpos_to_width(rows_linked[i]);
    }

    // Handle high quality graphics
    if (!is_minimap && State->high_quality) {
        draw_sprites(CanvasClipping_Ground);
        draw_grass_polygons(CanvasClipping_Ground);
        draw_sprites(CanvasClipping_Sky);
        draw_grass_polygons(CanvasClipping_Sky);
        adjust_background_distance();
        draw_sprites(CanvasClipping_Unclipped);
        draw_grass_polygons(CanvasClipping_Unclipped);
    }

    // Draw killers into the minimap
    draw_killers();
}

canvas::canvas(canvas* reference) {
    for (int i = 0; i < CANVAS_MAX_HEIGHT; i++) {
        rows_linked[i] = NULL;
        rows[i] = NULL;
        rows_x1[i] = 0;
        rows_x2[i] = 0;
        rows_position1[i] = NULL;
        rows_position2[i] = NULL;
    }
    node_array = node_array_last = NULL;
    node_array_index = 0;
    chunk_array = NULL;
    is_minimap = 0;
    width = height = 0.0;
    pixel_width = pixel_height = 0;

    if (!Ptop || !Segments) {
        internal_error("canvas::canvas !Ptop || !Segments!");
    }

    // Initialize node memory
    node_array = new canvas_chunk_node_array;
    if (!node_array) {
        internal_error("canvas::canvas out of memory!");
    }
    node_array->next = NULL;
    node_array_last = node_array;

    // Copy canvas dimensions from reference canvas
    origin = reference->origin;
    width = reference->width;
    height = reference->height;
    pixel_width = reference->pixel_width;
    pixel_height = reference->pixel_height;

    // Initialize the canvas with a single chunk for every row (already as width)
    for (int i = 0; i < pixel_height; i++) {
        rows_x1[i] = 1;
        rows_x2[i] = 1;

        rows_linked[i] = new_node();
        rows_linked[i]->next = NULL;
        rows_linked[i]->width = 1000000;
        rows_linked[i]->distance = 0;
        rows_linked[i]->pixels = (unsigned char*)PixelsTransparent;
    }
}

canvas::~canvas(void) {
    if (node_array) {
        internal_error("canvas::~canvas node_array");
    }
    if (!chunk_array) {
        internal_error("canvas::~canvas !chunk_array!");
    }
    delete chunk_array;
    chunk_array = NULL;
}

void canvas::delete_all_nodes(void) {
    if (!node_array) {
        internal_error("canvas::delete_all_nodes !node_array!");
    }
    canvas_chunk_node_array* cur_array = node_array;
    while (cur_array) {
        canvas_chunk_node_array* next_array = cur_array->next;
        delete cur_array;
        cur_array = next_array;
    }
    node_array = NULL;
    for (int i = 0; i < CANVAS_MAX_HEIGHT; i++) {
        rows_linked[i] = NULL;
    }
}

static int ViewLeftBorder, ViewRightBorder;

unsigned char *DefaultForeground = NULL, *DefaultBackground = NULL;

static inline void draw_default_foreground(unsigned char* dest, size_t offset, size_t width,
                                           bool minimap) {
    if (!minimap) {
        memcpy(dest, DefaultForeground + offset, width);
    } else {
        memset(dest, Lgr->minimap_foreground_palette_id, width);
    }
}

static inline void draw_default_background(unsigned char* dest, size_t offset, size_t width,
                                           bool minimap) {
    if (!minimap) {
        memcpy(dest, DefaultBackground + offset, width);
    } else {
        memset(dest, Lgr->minimap_background_palette_id, width);
    }
}

void canvas::render_row1(unsigned char* dest, int y) {
    // Find chunk corresponding to the left of the screen
    canvas_chunk* cur_chunk = rows_position1[y];
    int xpos = rows_x1[y];
    // Search leftwards using left border of chunk
    while (xpos > ViewLeftBorder) {
        cur_chunk--;
        xpos -= cur_chunk->width;
    }
    // Search rightwards using right border of chunk
    while (xpos + cur_chunk->width <= ViewLeftBorder) {
        xpos += cur_chunk->width;
        cur_chunk++;
    }
    // Remember our current position
    rows_position1[y] = cur_chunk;
    rows_x1[y] = xpos;

    // If the entire row is a single chunk, paste it
    int offset = ViewLeftBorder - xpos;
    if (xpos + cur_chunk->width > ViewRightBorder) {
        int width = ViewRightBorder - xpos + 1;
        if ((pixelek_t)cur_chunk->pixels & PIXELMASZK) {
            memcpy(dest, &cur_chunk->pixels[ViewLeftBorder - xpos], width - offset);
        } else {
            if (!cur_chunk->pixels) {
                draw_default_background(dest, 0, width - offset, is_minimap);
                return;
            }
            if ((pixelek_t)cur_chunk->pixels == PixelsDefaultForeground) {
                draw_default_foreground(dest, 0, width - offset, is_minimap);
            } else {
#ifdef DEBUG
                if ((pixelek_t)cur_chunk->pixels != PixelsTransparent) {
                    internal_error("cur_chunk->pixels unknown or texture type!");
                }
#endif
            }
        }
        return;
    }

    // Otherwise we need to split the situation into the first chunk (not left-aligned),
    // and the remaining chunks (all left-aligned)
    // First chunk (not left-aligned):
    int size = cur_chunk->width - offset;
    if ((pixelek_t)cur_chunk->pixels & PIXELMASZK) {
        memcpy(dest, &cur_chunk->pixels[ViewLeftBorder - xpos], size);
    } else {
        if (!cur_chunk->pixels) {
            draw_default_background(dest, 0, size, is_minimap);
        } else {
            if ((pixelek_t)cur_chunk->pixels == PixelsDefaultForeground) {
                draw_default_foreground(dest, 0, size, is_minimap);
            } else {
#ifdef DEBUG
                if ((pixelek_t)cur_chunk->pixels != PixelsTransparent) {
                    internal_error("cur_chunk->pixels unknown or texture type!");
                }
#endif
            }
        }
    }
    xpos += cur_chunk->width;
    cur_chunk++;
    dest += size;

    // Remaining chunks (left-aligned)
    while (xpos <= ViewRightBorder) {
        // Last chunk (not right-aligned)
        if (xpos + cur_chunk->width > ViewRightBorder) {
            if ((pixelek_t)cur_chunk->pixels & PIXELMASZK) {
                memcpy(dest, cur_chunk->pixels, ViewRightBorder - xpos + 1);
                return;
            }
            if (!cur_chunk->pixels) {
                draw_default_background(dest, xpos - ViewLeftBorder, ViewRightBorder - xpos + 1,
                                        is_minimap);
                return;
            }
            if ((pixelek_t)cur_chunk->pixels == PixelsDefaultForeground) {
                draw_default_foreground(dest, xpos - ViewLeftBorder, ViewRightBorder - xpos + 1,
                                        is_minimap);
                return;
            }
#ifdef DEBUG
            if ((pixelek_t)cur_chunk->pixels != PixelsTransparent) {
                internal_error("cur_chunk->pixels unknown or texture type!");
            }
#endif
        }

        // Middle chunk (left- and right-aligned)
        if ((pixelek_t)cur_chunk->pixels & PIXELMASZK) {
            memcpy(dest, cur_chunk->pixels, cur_chunk->width);
        } else {
            if (!cur_chunk->pixels) {
                draw_default_background(dest, xpos - ViewLeftBorder, cur_chunk->width, is_minimap);
            } else {
                if ((pixelek_t)cur_chunk->pixels == PixelsDefaultForeground) {
                    draw_default_foreground(dest, xpos - ViewLeftBorder, cur_chunk->width,
                                            is_minimap);
                } else {
#ifdef DEBUG
                    if ((pixelek_t)cur_chunk->pixels != PixelsTransparent) {
                        internal_error("cur_chunk->pixels unknown or texture type!");
                    }
#endif
                }
            }
        }
        xpos += cur_chunk->width;
        dest += cur_chunk->width;
        cur_chunk++;
    }
}

// Identical to render_row1 except using rows_position2/rows_x2
void canvas::render_row2(unsigned char* dest, int y) {
    canvas_chunk* cur_chunk = rows_position2[y];
    int xpos = rows_x2[y];
    while (xpos > ViewLeftBorder) {
        cur_chunk--;
        xpos -= cur_chunk->width;
    }
    while (xpos + cur_chunk->width <= ViewLeftBorder) {
        xpos += cur_chunk->width;
        cur_chunk++;
    }
    rows_position2[y] = cur_chunk;
    rows_x2[y] = xpos;

    int offset = ViewLeftBorder - xpos;
    if (xpos + cur_chunk->width > ViewRightBorder) {
        int width = ViewRightBorder - xpos + 1;
        if ((pixelek_t)cur_chunk->pixels & PIXELMASZK) {
            memcpy(dest, &cur_chunk->pixels[ViewLeftBorder - xpos], width - offset);
        } else {
            if (!cur_chunk->pixels) {
                draw_default_background(dest, 0, width - offset, is_minimap);
                return;
            }
            if ((pixelek_t)cur_chunk->pixels == PixelsDefaultForeground) {
                draw_default_foreground(dest, 0, width - offset, is_minimap);
            } else {
#ifdef DEBUG
                if ((pixelek_t)cur_chunk->pixels != PixelsTransparent) {
                    internal_error("cur_chunk->pixels unknown or texture type!");
                }
#endif
            }
        }
        return;
    }

    int size = cur_chunk->width - offset;
    if ((pixelek_t)cur_chunk->pixels & PIXELMASZK) {
        memcpy(dest, &cur_chunk->pixels[ViewLeftBorder - xpos], size);
    } else {
        if (!cur_chunk->pixels) {
            draw_default_background(dest, 0, size, is_minimap);
        } else {
            if ((pixelek_t)cur_chunk->pixels == PixelsDefaultForeground) {
                draw_default_foreground(dest, 0, size, is_minimap);
            } else {
#ifdef DEBUG
                if ((pixelek_t)cur_chunk->pixels != PixelsTransparent) {
                    internal_error("cur_chunk->pixels unknown or texture type!");
                }
#endif
            }
        }
    }

    xpos += cur_chunk->width;
    cur_chunk++;
    dest += size;

    while (xpos <= ViewRightBorder) {
        if (xpos + cur_chunk->width > ViewRightBorder) {
            if ((pixelek_t)cur_chunk->pixels & PIXELMASZK) {
                memcpy(dest, cur_chunk->pixels, ViewRightBorder - xpos + 1);
                return;
            }
            if (!cur_chunk->pixels) {
                draw_default_background(dest, xpos - ViewLeftBorder, ViewRightBorder - xpos + 1,
                                        is_minimap);
                return;
            }
            if ((pixelek_t)cur_chunk->pixels == PixelsDefaultForeground) {
                draw_default_foreground(dest, xpos - ViewLeftBorder, ViewRightBorder - xpos + 1,
                                        is_minimap);
                return;
            }
#ifdef DEBUG
            if ((pixelek_t)cur_chunk->pixels != PixelsTransparent) {
                internal_error("cur_chunk->pixels unknown or texture type!");
            }
#endif
        }

        if ((pixelek_t)cur_chunk->pixels & PIXELMASZK) {
            memcpy(dest, cur_chunk->pixels, cur_chunk->width);
        } else {
            if (!cur_chunk->pixels) {
                draw_default_background(dest, xpos - ViewLeftBorder, cur_chunk->width, is_minimap);
            } else {
                if ((pixelek_t)cur_chunk->pixels == PixelsDefaultForeground) {
                    draw_default_foreground(dest, xpos - ViewLeftBorder, cur_chunk->width,
                                            is_minimap);
                } else {
#ifdef DEBUG
                    if ((pixelek_t)cur_chunk->pixels != PixelsTransparent) {
                        internal_error("cur_chunk->pixels unknown or texture type!");
                    }
#endif
                }
            }
        }

        xpos += cur_chunk->width;
        dest += cur_chunk->width;
        cur_chunk++;
    }
}

void canvas::meters_to_pixels(vect2 meters, int* pixel_x, int* pixel_y) {
    vect2 dr = meters - origin;
    if (is_minimap) {
        *pixel_x = (int)(dr.x * (MetersToPixels / MinimapScaleFactor));
        *pixel_y = (int)(dr.y * (MetersToPixels / MinimapScaleFactor));
    } else {
        *pixel_x = (int)(dr.x * MetersToPixels);
        *pixel_y = (int)(dr.y * MetersToPixels);
    }
}

void canvas::render(int player1, pic8* pic, vect2 corner, int x1, int y1, int x2, int y2) {
    if (x1 >= x2 || y1 >= y2) {
        internal_error("canvas::render x1 >= x2 || y1 >= y2!");
    }

    // Convert corner frame of reference from meters to pixels
    int view_top_border = 0;
    meters_to_pixels(corner, &ViewLeftBorder, &view_top_border);
    ViewRightBorder = ViewLeftBorder + x2 - x1;
    int view_bottom_border = view_top_border + y2 - y1;

    if (ViewLeftBorder < 20 || ViewRightBorder > pixel_width - 20 || view_top_border < 20 ||
        view_bottom_border > pixel_height - 20) {
        internal_error(
            "canvas::render ScreenLeftBorder < 20 || ViewRightBorder > pixel_width-20 || "
            "view_top_border < 20 || view_bottom_border > "
            "pixel_height-20!");
    }

    // Calculate the texture graphic offset for the left edge of the screen
    int foreground_height = Lgr->foreground->get_height();
    int background_height = Lgr->background->get_height();
    int foreground_x = ViewLeftBorder % Lgr->foreground_original_width;
    int parallax = 2;
    int background_x = (ViewLeftBorder / parallax) % Lgr->background_original_width;

    // Draw each row
    int canvas_y1 = view_top_border - y1;
    if (player1) {
        for (int i = y1; i <= y2; i++) {
            int canvas_y = i + canvas_y1;
            // Calculate the texture graphic y-offset for the current row
            DefaultForeground =
                Lgr->foreground->get_row(canvas_y % foreground_height) + foreground_x;
            DefaultBackground =
                Lgr->background->get_row((i - y1) % background_height) + background_x;
            render_row1(pic->get_row(i) + x1, canvas_y);
        }
    } else {
        for (int i = y1; i <= y2; i++) {
            int canvas_y = i + canvas_y1;
            // Calculate the texture graphic y-offset for the current row
            DefaultForeground =
                Lgr->foreground->get_row(canvas_y % foreground_height) + foreground_x;
            DefaultBackground =
                Lgr->background->get_row((i - y1) % background_height) + background_x;
            render_row2(pic->get_row(i) + x1, canvas_y);
        }
    }
    DefaultForeground = nullptr;
    DefaultBackground = nullptr;
}

void canvas::render_minimap(int player1, pic8* pic, vect2 corner, int x1, int y1, int x2, int y2) {
    if (x1 >= x2 || y1 >= y2) {
        internal_error("canvas::render x1 >= x2 || y1 >= y2!");
    }

    // Convert corner frame of reference from meters to pixels
    int view_top_border = 0;
    meters_to_pixels(corner, &ViewLeftBorder, &view_top_border);
    ViewRightBorder = ViewLeftBorder + x2 - x1 + 1;         // Difference from render() -> +1
    int view_bottom_border = view_top_border + y2 - y1 + 1; // Difference from render() -> +1

    if (ViewLeftBorder < 20 || ViewRightBorder > pixel_width - 20 || view_top_border < 20 ||
        view_bottom_border > pixel_height - 20) {
        internal_error(
            "canvas::render ScreenLeftBorder < 20 || ViewRightBorder > pixel_width-20 || "
            "view_top_border < 20 || view_bottom_border > "
            "pixel_height-20!");
    }

    // Draw each row
    int canvas_y1 = view_top_border - y1;
    if (player1) {
        for (int y = y1; y <= y2; y++) {
            int canvas_y = y + canvas_y1;
            render_row1(pic->get_row(y) + x1, canvas_y);
        }
    } else {
        for (int y = y1; y <= y2; y++) {
            int canvas_y = y + canvas_y1;
            render_row2(pic->get_row(y) + x1, canvas_y);
        }
    }
}

static canvas_chunk_node* QgrassTextureId = NULL;

// Returns true if the pixel above (x, y) renders in front of the bike
static int above_is_front(int x, int y, node_finder* finder) {
    if (!QgrassTextureId) {
        internal_error("above_is_front no reference");
    }
    canvas_chunk_node* chunk = finder->get_chunk(x, y + 1);
    pixelek_t kepindex = (pixelek_t)chunk->pixels;
    if (kepindex < 10) {
        return 0;
    }
    if (chunk->pixels == QgrassTextureId->pixels) {
        return 0;
    }
    // Bug: It should actually check if the distance >= 500 to prevent floating
    // foreground grass
    if (chunk->distance > 500) {
        return 0;
    }
    return 1;
}

// Count the number of consecutive pixels located at (x1, y + 1) up to (x2, y + 1)
// that render behind the bike
static int consecutive_back(int x1, int x2, int y, node_finder* finder) {
    int count = 0;
    for (int x = x1; x < x2; x++) {
        if (above_is_front(x, y, finder)) {
            return count;
        }
        count++;
    }
    return count;
}

// Count the number of consecutive pixels located at (x1, y + 1) up to (x2, y + 1)
// that render in front of the bike
static int consecutive_front(int x1, int x2, int y, node_finder* finder) {
    int count = 0;
    for (int x = x1; x < x2; x++) {
        if (!above_is_front(x, y, finder)) {
            return count;
        }
        count++;
    }
    return count;
}

void canvas::create_front_grass(void) {
    node_finder* finder = new node_finder(this);

    // A crutch that we use to avoid keeping proper track of our linked lists,
    // by making sure our chunks don't get merged together.
    int alternate_distance = 0;

    // For each row
    for (int i = 10; i < pixel_height - 10; i++) {
        canvas_chunk_node* cur_node = rows_linked[i];
        int xpos = rows_x1[i];

        // Make sure canvas is unused so far
        if (xpos > 10) {
            internal_error("create_front_grass xpos > 10");
        }

        while (cur_node) {
            int node_width = cur_node->width;
            canvas_chunk_node* next_node = cur_node->next;
            QgrassTextureId = cur_node;

            // Only parse back grass
            pixelek_t kepindex = (pixelek_t)cur_node->pixels;
            if (((kepindex & PIXELMASZK) == 0) && (kepindex >= 10)) {
                kepindex -= 10;
#ifdef DEBUG
                if (kepindex >= Lgr->texture_count) {
                    internal_error("create_front_grass texture id out of range!");
                }
#endif
                texture* text = &Lgr->textures[kepindex];
                if (text->is_qgrass && cur_node->distance > 500) {
                    // Check to see if the qgrass is touching any front sprites
                    int x1 = xpos;
                    int x2 = xpos + cur_node->width;
                    while (x1 < x2) {
                        int skip = consecutive_back(x1, x2, i, finder);
                        x1 += skip;
                        int count = consecutive_front(x1, x2, i, finder);
                        if (count > 0) {
                            // Move qgrass to front
                            alternate_distance = !alternate_distance;
                            if (alternate_distance) {
                                draw_pixels(cur_node->pixels, 223, x1, x1 + count - 1, i,
                                            CanvasClipping_Unclipped);
                            } else {
                                draw_pixels(cur_node->pixels, 224, x1, x1 + count - 1, i,
                                            CanvasClipping_Unclipped);
                            }
                        }
                        x1 += count;
                    }
                }
            }
            xpos += node_width;
            cur_node = next_node;
        }
    }
    delete finder;
    finder = NULL;
}

void create_canvases(void) {
    Lgr->reload_default_textures();

    if (CanvasBack) {
        delete CanvasBack;
    }
    CanvasBack = NULL;
    if (CanvasFront) {
        delete CanvasFront;
    }
    CanvasFront = NULL;
    if (CanvasMinimap) {
        delete CanvasMinimap;
    }
    CanvasMinimap = NULL;

    // Base back canvas
    CanvasBack = new canvas(0);
    if (!CanvasBack) {
        internal_error("create_canvasses out of memory!");
    }

    if (!EolSettings->pictures_in_background()) {
        CanvasBack->create_front_grass();
    }

    // Front canvas
    CanvasFront = new canvas(CanvasBack);
    if (!CanvasFront) {
        internal_error("create_canvasses out of memory!");
    }

    if (!EolSettings->pictures_in_background()) {
        // Move over everything with a distance of less than 500
        for (int i = 0; i < CanvasBack->pixel_height; i++) {
            canvas_chunk_node* cur_node = CanvasBack->rows_linked[i];
            int xpos = CanvasBack->rows_x1[i];

            // Make sure canvas is unused so far
            if (xpos > 100) {
                internal_error("create_canvases xpos > 100!");
            }

            while (cur_node) {
                if (cur_node->distance < 500) {
                    if ((pixelek_t)cur_node->pixels >= 10) {
                        CanvasFront->draw_pixels(cur_node->pixels, cur_node->distance, xpos,
                                                 xpos + cur_node->width - 1, i,
                                                 CanvasClipping_Transparent);
                        // Delete from back canvas
                        cur_node->pixels = (unsigned char*)PixelsTransparent;
                    }
                }
                xpos += cur_node->width;
                cur_node = cur_node->next;
            }
        }
    }

    // Increase efficiency of canvas for faster rendering
    CanvasBack->default_foreground_to_pointers();
    CanvasBack->merge_redundant_chunks();
    CanvasBack->textures_to_pointers();
    CanvasBack->linked_list_to_array();
    CanvasBack->delete_all_nodes();

    CanvasFront->merge_redundant_chunks();
    CanvasFront->textures_to_pointers();
    CanvasFront->linked_list_to_array();
    CanvasFront->delete_all_nodes();

    // Minimap canvas
    CanvasMinimap = new canvas(1);
    if (!CanvasMinimap) {
        internal_error("create_canvasses out of memory!");
    }
    CanvasMinimap->linked_list_to_array();
    CanvasMinimap->delete_all_nodes();

    // Calculate the rendering position of objects
    CanvasBack->calculate_object_positions();
    CanvasMinimap->calculate_object_positions();
}

node_finder::node_finder(canvas* src) {
    current_x = -1;
    current_y = -1;
    current_node = NULL;
    source = src;
}

canvas_chunk_node* node_finder::get_chunk(int x, int y) {
    if (y == current_y && x >= current_x) {
        // Shortcut to iterate to the right of our last node
        while (x >= current_x + current_node->width) {
            current_x += current_node->width;
            current_node = current_node->next;
        }
    } else {
        // Fallback to default case by traversing list normally
        if (y < 0 || y >= source->pixel_height) {
            internal_error("node_finder invalid y");
        }
        current_y = y;
        current_node = source->rows_linked[y];
        current_x = source->rows_x1[y];

        // Make sure canvas is unused so far
        if (current_x > 10) {
            internal_error("node_finder current_x > 10");
        }

        while (x >= current_x + current_node->width) {
            current_x += current_node->width;
            current_node = current_node->next;
        }
    }
    return current_node;
}
