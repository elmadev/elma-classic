#ifndef ECSET_H
#define ECSET_H

#include "vect2.h"

class canvas;
class grass;
struct canvas_chunk_node;
class pic8;
class sprite;
struct segment;

// Helper class to efficiently find a chunk corresponding to a specific
// coordinate when iterating towards the right.
class node_finder {
    canvas* source;
    int current_x;
    int current_y;
    canvas_chunk_node* current_node;

  public:
    node_finder(canvas* src);
    canvas_chunk_node* get_chunk(int x, int y);
};

#define PixelsDefaultBackground (0)
#define PixelsDefaultForeground (1)
#define PixelsTransparent (2)

// Horizontal segment of pixels sharing the same source of pixel data, chained
// together to make one row of the rendered level.
struct canvas_chunk_node {
    canvas_chunk_node* next;
    union {
        // Initially, when drawing from segments class.
        int xpos;
        // Later converted to width.
        int width;
    };
    int distance;
    unsigned char* pixels;
};

constexpr int CHUNK_NODE_BLOCK_LENGTH = 10000;

// Memory structure to allocate memory for canvas_chunk_node
struct canvas_chunk_node_array {
    canvas_chunk_node nodes[CHUNK_NODE_BLOCK_LENGTH];
    canvas_chunk_node_array* next;
};

constexpr int CANVAS_MAX_HEIGHT = 12000;

// A stripped-down version of canvas_chunk_node in array form.
struct canvas_chunk {
    int width;
    unsigned char* pixels;
};

// Contains an image of the static level front, back or minimap
class canvas {
    friend canvas_chunk_node* node_finder::get_chunk(int x, int y);

    // Memory structure for canvas_chunk_node
    canvas_chunk_node_array* node_array;
    canvas_chunk_node_array* node_array_last;
    // Index of next unused node from node_array_last
    int node_array_index;
    // Get a new unused canvas_chunk_node
    canvas_chunk_node* new_node();
    void delete_all_nodes();
    // Memory structure for canvas_chunk
    canvas_chunk* chunk_array;

    // Main class variables
    bool is_minimap;
    // Top-left corner
    vect2 origin;
    // In meters
    double width;
    double height;
    // In pixels
    int pixel_width;
    int pixel_height;
    // Temporary, (deleted during init())
    canvas_chunk_node* rows_linked[CANVAS_MAX_HEIGHT];
    // Pointer to start of each row in canvas
    canvas_chunk* rows[CANVAS_MAX_HEIGHT];
    // Position of current offset (player 1)
    int rows_x1[CANVAS_MAX_HEIGHT];
    // Pointer to current offset (player 1)
    canvas_chunk* rows_position1[CANVAS_MAX_HEIGHT];
    // Position of current offset (player 2)
    int rows_x2[CANVAS_MAX_HEIGHT];
    // Pointer to current offset (player 2)
    canvas_chunk* rows_position2[CANVAS_MAX_HEIGHT];

    void set_origin_and_dimensions();
    void draw_segment(segment* seg);

    // Process data
    void default_foreground_to_pointers();
    void merge_redundant_chunks();
    void textures_to_pointers();
    void linked_list_to_array();
    void calculate_object_positions();
    void adjust_background_distance();

    // Drawing
    canvas_chunk_node* draw_one_chunk(canvas_chunk_node* dest, unsigned char* source, int dest_x,
                                      int source_x_left, int source_x_right, int source_dist,
                                      canvas_chunk_node* dest_prev, bool* prev_can_merge,
                                      int clipping);
    void draw_pixels(unsigned char* source, int source_dist, int x_left, int x_right, int y,
                     int clipping);
    void draw_texture(sprite* spr, int texture_index, int mask_index, int clipping);
    void draw_sprites(int clipping);
    void draw_killers();

    // Draw grass
    void draw_qgrass_texture(grass* gr, int qupdown_index, int x, int y);
    void draw_qupdown(grass* gr, int qupdown_index, int x, int y);
    void draw_grass_polygon(grass* gr, int* heightmap, int heightmap_length, int x0);
    void draw_grass_polygons();

    // Render
    void render_row(bool player1, unsigned char* dest, int y);

    void create_front_grass();

    // Create blank canvas from Segments
    canvas(bool minimap);
    // Copy the size of an existing canvas
    canvas(canvas* reference);
    ~canvas();

  public:
    void meters_to_pixels(vect2 meters, int* pixel_x, int* pixel_y);
    void render(bool player1, pic8* pic, vect2 corner, int x1, int y1, int x2, int y2);
    void render_minimap(bool player1, pic8* pic, vect2 corner, int x1, int y1, int x2, int y2);
    // Generate all 3 canvasses required to render a level
    static void create_canvases();
};

extern canvas* CanvasBack;
extern canvas* CanvasFront;
extern canvas* CanvasMinimap;

#endif
