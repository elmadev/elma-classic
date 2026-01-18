#include "segments.h"
#include "level.h"
#include "main.h"
#include "polygon.h"
#include <cmath>
#include <cstring>

constexpr int MAX_SEGMENTS = MAX_VERTICES;

segments* Segments = nullptr;

segments::segments(level* lev) {
    seg_list = nullptr;
    seg_list_allocated_length = 0;
    seg_list_length = 0;
    collision_grid = nullptr;
    collision_grid_width = 1;
    collision_grid_height = 1;
    collision_grid_cell_size = 1.0;
    collision_grid_origin = vect2(0, 0);
    collision_grid_iterable = nullptr;
    node_array = nullptr;
    node_array_index = 0;

    seg_list = new segment[MAX_SEGMENTS];
    if (!seg_list) {
        external_error("segments::segments out of memory!");
        return;
    }
    memset(seg_list, 0, sizeof(segment) * MAX_SEGMENTS);
    seg_list_allocated_length = MAX_SEGMENTS;

    // Load all solid polygons
    for (int i = 0; i < MAX_POLYGONS; i++) {
        polygon* poly = lev->polygons[i];
        if (!poly) {
            continue;
        }
        if (poly->is_grass) {
            continue;
        }
        for (int j = 0; j < poly->vertex_count; j++) {
            if (seg_list_length >= MAX_SEGMENTS) {
                internal_error("segments::segments seg_list_length >= MAX_SEGMENTS!");
            }
            vect2 r1;
            vect2 r2;
            if (j < poly->vertex_count - 1) {
                r1 = poly->vertices[j];
                r2 = poly->vertices[j + 1];
            } else {
                r1 = poly->vertices[j];
                r2 = poly->vertices[0];
            }
            seg_list[seg_list_length].r = r1;
            seg_list[seg_list_length].v = r2 - r1;
            // Invert y coordinates
            seg_list[seg_list_length].r.y = -seg_list[seg_list_length].r.y;
            seg_list[seg_list_length].v.y = -seg_list[seg_list_length].v.y;
            seg_list_length++;
        }
    }
}

segments::~segments() {
    if (seg_list) {
        delete seg_list;
    }
    if (collision_grid) {
        delete collision_grid;
    }
    delete_all_nodes();
}

// Get an unused node from memory
segment_node* segments::new_node() {
    // No nodes exist, initialize first node
    if (!node_array) {
        node_array = new segment_node_array;
        if (!node_array) {
            external_error("segments::new_node out of memory!");
        }
        node_array->next = nullptr;
        node_array_index = 0;
    }
    // Grab the last memory structure
    segment_node_array* cur_array = node_array;
    while (cur_array->next) {
        cur_array = cur_array->next;
    }
    // If we've hit the end of the array in the last memory structure, create a new memory structure
    if (node_array_index == SEGMENT_NODE_BLOCK_LENGTH) {
        cur_array->next = new segment_node_array;
        cur_array = cur_array->next;
        if (!cur_array) {
            external_error("segments::new_node out of memory!");
        }
        cur_array->next = nullptr;
        node_array_index = 0;
    }
    // Return the new unused node
    return &cur_array->nodes[node_array_index++];
}

void segments::delete_all_nodes() {
    segment_node_array* cur_array = node_array;
    node_array = nullptr;
    while (cur_array) {
        segment_node_array* delete_array = cur_array;
        cur_array = cur_array->next;
        delete delete_array;
    }
}

// collision_grid is a list representing a grid of the map, where each element represents a zone of
// dimensions 1.0x1.0.
//
// Each element of collision_grid contains a linked list of all the line
// segments that cross into this zone.
//
// We create a new segment_node and attach it to:
// collision_grid[collision_grid_width * cell_y + cell_x]
void segments::add_node_to_cell(int cell_x, int cell_y, segment* seg) {
#ifdef DEBUG
    if (cell_x < 0 || cell_y < 0) {
        internal_error("cell_x < 0 || cell_y < 0!");
    }
#endif
    if (cell_x >= collision_grid_width || cell_y >= collision_grid_height) {
        return;
    }
    // Create a new node for the linked list
    segment_node* new_n = new_node();
    new_n->next = nullptr;
    new_n->seg = seg;
    // Add the node to the end of the linked list for the selected cell
    segment_node* cur_n = collision_grid[collision_grid_width * cell_y + cell_x];
    if (!cur_n) {
        collision_grid[collision_grid_width * cell_y + cell_x] = new_n;
        return;
    }
    while (cur_n->next) {
        cur_n = cur_n->next;
    }
    cur_n->next = new_n;
}

