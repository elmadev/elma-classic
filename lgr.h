#ifndef LGRFILE_H
#define LGRFILE_H

#include "sprite.h"

class anim;
class grass;
class affine_pic;
class palette;
class pic8;
class piclist;

enum { MaskEncoding_Transparent, MaskEncoding_Solid, MaskEncoding_EndOfLine };

struct mask_element {
    int length; // 0 if EndOfLine
    int type;
};

#define MAX_MASKS (200)

struct mask {
    char name[10];
    int width, height;
    mask_element* data;
};

#define MAX_PICTURES (1000)

struct picture {
    char name[10];
    int default_distance;
    Clipping default_clipping;

    int width, height;
    // Encoded as array of {unsigned BE short skip, unsigned BE short length, char[length] pixels}
    // Jump to next row when skip length is 2^16 (and skip length/pixels)
    unsigned char* data;
};

#define MAX_TEXTURES (100)

struct texture {
    char name[10];
    pic8* pic; // Horizontally tiled
    int default_distance;
    Clipping default_clipping;
    int is_qgrass;
    int original_width; // Width before horizontal tiling
};

struct bike_pics {
    affine_pic *bike_part1, *bike_part2, *bike_part3, *bike_part4;
    affine_pic* body;
    affine_pic* thigh;
    affine_pic* leg;
    affine_pic* wheel;
    affine_pic* susp1;
    affine_pic* susp2;
    affine_pic* forarm;
    affine_pic* up_arm;
    affine_pic* head;
};

#define MAX_QFOOD (20)

class lgrfile {
    void chop_bike(pic8* bike, bike_pics* bp);

    void add_picture(pic8* pic, piclist* list, int index);
    void add_texture(pic8* pic, piclist* list, int index);
    void add_mask(pic8* pic, piclist* list, int index);

    lgrfile(const char* lgrname);
    ~lgrfile(void);

  public:
    friend void load_lgr_file(const char* lgrname);

    int picture_count;
    picture pictures[MAX_PICTURES];
    int get_picture_index(const char* name);

    int mask_count;
    mask masks[MAX_MASKS];
    int get_mask_index(const char* name);

    int texture_count;
    texture textures[MAX_TEXTURES];
    int get_texture_index(const char* name);

    int has_grass;
    int editor_hide_qgrass;
    grass* grass_pics;

    unsigned char* palette_data;
    palette* pal;
    unsigned char* timer_palette_map;

    bike_pics bike1;
    bike_pics bike2;
    affine_pic* flag;

    anim *killer, *exit;
    int food_count;
    anim* food[MAX_QFOOD];
    pic8* qframe;

    // Current level's default textures, horizontally tiled:
    pic8 *background, *foreground;
    int background_original_width, foreground_original_width;
    char foreground_name[10], background_name[10];
    void reload_default_textures();

    // From QCOLORS.pcx
    unsigned char minimap_foreground_palette_id;
    unsigned char minimap_background_palette_id;
    unsigned char minimap_bike1_palette_id;
    unsigned char minimap_bike2_palette_id;
    unsigned char minimap_border_palette_id;
    unsigned char minimap_exit_palette_id[3];
    unsigned char minimap_food_palette_id;
    unsigned char minimap_killer_palette_id[3];

    // Editor's Create Picture settings
    char editor_picture_name[10];
    char editor_mask_name[10];
    char editor_texture_name[10];
};

extern lgrfile* Lgr;
void load_lgr_file(const char* lgrname);
void invalidate_lgr_cache();

struct bike_box {
    int x1, y1, x2, y2;
};

extern bike_box BikeBox1, BikeBox2, BikeBox3, BikeBox4;

#endif
