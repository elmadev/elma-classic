#ifndef LGRFILE_H
#define LGRFILE_H

#include "sprite.h"

class anim;
class grass;
class palette;
class pic8;
class piclist;

enum class MaskEncoding { Transparent, Solid, EndOfLine };

struct mask_element {
    int length; // 0 if EndOfLine
    MaskEncoding type;
};

struct mask {
    char name[10];
    int width;
    int height;
    mask_element* data;
};

struct picture {
    char name[10];
    int default_distance;
    Clipping default_clipping;
    int width;
    int height;
    // Compression format:
    //  {
    //   Varint: transparent length or -1 if end of row,
    //   Varint: non-transparent length, (skipped if end of row)
    //   Raw pixel data, (skipped if end of row)
    //  }
    unsigned char* data;
};

struct texture {
    char name[10];
    pic8* pic; // Horizontally tiled
    int default_distance;
    Clipping default_clipping;
    bool is_qgrass;
    int original_width; // Width before horizontal tiling
};

struct bike_pics {
    pic8* bike_part1;
    pic8* bike_part2;
    pic8* bike_part3;
    pic8* bike_part4;
    pic8* body;
    pic8* thigh;
    pic8* leg;
    pic8* wheel;
    pic8* susp1;
    pic8* susp2;
    pic8* forarm;
    pic8* up_arm;
    pic8* head;
};

constexpr int MAX_PICTURES = 1000;
constexpr int MAX_MASKS = 200;
constexpr int MAX_TEXTURES = 100;
constexpr int MAX_QFOOD = 9;

class lgrfile {
    void chop_bike(pic8* bike, bike_pics* bp);

    void add_picture(pic8* pic, piclist* list, int index);
    void add_texture(pic8* pic, piclist* list, int index);
    void add_mask(pic8* pic, piclist* list, int index);

    static bool try_load_lgr(const char* lgr_name, const char* desc);
    lgrfile(const char* lgrname);
    ~lgrfile();

  public:
    static void load_lgr_file(const char* lgr_name);

    int picture_count;
    picture pictures[MAX_PICTURES];
    int get_picture_index(const char* name);

    int mask_count;
    mask masks[MAX_MASKS];
    int get_mask_index(const char* name);

    int texture_count;
    texture textures[MAX_TEXTURES];
    int get_texture_index(const char* name);

    bool has_grass;
    grass* grass_pics;

    unsigned char* palette_data;
    palette* pal;
    unsigned char* timer_palette_map;

    bike_pics bike1;
    bike_pics bike2;
    pic8* flag;

    anim* killer;
    anim* exit;
    int food_count;
    anim* food[MAX_QFOOD];
    pic8* qframe;

    // Current level's default textures, horizontally tiled:
    pic8* background;
    pic8* foreground;
    int background_original_width;
    int foreground_original_width;
    char foreground_name[10];
    char background_name[10];
    void reload_default_textures(bool force = false);

    // From QCOLORS.pcx
    unsigned char minimap_foreground_palette_id;
    unsigned char minimap_background_palette_id;
    unsigned char minimap_bike1_palette_id;
    unsigned char minimap_bike2_palette_id;
    unsigned char minimap_border_palette_id;
    unsigned char minimap_exit_palette_id;
    unsigned char minimap_food_palette_id;
    unsigned char minimap_killer_palette_id[3];

    // Editor's Create Picture settings
    char editor_picture_name[10];
    char editor_mask_name[10];
    char editor_texture_name[10];
};

extern lgrfile* Lgr;
void invalidate_lgr_cache();

void create_grass_mask(mask& msk, int* heightmap, int skip_rows);

int read_varint(const unsigned char* buffer, int& offset);

struct bike_box {
    int x1;
    int y1;
    int x2;
    int y2;
};

extern bike_box BikeBox1;
extern bike_box BikeBox2;
extern bike_box BikeBox3;
extern bike_box BikeBox4;

#endif
