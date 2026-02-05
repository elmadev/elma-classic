#ifndef ECSET_H
#define ECSET_H

#include "vect2.h"

class canvas;
class grass;
struct canvas_chunk_node;
class pic8;
class sprite;
struct segment;

// Egy iterator class (inkabb cache) ecsethez (foltoz hasznalja oket):
class node_finder {
    canvas* source;
    int current_x, current_y;
    canvas_chunk_node* current_node;

  public:
    node_finder(canvas* src);
    canvas_chunk_node* get_chunk(int x, int y);
};

// Elso par strukturat be lehetne majd vinni cpp-be:

#define PixelsDefaultForeground (1)
#define PixelsDefaultBackground (0)
#define PixelsTransparent (2)

struct canvas_chunk_node {
    canvas_chunk_node* next;
    int width;
    int distance;
    unsigned char* pixels;
};

#define CHUNK_NODE_BLOCK_LENGTH (10000)

struct canvas_chunk_node_array {
    canvas_chunk_node nodes[CHUNK_NODE_BLOCK_LENGTH];
    canvas_chunk_node_array* next;
};

#define CANVAS_MAX_HEIGHT (12000)

struct canvas_chunk {
    int width;
    unsigned char* pixels;
};

void create_canvases(void);

class canvas {
    friend void create_canvases(void);
    friend void segedfv(void);
    friend canvas_chunk_node* node_finder::get_chunk(int x, int y);

    // madarab allokalas:
    canvas_chunk_node_array* node_array;
    canvas_chunk_node_array* node_array_last;
    int node_array_index;
    canvas_chunk_node* new_node(void);
    void delete_all_nodes(void);

    canvas_chunk* chunk_array;

    int is_minimap;
    vect2 origin;
    double width, height;
    int pixel_width, pixel_height;
    canvas_chunk_node* rows_linked[CANVAS_MAX_HEIGHT];
    canvas_chunk* rows[CANVAS_MAX_HEIGHT];
    // Kulon van ket jatekosnak:
    int rows_x1[CANVAS_MAX_HEIGHT];                  // Eppen hol all mutatohoz tartozo x
    canvas_chunk* rows_position1[CANVAS_MAX_HEIGHT]; // Melyik darab kurrens
    int rows_x2[CANVAS_MAX_HEIGHT];                  // Eppen hol all mutatohoz tartozo x
    canvas_chunk* rows_position2[CANVAS_MAX_HEIGHT]; // Melyik darab kurrens

    void set_origin_and_dimensions(void);
    void draw_segment(segment* seg);
    // Rovid folddarabokat atalakitja konkret mutatova:
    void default_foreground_to_pointers(void);
    // Egymas utani azonos maszkokat es ureseket lecsereli egy hosszabbra:
    void merge_redundant_chunks(void);
    void textures_to_pointers(void);
    void linked_list_to_array(void);

    // Ptop kerek-jeinek egesz koordinatait kitolti:
    void calculate_object_positions(void);

    void adjust_background_distance(void);
    canvas_chunk_node* draw_one_chunk(canvas_chunk_node* dest, unsigned char* source, int dest_x,
                                      int source_x_left, int source_x_right, int source_dist,
                                      canvas_chunk_node* dest_prev, int* prev_can_merge,
                                      int clipping);
    void draw_pixels(unsigned char* source, int source_dist, int x_left, int x_right, int y,
                     int clipping);
    void draw_texture(sprite* spr, int texture_index, int mask_index, int clipping);
    void draw_sprites(int clipping);
    void draw_killers(void); // Ez most nem view eseten visszter

    void draw_qgrass_texture(grass* gr, int qupdown_index, int x, int y, int clipping);
    void draw_qupdown(grass* gr, int qupdown_index, int x, int y, int clipping);
    void draw_grass_polygon(grass* gr, int* heightmap, int heightmap_length, int x0, int clipping);
    void draw_grass_polygons(int clipping);

    void render_row1(unsigned char* dest, int y);
    void render_row2(unsigned char* dest, int y);
    void kiegysor_FF(unsigned char* pc, int ye); // fekete-feher

    void create_front_grass(void);

    canvas(int minimap);       // Ptop es Segments alapjan
    canvas(canvas* reference); // Ures sortomboket hoz benne letre
    ~canvas(void);

  public:
    void meters_to_pixels(vect2 meters, int* pixel_x, int* pixel_y);
    void render(int player1, pic8* pic, vect2 corner, int x1, int y1, int x2, int y2);
    void render_minimap(int player1, pic8* pic, vect2 corner, int x1, int y1, int x2, int y2);

    // void egetkeszit( void );
};

extern canvas* CanvasBack;
extern canvas* CanvasFront;
extern canvas* CanvasMinimap;

// create_canvases ide osszegzi osszes lefoglalt szakaszt:
extern int Osszegszam;

#endif
