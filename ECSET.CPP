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
#include "debug/profiler.h"
#include <climits>
#include <cmath>
#include <cstring>

canvas* CanvasBack = nullptr;
canvas* CanvasFront = nullptr;
canvas* CanvasMinimap = nullptr;

constexpr int RIGHTMOST_CHUNK_WIDTH = 1000000;

constexpr int DISTANCE_DEFAULT = 1000000;
constexpr int DISTANCE_SKY_CLIPPING_CORRECTION = 10000;

// Canvas margins in meters
constexpr double MARGIN_X = 209.0;
constexpr double MARGIN_Y_MINIMAP = 84.0;
constexpr double MARGIN_Y = 21.0;

static void memory_error() {
    external_error("You do not have enough memory to load this level!",
                   "Try to set the graphic detail to low at the options.");
}

canvas_chunk_node* canvas::new_node() {
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
        node_array_last->next = nullptr;
    }

    // Return the next unused node
    canvas_chunk_node* new_node = &node_array_last->nodes[node_array_index++];
    new_node->distance = 0;
    return new_node;
}

// Round a value in meters so it points to exactly the middle of a pixel
static void quantize(double* meters) {
    int pixels = (int)(*meters * MetersToPixels);
    *meters = (pixels + 0.5) / MetersToPixels;
}

void canvas::set_origin_and_dimensions() {
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
        origin = vect2(minx - MARGIN_X, miny - MARGIN_Y_MINIMAP);
        width = (maxx + MARGIN_X) - origin.x;
        height = (maxy + MARGIN_Y_MINIMAP) - origin.y;
    } else {
        // Main view, we need to quantize
        // Add a small buffer around the level to avoid crashing from out of bounds
        origin = vect2(minx - MARGIN_X, miny - MARGIN_Y);
        quantize(&origin.x);
        quantize(&origin.y);
        width = (maxx + MARGIN_X) - origin.x;
        height = (maxy + MARGIN_Y) - origin.y;
    }
}

