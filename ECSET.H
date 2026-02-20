#ifndef ECSET_H
#define ECSET_H

#include "lgr.h"
#include "main.h"
#include "sprite.h"
#include "vect2.h"
#include <cstdint>

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

class canvas_pixels {
  public:
    canvas_pixels()
        : value(0) {}

    static constexpr canvas_pixels default_background() { return canvas_pixels(0); }
    static constexpr canvas_pixels default_foreground() { return canvas_pixels(1); }
    static constexpr canvas_pixels transparent() { return canvas_pixels(2); }

    static canvas_pixels texture(int texture_index) {
        canvas_pixels texture(TEXTURE_START + texture_index);
#ifdef DEBUG
        if (!texture.is_texture()) {
            internal_error("canvas_pixels invalid texture constructor");
        }
#endif
        return texture;
    }

    static canvas_pixels pointer(unsigned char* ptr) {
        canvas_pixels pointer(reinterpret_cast<uintptr_t>(ptr));
#ifdef DEBUG
        if (!pointer.is_pointer()) {
            internal_error("canvas_pixels invalid pointer constructor");
        }
#endif
        return pointer;
    }

    bool is_pointer() const { return value >= POINTER_START; }
    bool is_enum() const { return value < ENUM_END; }
    bool is_texture() const { return value >= TEXTURE_START && value < TEXTURE_END; }

    int to_texture() const {
#ifdef DEBUG
        if (!is_texture()) {
            internal_error("to_texture() called on invalid value");
        }
#endif
        return static_cast<int>(value - TEXTURE_START);
    }

    unsigned char* to_pointer() const {
#ifdef DEBUG
        if (!is_pointer()) {
            internal_error("to_pointer() called on invalid value");
        }
#endif
        return reinterpret_cast<unsigned char*>(value);
    }

    constexpr friend bool operator==(canvas_pixels, canvas_pixels) = default;

    canvas_pixels& operator+=(int offset) {
#ifdef DEBUG
        if (!is_pointer()) {
            internal_error("operator+=() called on invalid value");
        }
#endif
        value += static_cast<uintptr_t>(offset);
        return *this;
    }

  private:
    constexpr explicit canvas_pixels(uintptr_t v) noexcept
        : value(v) {}

    static constexpr uintptr_t ENUM_END = 10;
    static constexpr uintptr_t TEXTURE_START = ENUM_END;
    static constexpr uintptr_t TEXTURE_END = TEXTURE_START + MAX_TEXTURES;
    static constexpr uintptr_t POINTER_START = 0x800;
    static_assert(TEXTURE_END < POINTER_START);

    uintptr_t value;
};

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
    canvas_pixels pixels;
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
    canvas_pixels pixels;
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
    canvas_chunk_node* draw_one_chunk(canvas_chunk_node* dest, canvas_pixels source, int dest_x,
                                      int source_x_left, int source_x_right, int source_dist,
                                      canvas_chunk_node* dest_prev, bool* prev_can_merge,
                                      Clipping clipping);
    void draw_pixels(canvas_pixels source, int source_dist, int x_left, int x_right, int y,
                     Clipping clipping);
    void draw_texture(sprite* spr, int texture_index, int mask_index, Clipping clipping);
    void draw_sprites(Clipping clipping);
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
