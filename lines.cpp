#include "lines.h"
#include "main.h"
#include "polygon.h"
#include "TOPOL.H"
#include <cmath>
#include <cstring>

constexpr int MAX_LINES = MAX_VERTICES;

lines* Lines = nullptr;

// Load a list of all the lines of a level
lines::lines(topol* lev) {
    line_list = nullptr;
    line_list_allocated_length = 0;
    line_list_length = 0;
    collision_grid = nullptr;
    collision_grid_width = 1;
    collision_grid_height = 1;
    collision_grid_cell_size = 1.0;
    collision_grid_origin = vect2(0.0, 0.0);
    collision_grid_next = nullptr;
    node_array = nullptr;
    node_array_index = 0;

    // Memory
    line_list = new line[MAX_LINES];
    if (!line_list) {
        external_error("lines::lines out of memory!");
        return;
    }
    memset(line_list, 0, sizeof(line) * MAX_LINES);
    line_list_allocated_length = MAX_LINES;

    // Load all solid polygons
    for (int i = 0; i < MAXGYURU; i++) {
        polygon* poly = lev->ptomb[i];
        if (!poly) {
            continue;
        }
        if (poly->is_grass) {
            continue;
        }
        for (int j = 0; j < poly->vertex_count; j++) {
            if (line_list_length >= MAX_LINES) {
                internal_error("lines::lines line_list_length >= MAX_LINES!");
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
            line_list[line_list_length].r = r1;
            line_list[line_list_length].v = r2 - r1;
            // Invert y coordinate
            line_list[line_list_length].r.y = -line_list[line_list_length].r.y;
            line_list[line_list_length].v.y = -line_list[line_list_length].v.y;

            line_list_length++;
        }
    }
}

lines::~lines() {
    if (line_list) {
        delete line_list;
    }
    if (collision_grid) {
        delete collision_grid;
    }
    delete_all_line_nodes();
}

// Get an unused line node from memory
line_node* lines::get_new_line_node() {
    // No nodes exist, initialize first node
    if (!node_array) {
        node_array = new line_node_array;
        if (!node_array) {
            external_error("lines::get_new_line_node out of memory!");
        }
        node_array->next = nullptr;
        node_array_index = 0;
    }
    // Grab the last memory structure
    line_node_array* last_array = node_array;
    while (last_array->next) {
        last_array = last_array->next;
    }
    // If we've hit the end of the array in the last memory structure, create a new memory structure
    if (node_array_index == LINE_NODE_BLOCK_SIZE) {
        last_array->next = new line_node_array;
        last_array = last_array->next;
        if (!last_array) {
            external_error("lines::get_new_line_node out of memory!");
        }
        last_array->next = nullptr;
        node_array_index = 0;
    }
    // Return the new line_node
    return &last_array->nodes[node_array_index++];
}

void lines::delete_all_line_nodes() {
    line_node_array* current_array = node_array;
    node_array = nullptr;
    while (current_array) {
        line_node_array* deleted_array = current_array;
        current_array = current_array->next;
        delete deleted_array;
    }
}

/* collision_grid is a list representing a grid of the map, where each element represents a zone of
   dimensions 1.0x1.0 Each element of collision_grid contains a linked list of all the lines that
   cross into this zone

   We create a new line_node and attach it to
   collision_grid[collision_grid_width * cell_y + cell_x]*/
void lines::add_line_to_cell(int cell_x, int cell_y, line* ln) {
#ifdef DEBUG
    if (cell_x < 0 || cell_y < 0) {
        internal_error("lines::add_line_to_cell cell_x < 0 || cell_y < 0!");
    }
#endif
    if (cell_x >= collision_grid_width || cell_y >= collision_grid_height) {
        return;
    }
    // Create a new line_node for the linked list
    line_node* new_node = get_new_line_node();
    new_node->next = nullptr;
    new_node->ln = ln;
    line_node* cell_node = collision_grid[collision_grid_width * cell_y + cell_x];
    // Add the line_node to the end of the linked list for the selected cell
    if (!cell_node) {
        collision_grid[collision_grid_width * cell_y + cell_x] = new_node;
        return;
    }
    while (cell_node->next) {
        cell_node = cell_node->next;
    }
    cell_node->next = new_node;
}

// Add one line to the collision lookup table
void lines::add_line_to_collision_grid(line* ln, double max_radius) {
    // Set the length of the line and the unit vector
    ln->length = ln->v.length();
    if (ln->length < 0.00000001) {
        internal_error("lines::add_line_to_collision_grid line too short!");
    }
    ln->unit_vector = unit_vector(ln->v);

    // Convert to collision_grid units.
    vect2 v = ln->v;
    vect2 r = ln->r - collision_grid_origin;
    v = v * (1 / collision_grid_cell_size);
    r = r * (1 / collision_grid_cell_size);
    // max_radius represents the max radius of a bike element (wheel or head - for our purposes,
    // always wheel). Extend the radius by 50% just to be safe
    max_radius *= 1.5 / collision_grid_cell_size;

    // The line is taller than long.
    // Let's invert x and y temporarily
    // This way, the slope is always less than 1.0
    // I'm not 100% sure this is necessary
    bool invert_axis = false;
    if (fabs(v.y) > fabs(v.x)) {
        invert_axis = true;
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
    int cell_x = 0;
    if (xstart > 0) {
        // Cap the starting x to be at least 0
        cell_x = (int)(xstart);
    }
    if (r.x + v.x + max_radius < 0) {
        internal_error("lines::add_line_to_collision_grid r.x+v.x+max_radius < 0!");
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
            internal_error("lines::add_line_to_collision_grid y1 > y2!");
        }
#endif
        if (y2 < 0.0) {
            internal_error("lines::add_line_to_collision_grid y2 < 0!");
        }
        // Cap the starting y to be at least 0
        int cell_y = 0;
        if (y1 > 0) {
            cell_y = (int)(y1);
        }
        int yend = (int)(y2);
        while (cell_y <= yend) {
            // Add the line to all the cells that are crossed at this x position
            if (invert_axis) {
                // Uninvert x and y if necessary
                add_line_to_cell(cell_y, cell_x, ln);
            } else {
                add_line_to_cell(cell_x, cell_y, ln);
            }
            cell_y++;
        }
        cell_x++;
    }
}

// Set-up
// 1) Get origin and dimensions
// 2) Initialize collision_grid, the collision lookup
void lines::setup_collision_grid(double max_radius) {
    if (line_list_length <= 0) {
        internal_error("lines::setup_collision_grid has no lines!");
    }
    if (collision_grid) {
        internal_error("lines::setup_collision_grid already has a collision grid!");
    }

    collision_grid_cell_size = 1.0;

    // Get min/max values
    iterate_all_lines();
    line* cur_line = get_line();
    double minx = cur_line->r.x;
    double maxx = cur_line->r.x;
    double miny = cur_line->r.y;
    double maxy = cur_line->r.y;
    while (cur_line) {
        if (cur_line->r.x < minx) {
            minx = cur_line->r.x;
        }
        if (cur_line->r.x > maxx) {
            maxx = cur_line->r.x;
        }
        if (cur_line->r.y < miny) {
            miny = cur_line->r.y;
        }
        if (cur_line->r.y > maxy) {
            maxy = cur_line->r.y;
        }

        // We don't need to check all the endpoints because in Elma all polygons are closed loops.
        // This check used to be necessary for Across 1.0 Verzio levels
        if (cur_line->r.x + cur_line->v.x < minx) {
            minx = cur_line->r.x + cur_line->v.x;
        }
        if (cur_line->r.x + cur_line->v.x > maxx) {
            maxx = cur_line->r.x + cur_line->v.x;
        }
        if (cur_line->r.y + cur_line->v.y < miny) {
            miny = cur_line->r.y + cur_line->v.y;
        }
        if (cur_line->r.y + cur_line->v.y > maxy) {
            maxy = cur_line->r.y + cur_line->v.y;
        }
        cur_line = get_line();
    }

    // Add a small margin for safety
    minx -= LINES_BORDER_SIZE;
    miny -= LINES_BORDER_SIZE;
    maxx += LINES_BORDER_SIZE;
    maxy += LINES_BORDER_SIZE;
    collision_grid_origin = vect2(minx, miny);
    double width = maxx - minx;
    double height = maxy - miny;

    // Convert to collision_grid units
    collision_grid_width = (int)(width / collision_grid_cell_size + 1.0);
    collision_grid_height = (int)(height / collision_grid_cell_size + 1.0);
    if (collision_grid_width < 0 || collision_grid_height < 0) {
        internal_error("collision_grid_width < 0 || collision_grid_height < 0!");
    }
    constexpr int MAX_SIZE = LEVEL_MAX_SIZE + 2 * LINES_BORDER_SIZE;
    if (collision_grid_width > MAX_SIZE || collision_grid_height > MAX_SIZE) {
        internal_error("collision_grid_width > MAX_SIZE || collision_grid_height > MAX_SIZE!");
    }

    // Allocate the collision_grid
    int grid_length = collision_grid_width * collision_grid_height;
    collision_grid = new pline_node[grid_length];
    if (!collision_grid) {
        external_error("lines::setup out of memory!");
    }
    for (int i = 0; i < grid_length; i++) {
        collision_grid[i] = nullptr;
    }

    // Populate the collision_grid
    iterate_all_lines();
    cur_line = get_line();
    while (cur_line) {
        add_line_to_collision_grid(cur_line, max_radius);
        cur_line = get_line();
    }
}

void lines::select_collision_grid_cell(vect2 r) {
    if (!collision_grid) {
        internal_error("lines::select_collision_grid_cell !collision_grid!");
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
        internal_error("lines::select_collision_grid_cell cell_x > collision_grid_width!");
    }
    if (cell_x == collision_grid_width) {
        cell_x = collision_grid_width - 1;
    }
    if (cell_y > collision_grid_height) {
        internal_error("lines::select_collision_grid_cell cell_y > collision_grid_height!");
    }
    if (cell_y == collision_grid_height) {
        cell_y = collision_grid_height - 1;
    }
    // We are now ready to iterate
    collision_grid_next = collision_grid[collision_grid_width * cell_y + cell_x];
}

line* lines::get_line_from_collision_grid_cell() {
    if (!collision_grid_next) {
        return nullptr;
    }
#ifdef DEBUG
    if (!collision_grid_next->ln) {
        internal_error("lines::get_line_from_collision_grid_cell !collision_grid_next->ln");
    }
#endif
    line* ret = collision_grid_next->ln;
    collision_grid_next = collision_grid_next->next;
    return ret;
}

void lines::iterate_all_lines() {
    if (line_list_length == 0) {
        internal_error("lines::iterate_all_lines there are no lines to iterate!");
    }
    line_list_current_index = 0;
}

line* lines::get_line() {
    if (line_list_current_index >= line_list_length) {
        return nullptr;
    }
    return &line_list[line_list_current_index++];
}