void segments::add_segment_to_collision_grid(segment* seg, double max_radius) {
    // Set the length of the line and the unit vector for future physics calculations
    seg->length = seg->v.length();
    if (seg->length < 0.00000001) {
        internal_error("segments::add_nodes_to_collision_grid too short!");
    }
    seg->unit_vector = unit_vector(seg->v);

    // Convert to collision_grid units
    vect2 v = seg->v;
    vect2 r = seg->r - collision_grid_origin;
    v = v * (1 / collision_grid_cell_size);
    r = r * (1 / collision_grid_cell_size);
    // max_radius represents the max radius of a bike element (wheel or head - for our purposes,
    // always wheel). Extend the radius by 50% just to be safe
    max_radius *= 1.5 / collision_grid_cell_size;

    // Check if the line is taller than long.
    // If so, invert x and y temporarily
    // This way, the slope is always less than 1.0
    bool invert_axes = false;
    if (fabs(v.y) > fabs(v.x)) {
        invert_axes = true;
        double tmp = v.x;
        v.x = v.y;
        v.y = tmp;
        tmp = r.x;
        r.x = r.y;
        r.y = tmp;
    }
    // Invert the line so it is always left to right
    if (v.x < 0) {
        r = r + v;
        v = Vect2null - v;
    }
    // Get the line equation, plotting from r.x - max_radius to r.x + v.x + max_radius
    double slope = v.y / v.x;
    double y0 = r.y - slope * r.x;
    double xstart = r.x - max_radius;
    // Cap the starting x to be at least 0
    int cell_x = 0;
    if (xstart > 0) {
        cell_x = (int)(xstart);
    }
    if (r.x + v.x + max_radius < 0) {
        internal_error("segments::add_segment_to_collision_grid r.x+v.x+max_radius < 0!");
    }
    int xend = (int)(r.x + v.x + max_radius);
    // For every x position
    while (cell_x <= xend) {
        // Get the minimum and maximum y in the cell
        double y1 = slope * cell_x + y0;
        double y2 = slope * (cell_x + 1) + y0;
        if (y1 > y2) {
            double tmp = y1;
            y1 = y2;
            y2 = tmp;
        }
        // Extend by the wheel radius up and down
        y1 -= max_radius;
        y2 += max_radius;
#ifdef DEBUG
        if (y1 > y2) {
            internal_error("segments::add_nodes_to_collision_grid not y1 > y2!");
        }
#endif
        if (y2 < 0.0) {
            internal_error("segments::add_nodes_to_collision_grid! not y2 < 0.0");
        }
        // Cap the starting y to be at least 0
        int cell_y = 0;
        if (y1 > 0) {
            cell_y = (int)(y1);
        }
        int yend = (int)(y2);
        while (cell_y <= yend) {
            // Add the segment to all the cells that are crossed at this x position,
            // uninverting the axes if necessary
            if (invert_axes) {
                add_node_to_cell(cell_y, cell_x, seg);
            } else {
                add_node_to_cell(cell_x, cell_y, seg);
            }
            cell_y++;
        }
        cell_x++;
    }
}

// Set-up
// - Get origin and dimensions
// - Initialize collision_grid, the collision lookup
void segments::setup_collision_grid(double max_radius) {
    if (seg_list_length <= 0) {
        internal_error("segments::setup_collision_grid no lines!");
    }
    if (collision_grid) {
        internal_error("segments::setup_collision_grid already setup!");
    }

    collision_grid_cell_size = 1.0;

    // Get min/max values
    iterate_all_segments();
    segment* seg = next_segment();
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
        // We don't need to check all the endpoints because in Elma all polygons are closed loops.
        // This check used to be necessary for Across 1.0 Verzio levels
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

        seg = next_segment();
    }

    // Add a small margin to prevent physics glitch at edge of map
    // We assume this is wider than max_radius
    minx -= SEGMENTS_BORDER;
    miny -= SEGMENTS_BORDER;
    maxx += SEGMENTS_BORDER;
    maxy += SEGMENTS_BORDER;
