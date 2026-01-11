#ifndef SZAKASZ_H
#define SZAKASZ_H

#include "vect2.h"

struct segment;

struct segment_node {
    segment* seg;
    segment_node* next;
};

typedef segment_node* psegment_node;

#define SEGMENT_NODE_BLOCK_LENGTH (500)
struct segment_node_array {
    segment_node nodes[SEGMENT_NODE_BLOCK_LENGTH];
    segment_node_array* next;
};

class topol;

class segments {
    segment* seg_list;
    int seg_list_allocated_length;
    int seg_list_length;
    int seg_list_iteration_index;

    // Terbeli kereseshez:
    psegment_node* collision_grid;
    int collision_grid_width, collision_grid_height;
    double collision_grid_cell_size;
    vect2 collision_grid_origin;
    segment_node* collision_grid_iterable; // next_collision_grid_segment ez alapjan dolgozik!

    void add_segment_to_collision_grid(segment* seg, double max_radius);
    void add_node_to_cell(int cell_x, int cell_y, segment* seg);

    // Terbeli kereseshez nodkezeles:
    segment_node_array* node_array;
    int node_array_index;
    segment_node* new_node(void);
    void delete_all_nodes(void);

  public:
    segments(topol* lev);
    ~segments(void);

    // Terbeli fv.-ek:
    // rendez-t csak egyszer szabad hivni!
    void setup_collision_grid(double max_radius);
    void iterate_collision_grid_cell_segments(vect2 r);
    segment* next_collision_grid_segment(void);

    // Osszes szakaszon vegigmennek:
    void iterate_all_segments(void);
    segment* next_segment(void);

    segment* getptrszakasz(int sorszam);
    int getsorszam(segment* pvonal);
};

struct segment {
    vect2 r, v, unit_vector;
    double length;
};

extern segments* Segments;

#endif
