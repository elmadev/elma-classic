#ifndef SZAKASZ_H
#define SZAKASZ_H

#include "vect2.h"

struct segment;

// Linked list of level line segments
struct segment_node {
    segment* seg;
    segment_node* next;
};

typedef segment_node* psegment_node;

// Memory structure to allocate data for segment_node
#define SEGMENT_NODE_BLOCK_LENGTH (500)
struct segment_node_array {
    segment_node nodes[SEGMENT_NODE_BLOCK_LENGTH];
    segment_node_array* next;
};

class topol;

/* This class contains a list of all the lines in a level (line_list).
 * You can iterate through all the lines using iterate_all_segments() and next_segment().
 * This class also contains physical 2D map of the level with a list of all the lines the kuski
 * might collide with at any given location.
 * You can iterate through potential collision candidates using
 * iterate_collision_grid_cell_segments() and next_collision_grid_segment().
 */
class segments {
    // All line segments in the level
    segment* seg_list;
    int seg_list_allocated_length;
    int seg_list_length;
    int seg_list_iteration_index;

    // 2D grid representing the level for (kuski <-> line segment) collision purposes
    psegment_node* collision_grid;
    int collision_grid_width, collision_grid_height;
    double collision_grid_cell_size;
    vect2 collision_grid_origin;
    segment_node* collision_grid_iterable;
    // Take one line segment and add it to one or several collision grid cells.
    void add_segment_to_collision_grid(segment* seg, double max_radius);
    void add_node_to_cell(int cell_x, int cell_y, segment* seg);

    // Memory management
    segment_node_array* node_array;
    int node_array_index;
    segment_node* new_node(void);
    void delete_all_nodes(void);

  public:
    // Load a list of all the line segments of a level.
    segments(topol* lev);
    ~segments(void);

    // Initialize the collision_grid with the max radius of any interactable bike part
    void setup_collision_grid(double max_radius);
    // Reset to iterate through all line segments passing through the cell from collision_grid
    // corresponding to position r
    void iterate_collision_grid_cell_segments(vect2 r);
    // Get the next line segment with potential collision from the cell in collision_grid
    segment* next_collision_grid_segment(void);

    // Iterate through all line segments in the level
    void iterate_all_segments(void);
    // Get the next line segment in the level
    segment* next_segment(void);

    segment* getptrszakasz(int sorszam);
    int getsorszam(segment* pvonal);
};

// A single line segment from a polygon
struct segment {
    vect2 r, v, unit_vector;
    double length;
};

extern segments* Segments;

#endif
