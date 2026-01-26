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
    unsigned char* data; // leiras kovetkezik:
                         // soronkent ismetlodve: {uresek szama telik szama, adat byte-ok}
                         // uresek szama 60000 felett eseten uj sor
                         // tehat minden sor uressel kezdodik
                         // szamok 2 byte-on tarolodnak elso-t meg kell szorozni 256-tal
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

#define MAX_QFOOD (20) // Valojaban ennek csak 9-nak kene lennie

class lgrfile {
    // Konstruktor hasznalja:
    void chop_bike(pic8* bike, bike_pics* bp);

    void add_picture(pic8* pic, piclist* list, int index);
    void add_texture(pic8* pic, piclist* list, int index);
    void add_mask(pic8* pic, piclist* list, int index);

    lgrfile(const char* lgrname); // kiterj es ut nelkul
    ~lgrfile(void);

  public:
    friend void load_lgr_file(const char* lgrname); // Leiras alul

    int picture_count;
    picture pictures[MAX_PICTURES];
    int get_picture_index(const char* name); // vagy -1

    int mask_count;
    mask masks[MAX_MASKS];
    int get_mask_index(const char* name); // vagy -1

    int texture_count;
    texture textures[MAX_TEXTURES];
    int get_texture_index(const char* name); // vagy -1

    int has_grass;
    // Textura szelekcio lista utolso eleme kimarad (qgrass):
    int editor_hide_qgrass;
    grass* grass_pics;

    unsigned char* palette_data;
    palette* pal;
    // paltomb alapjan ido kiirashoz negalt 256 byte-os lookup tabla:
    unsigned char* timer_palette_map;

    bike_pics bike1;
    bike_pics bike2;
    affine_pic* flag;

    anim *killer, *exit; // Nagy objektumok
    int food_count;
    // Ennek indexe 0-tol indul, de ez 1-nek felel meg filenev es
    // set properties-ben:
    anim* food[MAX_QFOOD];
    pic8* qframe; // Csokkentett meret eseten ez latszik

    // Hater es eloter kepeket mindig kulon vesszuk:
    pic8 *background, *foreground; // Ezek mar kelloen hosszuak vizszintesen
    int background_original_width, foreground_original_width;
    char foreground_name[10], background_name[10];
    // Ezt betoltecseteket hivja (Ptop-bol szedi kep neveket):
    void reload_default_textures();

    // Ezeket colors.pcx file-bol veszi:
    unsigned char minimap_foreground_palette_id;
    unsigned char minimap_background_palette_id;
    unsigned char minimap_bike1_palette_id;
    unsigned char minimap_bike2_palette_id;
    unsigned char minimap_border_palette_id;
    unsigned char minimap_exit_palette_id[3];
    unsigned char minimap_food_palette_id;
    unsigned char minimap_killer_palette_id[3];

    // tolt_pickasprite tudja allitani oket egyedul:
    char editor_picture_name[10];
    char editor_mask_name[10];
    char editor_texture_name[10];
};

extern lgrfile* Lgr; // Az eppen bentlevo lgrfile-ra mutat
// Ha nincs ilyen nevu file, akkor default-ot olvassa:
void load_lgr_file(const char* lgrname);
void invalidate_lgr_cache();

struct bike_box {
    int x1, y1, x2, y2;
};

extern bike_box BikeBox1, BikeBox2, BikeBox3, BikeBox4;

#endif
