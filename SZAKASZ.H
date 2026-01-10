#ifndef SZAKASZ_H
#define SZAKASZ_H

#include "vect2.h"

class topol;

// A single line from a polygon
struct line {
    vect2 r, v, unit_vector;
    double length;
};

// Linked list of level lines
struct line_node {
    line* ln;
    line_node* next;
};

typedef line_node* pline_node;

// Memory structure to allocate data for line_node
constexpr int LINE_NODE_BLOCK_SIZE = 500;
struct line_node_array {
    line_node nodes[LINE_NODE_BLOCK_SIZE];
    line_node_array* next;
};

/* This class contains a list of all the lines in a level (line_list).
 * You can iterate through all the lines using iterate_all_lines() and get_line().
 * This class also contains physical 2D map of the level with a list of all the lines the kuski
 * might collide with at any given location.
 * You can iterate through potential collision candidates using select_collision_grid_cell() and
 * get_line_from_collision_grid_cell().*/
class lines {
    // All lines in the level
    line* line_list;
    int line_list_allocated_length;
    int line_list_length;
    int line_list_current_index; // Current index when using iterate_all_lines()/get_line()

    // 2D grid representing the level for kuski<->line collision purposes
    pline_node* collision_grid;
    int collision_grid_width, collision_grid_height;
    double collision_grid_cell_size;
    vect2 collision_grid_origin;
    // Current linked list position when using
    // select_collision_grid_cell()/get_line_from_collision_grid_cell()
    line_node* collision_grid_next;
    void add_line_to_collision_grid(line* ln, double max_radius);
    void add_line_to_cell(int cell_x, int cell_y, line* ln);

    // Memory structures:
    line_node_array* node_array;
    int node_array_index;
    line_node* get_new_line_node();
    void delete_all_line_nodes();

  public:
    lines(topol* lev);
    ~lines();

    // Initialize the collision_grid with the max radius of any interactable bike part
    void setup_collision_grid(double max_radius);
    // Reset to iterate through all lines passing through the cell from collision_grid corresponding
    // to position r
    void select_collision_grid_cell(vect2 r);
    // Get the next line of potential collision from the cell in collision_grid
    line* get_line_from_collision_grid_cell();

    // Reset to iterate through all lines in the level
    void iterate_all_lines();
    // Grab the next line in the level
    line* get_line();

    line* getptrszakasz(int sorszam);
};

extern lines* Lines;

#endif