void canvas::draw_segment(segment* seg) {
    // Get the line position relative to the origin, converted to pixels
    vect2 r = seg->r - origin;
    r.x *= MetersToPixels;
    r.y *= MetersToPixels;
    vect2 v;
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
    bool line_direction_negative = v.y < 0;

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
        node->next = nullptr;
        node->xpos = x;
        node->distance = DISTANCE_DEFAULT;
        if (line_direction_negative) {
            node->pixels = canvas_pixels::default_foreground();
        } else {
            node->pixels = canvas_pixels::default_background();
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
    int xpos = row_nodes->xpos;
    canvas_chunk_node* cur_node = row_nodes;
    while (cur_node) {
        if (cur_node->next) {
            cur_node->width = cur_node->next->xpos - cur_node->xpos;
        } else {
            // Very last chunk has a infinite-ish width
            cur_node->width = RIGHTMOST_CHUNK_WIDTH;
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
            if (cur_node->xpos > next_node->xpos) {
                int tmp_xpos = cur_node->xpos;
                cur_node->xpos = next_node->xpos;
                next_node->xpos = tmp_xpos;

                canvas_pixels tmp_pixels = cur_node->pixels;
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
        if (cur_node->xpos == next_node->xpos) {
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

void canvas::default_foreground_to_pointers() {
    for (int i = 0; i < pixel_height; i++) {
        canvas_chunk_node* cur_node = rows_linked[i];
        // Make sure canvas is unused so far
        int xpos = rows_x1[i];
        if (xpos > 100) {
            internal_error("canvas::default_foreground_to_pointers xpos > 100!");
        }
        while (cur_node) {
            // Make sure width doesn't exceed the width in generate_default_texture()
            if (cur_node->pixels == canvas_pixels::default_foreground() &&
                cur_node->width < SCREEN_WIDTH) {
                int x = xpos % Lgr->foreground_original_width;
                int y = i % Lgr->foreground->get_height();
                cur_node->pixels = canvas_pixels::pointer(Lgr->foreground->get_row(y) + x);
            }
            xpos += cur_node->width;
            cur_node = cur_node->next;
        }
    }
}

void canvas::merge_redundant_chunks() {
    for (int i = 0; i < pixel_height; i++) {
        bool again = true;
        while (again) {
            again = false;
            canvas_chunk_node* cur_node = rows_linked[i];
            while (cur_node && cur_node->next) {
                canvas_chunk_node* next_node = cur_node->next;
                // Merge enums and textures only
                if (!cur_node->pixels.is_pointer() && cur_node->pixels == next_node->pixels) {
                    cur_node->next = next_node->next;
                    cur_node->width += next_node->width;
                    again = true;
                }
                cur_node = cur_node->next;
            }
        }
    }
}

void canvas::textures_to_pointers() {
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
            if (cur_node->pixels.is_texture()) {
                int texture_index = cur_node->pixels.to_texture();
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
                bool first = true;
                while (remaining_width > 0) {
                    if (!first) {
                        canvas_chunk_node* node = new_node();
                        node->next = cur_node->next;
                        cur_node->next = node;
                        cur_node = node;
                    }
                    first = false;

                    texture_x_offset %= text->original_width;
                    int node_width = remaining_width;
                    if (texture_x_offset + node_width > pic->get_width()) {
                        node_width = pic->get_width() - texture_x_offset;
                    }

                    cur_node->width = node_width;
                    cur_node->pixels = canvas_pixels::pointer(pic->get_row(y % pic->get_height()) +
                                                              texture_x_offset);
                    if (!cur_node->pixels.is_pointer()) {
                        external_error("textures_to_pointers pointer has invalid memory address!");
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

void canvas::linked_list_to_array() {
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
    rows.resize(pixel_height);
    rows_position1.resize(pixel_height);
    rows_position2.resize(pixel_height);
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

void canvas::calculate_object_positions() {
    const double offset = ANIM_WIDTH / 2.0 * EolSettings->zoom();
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* obj = Ptop->objects[i];
        if (!obj) {
            continue;
        }
        if (is_minimap) {
            obj->minimap_canvas_x = (int)((obj->r.x - origin.x) * MetersToMinimapPixels);
            obj->minimap_canvas_y = (int)((-obj->r.y - origin.y) * MetersToMinimapPixels);
        } else {
            obj->canvas_x = (int)((obj->r.x - origin.x) * MetersToPixels - offset);
            obj->canvas_y = (int)((-obj->r.y - origin.y) * MetersToPixels - offset);
        }
    }
}

// source must be aligned to a single chunk.
canvas_chunk_node* canvas::draw_one_chunk(canvas_chunk_node* dest, canvas_pixels source, int dest_x,
                                          int source_x_left, int source_x_right, int source_dist,
                                          canvas_chunk_node* dest_prev, bool* prev_can_merge,
                                          Clipping clipping) {

    if (source == canvas_pixels::transparent()) {
        internal_error("canvas::draw_one_chunk source is transparent!");
    }
    // Transparent pixels are only specifically used for the front canvas, so enforce this usage
    if (dest->pixels == canvas_pixels::transparent() && clipping != Clipping::Transparent) {
        internal_error("canvas::draw_one_chunk",
                       "dest is transparent but clipping is not transparent!");
    }

    // Validate whether we want to draw based on clipping
    if (clipping == Clipping::Ground) {
        if (dest->pixels == canvas_pixels::default_background()) {
            *prev_can_merge = false;
            return dest;
        }
    }
    if (clipping == Clipping::Sky) {
        // Background distance needs to be temporarily modified as Sky clipping should never draw on
        // top of Ground clipping
        source_dist += DISTANCE_SKY_CLIPPING_CORRECTION;
        if (dest->pixels == canvas_pixels::default_foreground()) {
            *prev_can_merge = false;
            return dest;
        }
    }
    if (clipping == Clipping::Transparent) {
        // Transparent pixels are only specifically used for the front canvas, so enforce this usage
        if (dest->pixels == canvas_pixels::default_foreground() ||
            dest->pixels == canvas_pixels::default_background()) {
            internal_error("canvas::draw_one_chunk",
                           "dest is non-transparent but clipping is transparent!");
        }
    }

    // Only draw if closer distance
    if (source_dist >= dest->distance) {
        *prev_can_merge = false;
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

        *prev_can_merge = true;
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
            !dest_prev->pixels.is_pointer() && dest_prev->pixels == source) {
            *prev_can_merge = true;
        }
        if (*prev_can_merge) {
            // Merge with previous chunk (which was right-aligned or matching texture)
            int insertion_width = source_x_right - source_x_left + 1;
            dest_prev->width += insertion_width;
            dest->width -= insertion_width;
            if (dest->pixels.is_pointer()) {
                dest->pixels += insertion_width;
            }

            *prev_can_merge = false;
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
            if (dest->pixels.is_pointer()) {
                dest->pixels += node->width;
            }

            *prev_can_merge = false;
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
            !dest_prev->pixels.is_pointer() && dest_prev->pixels == source) {
            *prev_can_merge = true;
        }
        if (*prev_can_merge) {
            // Merge with previous chunk (which was right-aligned or matching texture)
            dest_prev->width += dest->width;
            dest_prev->next = dest->next;

            *prev_can_merge = true;
            return dest_prev;
        } else {
            // Overwrite chunk with new data
            dest->pixels = source;
            dest->distance = source_dist;

            *prev_can_merge = true;
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
        node_right->pixels = dest->pixels;
        if (node_right->pixels.is_pointer()) {
            node_right->pixels += source_x_right + 1 - dest_x;
        }
        node_right->distance = dest->distance;
        node_right->next = dest->next;

        dest->width -= node_middle->width + node_right->width;
        dest->next = node_middle;

        *prev_can_merge = false;
        return node_right;
    }

    internal_error("canvas::draw_one_chunk unknown case!");
    return nullptr;
}

void canvas::draw_pixels(canvas_pixels source, int source_dist, int x_left, int x_right, int y,
                         Clipping clipping) {
    // Make sure canvas is unused so far
    canvas_chunk_node* cur_node = rows_linked[y];
    int xpos = rows_x1[y];
    if (xpos > 10) {
        internal_error("canvas::draw_pixels xpos > 10!");
    }
    // Find the first chunk onto which we will draw
    canvas_chunk_node* prev_node = nullptr;
    while (xpos + cur_node->width - 1 < x_left) {
        xpos += cur_node->width;
        prev_node = cur_node;
        cur_node = cur_node->next;
        if (!cur_node) {
            internal_error("canvas::draw_pixels !cur_node!");
        }
    }
    // Draw one chunk at a time
    bool prev_can_merge = false;
    bool done = false;
    while (!done) {
        if (!cur_node) {
            internal_error("canvas::draw_pixels !cur_node!");
        }
        int xpos_left = xpos;
        int xpos_right = xpos + cur_node->width - 1;
        if (x_right < xpos_left) {
            internal_error("canvas::draw_pixels x_right < xpos_left!");
        }
        if (xpos_right + 1 > x_right) {
            done = true;
        }

        // Offset the source data if it is a pointer to pixel data
        canvas_pixels source_next = source;
        if (source_next.is_pointer()) {
            int left_skip = 0;
            if (x_left > xpos_left) {
                left_skip = x_left - xpos_left;
            }
            source_next += cur_node->width - left_skip;
        }

        // Draw
        int xpos_next = xpos + cur_node->width;
        canvas_chunk_node* next_node = cur_node->next;
        prev_node = draw_one_chunk(cur_node, source, xpos, x_left, x_right, source_dist, prev_node,
                                   &prev_can_merge, clipping);

        source = source_next;
        xpos = xpos_next;
        cur_node = next_node;
    }
}

void canvas::draw_texture(sprite* spr, int texture_index, int mask_index, Clipping clipping) {
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
                draw_pixels(canvas_pixels::texture(texture_index), distance, x + j,
                            x + j + msk->data[offset].length - 1, y - i, clipping);
            }
            j += msk->data[offset].length;
            offset++;
        }
        offset++;
    }
}

void canvas::draw_sprites(Clipping clipping) {
    for (int sprite_index = 0; sprite_index < MAX_SPRITES; sprite_index++) {
        sprite* spr = Ptop->sprites[sprite_index];
        if (!spr) {
            return;
        }
        // Only draw the selected clipping
        if (clipping != spr->clipping) {
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
            continue;
        }

        unsigned char* pixeldata = pict->data;
        if (!canvas_pixels::pointer(pixeldata).is_pointer()) {
            external_error("draw_sprites pointer has invalid memory address!");
        }

        // Draw the picture
        int offset = 0;
        for (int i = 0; i < height; i++) {
            int j = 0;
            while (true) {
                int skip = pixeldata[offset] * 256 + pixeldata[offset + 1];
                offset += 2;
                if (skip > 60000) {
                    break;
                }
                j += skip;

                int count = pixeldata[offset] * 256 + pixeldata[offset + 1];
                offset += 2;
                if (y - i >= 0 && y - i < pixel_height) {
                    draw_pixels(canvas_pixels::pointer(&pixeldata[offset]), distance, x + j,
                                x + j + count - 1, y - i, clipping);
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
        if (pic_row[i] != transparency) {
            return count;
        }
        count++;
    }
    return count;
}

static int consecutive_solid_pixels(unsigned char* pic_row, int x, int width,
                                    unsigned char transparency) {
    int count = 0;
    for (int i = x; i < width; i++) {
        if (pic_row[i] == transparency) {
            return count;
        }
        count++;
    }
    return count;
}

constexpr int MAX_HEIGHTMAP_LENGTH = 10000;
static int GrassHeightmap[MAX_HEIGHTMAP_LENGTH + 10];

constexpr int GRASS_DISTANCE = 600;

void canvas::draw_qgrass_texture(updown& qupdown, int x, int y) {
    if (!BufferMain) {
        internal_error("draw_qgrass_texture !BufferMain");
    }

    // Grab the QGRASS texture
    int distance = GRASS_DISTANCE;
    int texture_index = Lgr->get_texture_index("qgrass");
    if (texture_index < 0) {
        internal_error("draw_qgrass_texture texture_index < 0");
    }

    // Grab the QUP/QDOWN image
    int width = qupdown.pic->get_width();
    int height = qupdown.pic->get_height();
    unsigned char transparency = qupdown.pic->gpixel(0, 0);

    // Initialize picture buffer
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            BufferMain->ppixel(j, i, 0);
        }
    }
    // Calculate qupdown mask
    for (int j = 0; j < width; j++) {
        for (int i = 0; i < height; i++) {
            if (qupdown.pic->gpixel(j, i) != transparency) {
                break;
            }
            BufferMain->ppixel(j, i, 1);
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
            draw_pixels(canvas_pixels::texture(texture_index), distance, x + j, x + j + count - 1,
                        y + i, Clipping::Ground);
            j += count;
        }
    }

    // Add 20 pixels of height to grass
    int grass_height_padding =
        (QGRASS_EXTRA_HEIGHT + GRASS_MARGIN) * EolSettings->zoom() - GRASS_MARGIN;
    for (int i = 0; i < grass_height_padding; i++) {
        draw_pixels(canvas_pixels::texture(texture_index), distance, x, x + width - 1,
                    y + height + i, Clipping::Ground);
    }
}

void canvas::draw_qupdown(updown& qupdown, int x, int y) {
    int distance = GRASS_DISTANCE;

    // Grab the QUP/QDOWN image
    int width = qupdown.pic->get_width();
    int height = qupdown.pic->get_height();
    unsigned char transparency = qupdown.pic->gpixel(0, 0);

    // Slide image down by top of image buffer
    y -= GRASS_MARGIN;
    if (!qupdown.is_up) {
        // Slide image down again by the slope
        y += qupdown.slope;
    }

    // Draw upside down
    for (int i = 0; i < height; i++) {
        int j = 0;
        unsigned char* sor = qupdown.pic->get_row(height - 1 - i);
        while (j < width) {
            j += consecutive_transparent_pixels(sor, j, width, transparency);
            if (j >= width) {
                break;
            }
            int count = consecutive_solid_pixels(sor, j, width, transparency);
            if (count <= 0) {
                internal_error("draw_qupdown count <= 0");
            }
            draw_pixels(canvas_pixels::pointer(&sor[j]), distance, x + j, x + j + count - 1, y + i,
                        Clipping::Ground);
            j += count;
        }
    }

    // Draw the QGRASS image on top of the QUP/QDOWN
    if (width > 640 || height > 480) {
        return;
    }
    draw_qgrass_texture(qupdown, x, y);
}

void canvas::draw_grass_polygon(grass* gr, int* heightmap, int heightmap_length, int x0) {
    if (heightmap_length < 1) {
        internal_error("draw_grass_polygon should always have a length of at least 1!");
    }
    int x = x0;
    int y = heightmap[0];
    while (true) {
        if (x >= x0 + heightmap_length) {
            return;
        }

        // Let's pick the best matching QUP/QDOWN
        int best_score = INT_MAX;
        updown* best_qupdown = nullptr;
        int best_slope = 0;
        for (updown& qupdown : gr->elements) {
            // Check each image and choose the one with the smallest y-offset from desired
            // height
            int target_x = x + qupdown.pic->get_width();
            int target_y;
            if (target_x >= x0 + heightmap_length) {
                target_y = heightmap[heightmap_length - 1];
            } else {
                target_y = heightmap[target_x - x0];
            }

            int score = abs(y + qupdown.slope - target_y);
            if (score < best_score) {
                best_score = score;
                best_qupdown = &qupdown;
                best_slope = qupdown.slope;
            }
        }

        // Draw the best matching QUP/QDOWN
        if (!best_qupdown) {
            internal_error("draw_grass_polygon no qupdown identified!");
        }

        draw_qupdown(*best_qupdown, x, y);
        x += best_qupdown->pic->get_width();
        y += best_slope;
    }
}

void canvas::draw_grass_polygons() {
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

        grass* gr = Lgr->grass_pics;

        // Calculate the grass heightmap
        int heightmap_length = 0;
        int x0 = 0;
        if (!create_grass_polygon_heightmap(poly, GrassHeightmap, &heightmap_length, &x0,
                                            MAX_HEIGHTMAP_LENGTH, &origin)) {
            continue;
        }
        if (heightmap_length > MAX_HEIGHTMAP_LENGTH) {
            internal_error("draw_grass_polygons heightmap_length > MAX_HEIGHTMAP_LENGTH");
        }

        // Draw the grass polygon
        draw_grass_polygon(gr, GrassHeightmap, heightmap_length, x0);
    }
}

void canvas::draw_killers() {
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
            int x = (int)((obj->r.x - origin.x) * MetersToMinimapPixels);
            int y = (int)((-obj->r.y - origin.y) * MetersToMinimapPixels);
            canvas_pixels source_pixels = canvas_pixels::pointer(source);
            draw_pixels(source_pixels, distance, x - 1, x + 1, y - 1, Clipping::Unclipped);
            draw_pixels(source_pixels, distance, x - 1, x - 1, y, Clipping::Unclipped);
            draw_pixels(source_pixels, distance, x + 1, x + 1, y, Clipping::Unclipped);
            draw_pixels(source_pixels, distance, x - 1, x + 1, y + 1, Clipping::Unclipped);
        }
    }
}

void canvas::adjust_background_distance() {
    for (int i = 0; i < pixel_height; i++) {
        canvas_chunk_node* cur_node = rows_linked[i];
        while (cur_node) {
            // In draw_one_chunk(), we temporarily added distance to properly layer assets
            // Remove that distance
            if (cur_node->distance >= DISTANCE_SKY_CLIPPING_CORRECTION) {
                cur_node->distance -= DISTANCE_SKY_CLIPPING_CORRECTION;
            }
            cur_node = cur_node->next;
        }
    }
};

canvas::canvas(bool minimap) {
    is_minimap = minimap;

    node_array = nullptr;
    node_array_last = nullptr;
    node_array_index = 0;
    chunk_array = nullptr;
    width = 0.0;
    height = 0.0;
    pixel_width = 0;
    pixel_height = 0;

    if (!Ptop || !Segments) {
        internal_error("canvas::canvas !Ptop || !Segments!");
    }

    // Initialize node memory
    node_array = new canvas_chunk_node_array;
    if (!node_array) {
        internal_error("canvas::canvas out of memory!");
    }
    node_array->next = nullptr;
    node_array_last = node_array;

    // Set canvas dimensions
    set_origin_and_dimensions();
    if (is_minimap) {
        pixel_width = (int)(width * MetersToMinimapPixels);
        pixel_height = (int)(height * MetersToMinimapPixels);

        constexpr double MAX_HEIGHT = LEVEL_MAX_SIZE + MARGIN_Y_MINIMAP * 2;
        if (height > MAX_HEIGHT) {
            internal_error("canvas::canvas is too tall!");
        }
    } else {
        pixel_width = (int)(width * MetersToPixels);
        pixel_height = (int)(height * MetersToPixels);

        constexpr double MAX_HEIGHT = LEVEL_MAX_SIZE + MARGIN_Y * 2;
        if (height > MAX_HEIGHT) {
            internal_error("canvas::canvas is too tall!");
        }
    }

    // Initialize the canvas with a single chunk for every row
    rows_linked.resize(pixel_height);
    for (int i = 0; i < pixel_height; i++) {
        rows_linked[i] = new_node();
        rows_linked[i]->next = nullptr;
        rows_linked[i]->width = 1;
        rows_linked[i]->distance = DISTANCE_DEFAULT;
        rows_linked[i]->pixels = canvas_pixels::default_foreground();
    }

    // Draw all the polygon data
    Segments->iterate_all_segments();
    segment* seg = Segments->next_segment();
    while (seg) {
        draw_segment(seg);
        seg = Segments->next_segment();
    }

    // Convert the polygon data into widths instead of x position
    rows_x1.resize(pixel_height);
    rows_x2.resize(pixel_height);
    for (int i = 0; i < pixel_height; i++) {
        sort_xpos_and_merge(rows_linked[i]);
        rows_x1[i] = rows_x2[i] = xpos_to_width(rows_linked[i]);
    }

    // Handle high quality graphics
    if (!is_minimap && State->high_quality) {
        draw_sprites(Clipping::Ground);
        draw_grass_polygons();
        draw_sprites(Clipping::Sky);
        adjust_background_distance();
        draw_sprites(Clipping::Unclipped);
    }

    // Draw killers into the minimap
    draw_killers();
}

canvas::canvas(canvas* reference) {
    node_array = nullptr;
    node_array_last = nullptr;
    node_array_index = 0;
    chunk_array = nullptr;
    is_minimap = false;
    width = 0.0;
    height = 0.0;
    pixel_width = 0;
    pixel_height = 0;

    if (!Ptop || !Segments) {
        internal_error("canvas::canvas !Ptop || !Segments!");
    }

    // Initialize node memory
    node_array = new canvas_chunk_node_array;
    if (!node_array) {
        internal_error("canvas::canvas out of memory!");
    }
    node_array->next = nullptr;
    node_array_last = node_array;

    // Copy canvas dimensions from reference canvas
    origin = reference->origin;
    width = reference->width;
    height = reference->height;
    pixel_width = reference->pixel_width;
    pixel_height = reference->pixel_height;

    // Initialize the canvas with a single chunk for every row (already as width)
    rows_linked.resize(pixel_height);
    rows_x1.resize(pixel_height);
    rows_x2.resize(pixel_height);
    for (int i = 0; i < pixel_height; i++) {
        rows_x1[i] = 1;
        rows_x2[i] = 1;

        rows_linked[i] = new_node();
        rows_linked[i]->next = nullptr;
        rows_linked[i]->width = RIGHTMOST_CHUNK_WIDTH;
        rows_linked[i]->distance = DISTANCE_DEFAULT;
        rows_linked[i]->pixels = canvas_pixels::transparent();
    }
}

canvas::~canvas() {
    if (node_array) {
        internal_error("canvas::~canvas node_array");
    }
    if (!chunk_array) {
        internal_error("canvas::~canvas !chunk_array!");
    }
    delete chunk_array;
    chunk_array = nullptr;
}

void canvas::delete_all_nodes() {
    if (!node_array) {
        internal_error("canvas::delete_all_nodes !node_array!");
    }
    canvas_chunk_node_array* cur_array = node_array;
    while (cur_array) {
        canvas_chunk_node_array* next_array = cur_array->next;
        delete cur_array;
        cur_array = next_array;
    }
    node_array = nullptr;
    rows_linked.resize(0);
    rows_linked.shrink_to_fit();
}

static int ViewLeftBorder;
static int ViewRightBorder;

unsigned char* DefaultForeground = nullptr;
unsigned char* DefaultBackground = nullptr;

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

void canvas::render_row(bool player1, unsigned char* dest, int y) {
    canvas_chunk** rows_position = player1 ? &rows_position1[y] : &rows_position2[y];
    int* rows_x = player1 ? &rows_x1[y] : &rows_x2[y];

    // Find chunk corresponding to the left of the screen
    canvas_chunk* cur_chunk = *rows_position;
    int xpos = *rows_x;
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
    *rows_position = cur_chunk;
    *rows_x = xpos;

    // If the entire row is a single chunk, paste it
    int offset = ViewLeftBorder - xpos;
    if (xpos + cur_chunk->width > ViewRightBorder) {
        int width = ViewRightBorder - xpos + 1;
        if (cur_chunk->pixels.is_pointer()) {
            memcpy(dest, &cur_chunk->pixels.to_pointer()[ViewLeftBorder - xpos], width - offset);
        } else if (cur_chunk->pixels == canvas_pixels::default_background()) {
            draw_default_background(dest, 0, width - offset, is_minimap);
        } else if (cur_chunk->pixels == canvas_pixels::default_foreground()) {
            draw_default_foreground(dest, 0, width - offset, is_minimap);
        } else {
#ifdef DEBUG
            if (cur_chunk->pixels != canvas_pixels::transparent()) {
                internal_error("cur_chunk->pixels unknown or texture type!");
            }
#endif
        }
        return;
    }

    // Otherwise we need to split the situation into the first chunk (not left-aligned),
    // and the remaining chunks (all left-aligned)
    // First chunk (not left-aligned):
    int size = cur_chunk->width - offset;
    if (cur_chunk->pixels.is_pointer()) {
        memcpy(dest, &cur_chunk->pixels.to_pointer()[ViewLeftBorder - xpos], size);
    } else if (cur_chunk->pixels == canvas_pixels::default_background()) {
        draw_default_background(dest, 0, size, is_minimap);
    } else if (cur_chunk->pixels == canvas_pixels::default_foreground()) {
        draw_default_foreground(dest, 0, size, is_minimap);
    } else {
#ifdef DEBUG
        if (cur_chunk->pixels != canvas_pixels::transparent()) {
            internal_error("cur_chunk->pixels unknown or texture type!");
        }
#endif
    }
    xpos += cur_chunk->width;
    cur_chunk++;
    dest += size;

    // Remaining chunks (left-aligned)
    while (xpos <= ViewRightBorder) {
        // Last chunk (not right-aligned)
        if (xpos + cur_chunk->width > ViewRightBorder) {
            if (cur_chunk->pixels.is_pointer()) {
                memcpy(dest, cur_chunk->pixels.to_pointer(), ViewRightBorder - xpos + 1);
            } else if (cur_chunk->pixels == canvas_pixels::default_background()) {
                draw_default_background(dest, xpos - ViewLeftBorder, ViewRightBorder - xpos + 1,
                                        is_minimap);
            } else if (cur_chunk->pixels == canvas_pixels::default_foreground()) {
                draw_default_foreground(dest, xpos - ViewLeftBorder, ViewRightBorder - xpos + 1,
                                        is_minimap);
            } else {
#ifdef DEBUG
                if (cur_chunk->pixels != canvas_pixels::transparent()) {
                    internal_error("cur_chunk->pixels unknown or texture type!");
                }
#endif
            }
            return;
        }

        // Middle chunk (left- and right-aligned)
        if (cur_chunk->pixels.is_pointer()) {
            memcpy(dest, cur_chunk->pixels.to_pointer(), cur_chunk->width);
        } else if (cur_chunk->pixels == canvas_pixels::default_background()) {
            draw_default_background(dest, xpos - ViewLeftBorder, cur_chunk->width, is_minimap);
        } else if (cur_chunk->pixels == canvas_pixels::default_foreground()) {
            draw_default_foreground(dest, xpos - ViewLeftBorder, cur_chunk->width, is_minimap);
        } else {
#ifdef DEBUG
            if (cur_chunk->pixels != canvas_pixels::transparent()) {
                internal_error("cur_chunk->pixels unknown or texture type!");
            }
#endif
        }
        xpos += cur_chunk->width;
        dest += cur_chunk->width;
        cur_chunk++;
    }
}

void canvas::meters_to_pixels(vect2 meters, int* pixel_x, int* pixel_y) {
    vect2 dr = meters - origin;
    if (is_minimap) {
        *pixel_x = (int)(dr.x * MetersToMinimapPixels);
        *pixel_y = (int)(dr.y * MetersToMinimapPixels);
    } else {
        *pixel_x = (int)(dr.x * MetersToPixels);
        *pixel_y = (int)(dr.y * MetersToPixels);
    }
}

void canvas::render(bool player1, pic8* pic, vect2 corner, int x1, int y1, int x2, int y2) {
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
    constexpr int PARALLAX = 2;
    int background_x = (ViewLeftBorder / PARALLAX) % Lgr->background_original_width;

    // Draw each row
    int canvas_y1 = view_top_border - y1;
    for (int i = y1; i <= y2; i++) {
        int canvas_y = i + canvas_y1;
        // Calculate the texture graphic y-offset for the current row
        DefaultForeground = Lgr->foreground->get_row(canvas_y % foreground_height) + foreground_x;
        DefaultBackground = Lgr->background->get_row((i - y1) % background_height) + background_x;
        render_row(player1, pic->get_row(i) + x1, canvas_y);
    }
    DefaultForeground = nullptr;
    DefaultBackground = nullptr;
}

void canvas::render_minimap(bool player1, pic8* pic, vect2 corner, int x1, int y1, int x2, int y2) {
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
    for (int y = y1; y <= y2; y++) {
        int canvas_y = y + canvas_y1;
        render_row(player1, pic->get_row(y) + x1, canvas_y);
    }
}

static canvas_pixels* QgrassTextureId = nullptr;

// Returns true if the pixel above (x, y) renders in front of the bike
static bool above_is_front(int x, int y, node_finder* finder) {
    if (!QgrassTextureId) {
        internal_error("above_is_front no reference");
    }
    canvas_chunk_node* chunk = finder->get_chunk(x, y + 1);
    if (chunk->pixels.is_enum()) {
        return false;
    }
    if (chunk->pixels == *QgrassTextureId) {
        return false;
    }
    // Bug: It should actually check if the distance >= 500 to prevent floating
    // foreground grass
    if (chunk->distance > 500) {
        return false;
    }
    return true;
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

void canvas::create_front_grass() {
    node_finder* finder = new node_finder(this);

    // A crutch that we use to avoid keeping proper track of our linked lists,
    // by making sure our chunks don't get merged together.
    bool alternate_distance = false;

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
            QgrassTextureId = &cur_node->pixels;

            // Only parse back grass
            if (cur_node->pixels.is_texture()) {
                int texture_index = cur_node->pixels.to_texture();
#ifdef DEBUG
                if (texture_index >= Lgr->texture_count) {
                    internal_error("create_front_grass texture id out of range!");
                }
#endif
                texture* text = &Lgr->textures[texture_index];
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
                                            Clipping::Unclipped);
                            } else {
                                draw_pixels(cur_node->pixels, 224, x1, x1 + count - 1, i,
                                            Clipping::Unclipped);
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
    finder = nullptr;
}

void canvas::create_canvases() {
    START_TIME(canvas_timer);
    Lgr->reload_default_textures();

    delete CanvasBack;
    CanvasBack = nullptr;
    delete CanvasFront;
    CanvasFront = nullptr;
    delete CanvasMinimap;
    CanvasMinimap = nullptr;

    // Base back canvas
    CanvasBack = new canvas(false);
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
                    if (!cur_node->pixels.is_enum()) {
                        CanvasFront->draw_pixels(cur_node->pixels, cur_node->distance, xpos,
                                                 xpos + cur_node->width - 1, i,
                                                 Clipping::Transparent);
                        // Delete from back canvas
                        cur_node->pixels = canvas_pixels::transparent();
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
    CanvasMinimap = new canvas(true);
    if (!CanvasMinimap) {
        internal_error("create_canvasses out of memory!");
    }
    CanvasMinimap->linked_list_to_array();
    CanvasMinimap->delete_all_nodes();

    // Calculate the rendering position of objects
    CanvasBack->calculate_object_positions();
    CanvasMinimap->calculate_object_positions();

    END_TIME(canvas_timer, std::format("Canvases"));
}

node_finder::node_finder(canvas* src) {
    current_x = -1;
    current_y = -1;
    current_node = nullptr;
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