#ifdef DEBUG
    if (SEGMENTS_BORDER < max_radius * 1.5) {
        internal_error(
            "segments::setup_collision_grid border buffer too small for collision physics!");
    }
#endif

    collision_grid_origin = vect2(minx, miny);
    double width = maxx - minx;
    double height = maxy - miny;

    // Convert to collision_grid units
    collision_grid_width = (int)(width / collision_grid_cell_size + 1.0);
    collision_grid_height = (int)(height / collision_grid_cell_size + 1.0);
    if (collision_grid_width < 0 || collision_grid_height < 0) {
        internal_error("collision_grid_width < 0 || collision_grid_height < 0!");
    }
    constexpr int MAX_SIZE = LEVEL_MAX_SIZE + 2 * SEGMENTS_BORDER;
    if (collision_grid_width > MAX_SIZE || collision_grid_height > MAX_SIZE) {
        internal_error("collision_grid_width > MAX_SIZE || collision_grid_height > MAX_SIZE!");
    }

    // Allocate the collision_grid
    int grid_size = collision_grid_width * collision_grid_height;
    collision_grid = new psegment_node[grid_size];
    if (!collision_grid) {
        external_error("segments::setup_collision_grid out of memory!");
    }
    for (int i = 0; i < grid_size; i++) {
        collision_grid[i] = nullptr;
    }

    // Populate the collision_grid
    iterate_all_segments();
    seg = next_segment();
    while (seg) {
        add_segment_to_collision_grid(seg, max_radius);
        seg = next_segment();
    }
}

void segments::iterate_collision_grid_cell_segments(vect2 r) {
    if (!collision_grid) {
        internal_error("segments::iterate_collision_grid_cell_segments !collision_grid!");
    }
    // Convert from elmameters to grid position
    // This function is responsible for crashing when you go out of bounds in the up/right direction
    // down/left position is handled without a crash
    r = (r - collision_grid_origin) * (1 / collision_grid_cell_size);
    int cell_x = 0;
    if (r.x > 0) {
        cell_x = (int)(r.x);
    }
    int cell_y = 0;
    if (r.y > 0) {
        cell_y = (int)(r.y);
    }
    if (cell_x > collision_grid_width) {
        internal_error(
            "segments::iterate_collision_grid_cell_segments cell_x > collision_grid_width!");
    }
    if (cell_x == collision_grid_width) {
        cell_x = collision_grid_width - 1;
    }
    if (cell_y > collision_grid_height) {
        internal_error(
            "segments::iterate_collision_grid_cell_segments cell_y > collision_grid_height!");
    }
    if (cell_y == collision_grid_height) {
        cell_y = collision_grid_height - 1;
    }
    // We are now ready to iterate
    collision_grid_iterable = collision_grid[collision_grid_width * cell_y + cell_x];
}

segment* segments::next_collision_grid_segment() {
    if (!collision_grid_iterable) {
        return nullptr;
    }
#ifdef DEBUG
    if (!collision_grid_iterable->seg) {
        internal_error("segments::next_collision_grid_segment !collision_grid_iterable->seg");
    }
#endif
    segment* ret = collision_grid_iterable->seg;
    collision_grid_iterable = collision_grid_iterable->next;
    return ret;
}

void segments::iterate_all_segments() {
    if (seg_list_length == 0) {
        internal_error("lines::iterate_all_segments there are no line segments to iterate!");
    }
    seg_list_iteration_index = 0;
}

segment* segments::next_segment() {
    if (seg_list_iteration_index >= seg_list_length) {
        return nullptr;
    }
    return &seg_list[seg_list_iteration_index++];
}
