#ifndef SZAKASZ_H
#define SZAKASZ_H

#include "vect2.h"

struct line;

struct line_node {
    line* ln;
    line_node* next;
};

typedef line_node* pline_node;

#define LINE_NODE_BLOCK_SIZE (500)
struct line_node_array {
    line_node nodes[LINE_NODE_BLOCK_SIZE];
    line_node_array* next;
};

class topol;

class lines {
    line* line_list;
    int line_list_allocated_length;
    int line_list_length;
    int line_list_current_index;

    // Terbeli kereseshez:
    pline_node* collision_grid;
    int collision_grid_width, collision_grid_height;
    double collision_grid_cell_size;
    vect2 collision_grid_origin;
    line_node* collision_grid_next; // getnext ez alapjan dolgozik!

    void add_line_to_collision_grid(line* ln, double max_radius);
    void add_line_to_cell(int cell_x, int cell_y, line* ln);

    // Terbeli kereseshez nodkezeles:
    line_node_array* node_array;
    int node_array_index;
    line_node* get_new_line_node(void);
    void delete_all_line_nodes(void);

  public:
    lines(topol* lev);
    ~lines(void);

    // Terbeli fv.-ek:
    // rendez-t csak egyszer szabad hivni!
    void setup_collision_grid(double max_radius);
    void select_collision_grid_cell(vect2 r);
    line* get_line_from_collision_grid_cell(void);

    // Osszes szakaszon vegigmennek:
    void iterate_all_lines(void);
    line* get_line(void);

    line* getptrszakasz(int sorszam);
    int getsorszam(line* pvonal);
};

struct line {
    vect2 r, v, unit_vector;
    double length;
};

extern lines* Lines;

#endif
