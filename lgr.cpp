#include "lgr.h"
#include "editor_dialog.h"
#include "EDITUJ.H"
#include "affine_pic.h"
#include "anim.h"
#include "ECSET.H"
#include "eol_settings.h"
#include "fs_utils.h"
#include "grass.h"
#include "KIRAJZOL.H"
#include "level.h"
#include "LOAD.H"
#include "M_PIC.H"
#include "main.h"
#include "menu_pic.h"
#include "pic8.h"
#include "piclist.h"
#include "platform_impl.h"
#include "platform_utils.h"
#include "sprite.h"
#include <algorithm>
#include <cstring>

constexpr int MAGIC_NUMBER = 187565543;

lgrfile* Lgr = nullptr;
static char CurrentLgrName[30] = "";

bike_box BikeBox1 = {3, 36, 147, 184};
bike_box BikeBox2 = {32, 183, 147, 297};
bike_box BikeBox3 = {146, 141, 273, 264};
bike_box BikeBox4 = {272, 181, 353, 244};

void invalidate_lgr_cache() {
    invalidate_ptop();
    CurrentLgrName[0] = '\0';
}

void lgrfile::load_lgr_file(char* lgr_name) {
    if (strlen(lgr_name) > MAX_FILENAME_LEN) {
        internal_error("load_lgr_file strlen( lgr_name ) > MAX_FILENAME_LEN!");
    }
    // This lgr is already loaded, so skip
    if (strcmpi(lgr_name, CurrentLgrName) == 0) {
        return;
    }
    strlwr(lgr_name);

    char path[30];
    sprintf(path, "lgr/%s.lgr", lgr_name);
    if (access(path, 0) != 0) {
        // LGR not found
        if (!Ptop) {
            internal_error("load_lgr_file !Ptop!");
        }

        // Display warning
        char filename[20];
        strcpy(filename, lgr_name);
        strcat(filename, ".lgr");
        blit8(BufferBall, BufferMain);
        BufferMain->fill_box(Hatterindex);
        bltfront(BufferMain);
        if (!InEditor) {
            Pal_editor->set();
        }
        dialog(
            "LGR file not found!",
            "The level file uses the pictures that are stored in this LGR file:", filename,
            "This file doesn't exist in the LGR directory, so the default.lgr file will be loaded.",
            "This level file will look now different from that it was designed to look.");
        if (!InEditor) {
            MenuPalette->set();
        }
        blit8(BufferMain, BufferBall);
        bltfront(BufferMain);

        // Modify our input lgr (i.e. our class level) to default and then try and load it
        strcpy(lgr_name, "default");
        Valtozott = 1;
        if (strcmpi(CurrentLgrName, "default") == 0) {
            return;
        }

        strcpy(path, "lgr/default.lgr");
        Ptop->lgr_not_found = true;
        if (access(path, 0) != 0) {
            external_error("Could not open file lgr/default.lgr!");
        }
    }
    // Actually load the lgr
    strcpy(CurrentLgrName, lgr_name);

    if (Lgr) {
        delete Lgr;
    }
    Lgr = new lgrfile(CurrentLgrName);
    return;
}

static void bike_slice(pic8* bike, affine_pic** ret, bike_box* bbox) {
    pic8* slice = new pic8(bbox->x2 - bbox->x1 + 1, bbox->y2 - bbox->y1 + 1);
    blit8(slice, bike, -bbox->x1, -bbox->y1);
    *ret = new affine_pic(nullptr, slice);
}

// Slice the bike into 4 sub-components
void lgrfile::chop_bike(pic8* bike, bike_pics* bp) {
    bike_slice(bike, &bp->bike_part1, &BikeBox1);
    bike_slice(bike, &bp->bike_part2, &BikeBox2);
    bike_slice(bike, &bp->bike_part3, &BikeBox3);
    bike_slice(bike, &bp->bike_part4, &BikeBox4);

    // Transparency taken from topleft corner of BikeBox1
    bp->bike_part2->transparency = bp->bike_part1->transparency;
    bp->bike_part3->transparency = bp->bike_part1->transparency;
    bp->bike_part4->transparency = bp->bike_part1->transparency;
}

// Tile a texture horizontally to fit SCREEN_WIDTH.
static pic8* generate_default_texture(texture* text) {
    int original_width = text->original_width;
    int tiles = (SCREEN_WIDTH + original_width - 1) / original_width + 1;

    pic8* tiled = new pic8(original_width * tiles, text->pic->get_height());
    for (int i = 0; i < tiles; i++) {
        blit8(tiled, text->pic, i * original_width, 0);
    }
    return tiled;
}

// Get the palette data from q1bike.pcx. h file offset must be at the end of the pcx file!
static unsigned char* create_lgr_palette(FILE* h) {
    unsigned char* pal = new unsigned char[768];

    if (fseek(h, -769, SEEK_CUR) != 0) {
        internal_error("create_lgr_palette cannot seek back 769 bytes!");
    }
    char palette_header;
    if (fread(&palette_header, 1, 1, h) != 1) {
        internal_error("create_lgr_palette failed to read file!");
    }
    if (palette_header != 0x0c) {
        internal_error("create_lgr_palette palette header invalid!");
    }
    if (fread(pal, 1, 768, h) != 768) {
        internal_error("create_lgr_palette failed to read file!");
    }
    return pal;
}

static int get_transparency_palette_id(piclist::Transparency type, pic8* pic) {
    switch (type) {
    case piclist::Transparency::None:
        return -1;
    case piclist::Transparency::Palette0:
        return 0;
    case piclist::Transparency::TopLeft:
        return pic->gpixel(0, 0);
    case piclist::Transparency::TopRight:
        return pic->gpixel(pic->get_width() - 1, 0);
    case piclist::Transparency::BottomLeft:
        return pic->gpixel(0, pic->get_height() - 1);
    case piclist::Transparency::BottomRight:
        return pic->gpixel(pic->get_width() - 1, pic->get_height() - 1);
    }
    internal_error("get_transparency_palette_id unknown type");
    return -1;
}

static int consecutive_transparent_pixels(int x, int pic_width, unsigned char* pic_row,
                                          unsigned char transparency) {
    int count = 0;
    while (x < pic_width && pic_row[x] == transparency) {
        x++;
        count++;
    }
    return count;
}

static int consecutive_solid_pixels(int x, int pic_width, unsigned char* pic_row,
                                    unsigned char transparency) {
    int count = 0;
    while (x < pic_width && pic_row[x] != transparency) {
        x++;
        count++;
    }
    return count;
}

constexpr size_t PICTURE_MAX_MEMORY = 600000;

static unsigned char* PictureBuffer = nullptr;

// Store a picture into the lgr.
// Compression format:
//  {
//   Big-endian unsigned short: transparent length or 0xFFFF if end of row,
//   Big-endian unsigned short: non-transparent length, (skipped if end of row)
//   Raw pixel data, (skipped if end of row)
//  }
void lgrfile::add_picture(pic8* pic, piclist* list, int index) {
    if (picture_count >= MAX_PICTURES) {
        external_error("Too many pictures in lgr file!");
    }

    // Set picture properties
    picture* new_pic = &pictures[picture_count];
    strcpy(new_pic->name, list->name[index]);
    new_pic->default_distance = list->default_distance[index];
    new_pic->default_clipping = list->default_clipping[index];
    new_pic->width = pic->get_width();
    new_pic->height = pic->get_height();

    // Compress picture data
    if (new_pic->width > 60000) {
        external_error("Picture width is too big!", new_pic->name);
    }

    int transparency = get_transparency_palette_id(list->transparency[index], pic);
    if (transparency < 0) {
        external_error("Picture must be transparent in lgr file!", new_pic->name);
    }

    if (!PictureBuffer) {
        PictureBuffer = new unsigned char[PICTURE_MAX_MEMORY + 10];
    }
    if (!PictureBuffer) {
        internal_error("Not enough memory!");
    }

    int buffer_offset = 0;
    for (int i = 0; i < new_pic->height; i++) {
        unsigned char* row = pic->get_row(i);
        if (buffer_offset > PICTURE_MAX_MEMORY) {
            external_error("Picture is too big in lgr file! Picture name:", new_pic->name);
        }

        int x = 0;
        while (true) {
            // Skip pixels
            int skip =
                consecutive_transparent_pixels(x, new_pic->width, row, (unsigned char)transparency);
            if (skip > 60000) {
                internal_error("add_picture skip width too long!");
            }

            PictureBuffer[buffer_offset] = (unsigned char)(skip / 256);
            PictureBuffer[buffer_offset + 1] = (unsigned char)(skip % 256);
            x += skip;
            if (x >= new_pic->width) {
                // End of line
                PictureBuffer[buffer_offset] = 255;
                PictureBuffer[buffer_offset + 1] = 255;
                buffer_offset += 2;
                break;
            }

            buffer_offset += 2;
            // Solid pixels
            int count =
                consecutive_solid_pixels(x, new_pic->width, row, (unsigned char)transparency);
            if (count <= 0) {
                internal_error("add_picture count width negative!");
            }
            if (count > 60000) {
                internal_error("add_picture count width too long!");
            }

            PictureBuffer[buffer_offset] = (unsigned char)(count / 256);
            PictureBuffer[buffer_offset + 1] = (unsigned char)(count % 256);

            buffer_offset += 2;
            if (buffer_offset + count > PICTURE_MAX_MEMORY) {
                external_error("Picture is too big in lgr file! Picture name:", new_pic->name);
            }

            memcpy(&PictureBuffer[buffer_offset], &row[x], count);
            x += count;
            buffer_offset += count;
        }
    }

    new_pic->data = new unsigned char[buffer_offset + 10];
    if (!new_pic->data) {
        internal_error("Not enough memory!");
    }
    memcpy(new_pic->data, PictureBuffer, buffer_offset);

    picture_count++;
}

void lgrfile::add_texture(pic8* pic, piclist* list, int index) {
    if (texture_count >= MAX_TEXTURES) {
        external_error("Too many textures in lgr file!");
    }

    forditkepet(pic);

    texture* new_text = &textures[texture_count];
    if (list) {
        // Copy all the properties
        strcpy(new_text->name, list->name[index]);
        new_text->pic = pic;
        new_text->default_distance = list->default_distance[index];
        new_text->default_clipping = list->default_clipping[index];
        new_text->is_qgrass = false;
    } else {
        // QGRASS special case
        strcpy(new_text->name, "qgrass");
        new_text->pic = pic;
        new_text->default_distance = 450;
        new_text->default_clipping = Clipping::Ground;
        new_text->is_qgrass = true;
    }
    texture_count++;
}

constexpr size_t MASK_MAX_MEMORY = 20000;
static mask_element MaskBuffer[MASK_MAX_MEMORY];

void lgrfile::add_mask(pic8* pic, piclist* list, int index) {
    if (mask_count >= MAX_MASKS) {
        external_error("Too many masks in lgr file!");
    }

    // Copy properties
    mask* new_mask = &masks[mask_count];
    strcpy(new_mask->name, list->name[index]);
    new_mask->width = pic->get_width();
    new_mask->height = pic->get_height();

    // Special compression format type
    int buffer_offset = 0;
    int transparency = get_transparency_palette_id(list->transparency[index], pic);
    if (transparency >= 0) {
        for (int i = 0; i < new_mask->height; i++) {
            // Transparent data
            unsigned char* row = pic->get_row(i);
            int j = consecutive_transparent_pixels(0, new_mask->width, row,
                                                   (unsigned char)transparency);
            if (j > 0) {
                MaskBuffer[buffer_offset].type = MaskEncoding::Transparent;
                MaskBuffer[buffer_offset].length = j;
                buffer_offset++;
            }
            while (j <= new_mask->width - 1) {
                // Solid data
                int count =
                    consecutive_solid_pixels(j, new_mask->width, row, (unsigned char)transparency);
                if (count <= 0) {
                    internal_error("add_mask count length negative!");
                }

                MaskBuffer[buffer_offset].type = MaskEncoding::Solid;
                MaskBuffer[buffer_offset].length = count;
                buffer_offset++;
                if (buffer_offset > MASK_MAX_MEMORY - 10) {
                    external_error("Mask picture is too complicated!:", list->name[index]);
                }

                j += count;

                // Transparent data
                count = consecutive_transparent_pixels(j, new_mask->width, row,
                                                       (unsigned char)transparency);
                if (count > 0) {
                    MaskBuffer[buffer_offset].type = MaskEncoding::Transparent;
                    MaskBuffer[buffer_offset].length = count;
                    buffer_offset++;
                }
                j += count;
            }
            // End of row
            MaskBuffer[buffer_offset].type = MaskEncoding::EndOfLine;
            MaskBuffer[buffer_offset].length = 0;
            buffer_offset++;
        }
    } else {
        // Solid square special case
        for (int i = 0; i < new_mask->height; i++) {
            MaskBuffer[buffer_offset].type = MaskEncoding::Solid;
            MaskBuffer[buffer_offset].length = new_mask->width;
            buffer_offset++;
            MaskBuffer[buffer_offset].type = MaskEncoding::EndOfLine;
            MaskBuffer[buffer_offset].length = 0;
            buffer_offset++;
        }
    }

    new_mask->data = new mask_element[buffer_offset];
    if (!new_mask->data) {
        internal_error("Memory!");
    }
    for (int j = 0; j < buffer_offset; j++) {
        new_mask->data[j] = MaskBuffer[j];
    }

    delete pic;
    mask_count++;
}

// Map the 256 lgr palette colors to either the brightest or darkest color
// Used to draw the timer
static unsigned char* create_timer_palette_map(unsigned char* pal) {
    unsigned char* map = new unsigned char[256];
    if (!map) {
        internal_error("create_timer_palettemap memory!");
    }

    // Find brightest color of the palette
    int brightest_color_sum = -1;
    int brightest_color_index = 0;
    for (int i = 0; i < 256; i++) {
        int new_value = pal[i * 3] + pal[i * 3 + 1] + pal[i * 3 + 2];
        if (brightest_color_sum < new_value) {
            brightest_color_sum = new_value;
            brightest_color_index = i;
        }
    }

    // Find darkest color of the palette
    int darkest_color_sum = 1000;
    int darkest_color_index = 0;
    for (int i = 0; i < 256; i++) {
        int new_value = pal[i * 3] + pal[i * 3 + 1] + pal[i * 3 + 2];
        if (darkest_color_sum > new_value) {
            darkest_color_sum = new_value;
            darkest_color_index = i;
        }
    }

    // Map each color from the palette to the darkest or brightest color
    for (int i = 0; i < 256; i++) {
        // Brightest color only if R < 80, G < 80, B < 80
        if ((pal[i * 3] < 80) && (pal[i * 3 + 1] < 80) && (pal[i * 3 + 2] < 80)) {
            map[i] = (unsigned char)brightest_color_index;
        } else {
            map[i] = (unsigned char)darkest_color_index;
        }
    }
    return map;
}

lgrfile::lgrfile(const char* lgrname) {
    picture_count = 0;
    mask_count = 0;
    texture_count = 0;
    memset(pictures, 0, sizeof(pictures));
    memset(masks, 0, sizeof(masks));
    memset(textures, 0, sizeof(textures));
    pal = nullptr;
    palette_data = nullptr;
    timer_palette_map = nullptr;
    memset(&bike1, 0, sizeof(bike1));
    memset(&bike2, 0, sizeof(bike2));
    flag = nullptr;
    killer = nullptr;
    exit = nullptr;
    qframe = nullptr;
    background = nullptr;
    foreground = nullptr;
    foreground_name[0] = 0;
    background_name[0] = 0;
    food_count = 0;
    memset(food, 0, sizeof(food));
    grass_pics = new grass;

    double zoom = EolSettings->zoom();

    // Load file
    char path[30];
    sprintf(path, "lgr/%s.lgr", lgrname);
    FILE* h = fopen(path, "rb");
    if (!h) {
        external_error("Cannot find file:", path);
    }

#define ERROR_CORRUPT() external_error("Corrupt LGR file!:", path)

    // LGR12
    char version[10];
    if (fread(version, 1, 5, h) != 5) {
        ERROR_CORRUPT();
    }
    version[5] = 0;
    if (strncmp(version, "LGR", 3) != 0) {
        external_error("This is not an LGR file!:", path);
    }
    if (strcmp(version + 3, "12") != 0) {
        external_error("LGR file's version is too new!:", path);
    }

    // Pcx object file count
    int pcx_length;
    if (fread(&pcx_length, 1, 4, h) != 4) {
        ERROR_CORRUPT();
    }
    if (pcx_length < 10 || pcx_length > 3500) {
        ERROR_CORRUPT();
    }

    // Pictures.lst
    piclist* list = new piclist(h);
    if (!list) {
        internal_error("lgrfile::lgrfile out of memory!");
    }

    // Iterate through the pcx objects
    pic8* q1bike = nullptr;
    pic8* q2bike = nullptr;
    pic8* qcolors = nullptr;
    for (int i = 0; i < pcx_length; i++) {
        char asset_filename[30];
        if (fread(asset_filename, 1, 20, h) != 20) {
            ERROR_CORRUPT();
        }
        int asset_size = 0;
        if (fread(&asset_size, 1, 4, h) != 4) {
            ERROR_CORRUPT();
        }
        if (asset_size < 1 || asset_size > 10000000) {
            ERROR_CORRUPT();
        }

        int curpos = ftell(h);
        pic8* asset_pic = new pic8(asset_filename, h);
        fseek(h, curpos + asset_size, SEEK_SET);

        if (strcmpi(asset_filename, "q1bike.pcx") == 0) {
            q1bike = asset_pic;
            palette_data = create_lgr_palette(h);
            timer_palette_map = create_timer_palette_map(palette_data);
            pal = new palette(palette_data);
            // Keep pic8
            continue;
        }
        if (strcmpi(asset_filename, "q2bike.pcx") == 0) {
            q2bike = asset_pic;
            // Keep pic8
            continue;
        }

#define LOAD_AFFINE(name, destination)                                                             \
    if (strcmpi(asset_filename, name) == 0) {                                                      \
        destination = new affine_pic(nullptr, asset_pic);                                          \
        continue;                                                                                  \
    }

        LOAD_AFFINE("q1body.pcx", bike1.body);
        LOAD_AFFINE("q1thigh.pcx", bike1.thigh);
        LOAD_AFFINE("q1leg.pcx", bike1.leg);
        LOAD_AFFINE("q1wheel.pcx", bike1.wheel);
        LOAD_AFFINE("q1susp1.pcx", bike1.susp1);
        LOAD_AFFINE("q1susp2.pcx", bike1.susp2);
        LOAD_AFFINE("q1forarm.pcx", bike1.forarm);
        LOAD_AFFINE("q1up_arm.pcx", bike1.up_arm);
        LOAD_AFFINE("q1head.pcx", bike1.head);

        LOAD_AFFINE("q2body.pcx", bike2.body);
        LOAD_AFFINE("q2thigh.pcx", bike2.thigh);
        LOAD_AFFINE("q2leg.pcx", bike2.leg);
        LOAD_AFFINE("q2wheel.pcx", bike2.wheel);
        LOAD_AFFINE("q2susp1.pcx", bike2.susp1);
        LOAD_AFFINE("q2susp2.pcx", bike2.susp2);
        LOAD_AFFINE("q2forarm.pcx", bike2.forarm);
        LOAD_AFFINE("q2up_arm.pcx", bike2.up_arm);
        LOAD_AFFINE("q2head.pcx", bike2.head);

        LOAD_AFFINE("qflag.pcx", flag);
#undef LOAD_AFFINE

        if (strcmpi(asset_filename, "qkiller.pcx") == 0) {
            killer = new anim(asset_pic, "qkiller.pcx", zoom);
            delete asset_pic;
            asset_pic = nullptr;
            continue;
        }
        if (strcmpi(asset_filename, "qexit.pcx") == 0) {
            exit = new anim(asset_pic, "qexit.pcx", zoom);
            delete asset_pic;
            asset_pic = nullptr;
            continue;
        }

        if (strcmpi(asset_filename, "qframe.pcx") == 0) {
            qframe = asset_pic;
            // Keep pic8
            continue;
        }
        if (strcmpi(asset_filename, "qcolors.pcx") == 0) {
            qcolors = asset_pic;
            // Keep pic8
            continue;
        }

        bool is_food = false;
        for (int foodi = 0; foodi < MAX_QFOOD; foodi++) {
            char qfood_name[20];
            sprintf(qfood_name, "qfood%d.pcx", foodi + 1);
            if (strcmpi(asset_filename, qfood_name) == 0) {
                food[foodi] = new anim(asset_pic, qfood_name, zoom);
                delete asset_pic;
                asset_pic = nullptr;
                is_food = true;
            }
        }
        if (is_food) {
            continue;
        }

        // QUP/QDOWN
        if (strnicmp(asset_filename, "qup_", 4) == 0) {
            grass_pics->add(asset_pic, true);
            continue;
        }
        if (strnicmp(asset_filename, "qdown_", 6) == 0) {
            grass_pics->add(asset_pic, false);
            continue;
        }

        // Truncate file extension
        if (!strchr(asset_filename, '.')) {
            external_error("Cannot find dot in name:", asset_filename);
        }
        *strchr(asset_filename, '.') = 0;
        if (strlen(asset_filename) > MAX_FILENAME_LEN) {
            external_error("Filename is too long in LGR file!:", asset_filename, path);
        }

        // QGRASS
        if (strcmpi(asset_filename, "qgrass") == 0) {
            add_texture(asset_pic, nullptr, 0);
            continue;
        }

        // Generic asset
        int index = list->get_index(asset_filename);
        if (index < 0) {
            external_error("There is no line in PICTURES.LST corresponding to picture:",
                           asset_filename);
        }
        if (list->type[index] == piclist::Type::Picture) {
            asset_pic = pic8::scale(asset_pic, zoom);
            add_picture(asset_pic, list, index);
            delete asset_pic;
            asset_pic = nullptr;
            continue;
        }
        if (list->type[index] == piclist::Type::Texture) {
            if (EolSettings->zoom_textures()) {
                asset_pic = pic8::scale(asset_pic, zoom);
            }
            add_texture(asset_pic, list, index);
            // Keep pic8
            continue;
        }
        if (list->type[index] == piclist::Type::Mask) {
            asset_pic = pic8::scale(asset_pic, zoom);
            add_mask(asset_pic, list, index);
            // pic8 deleted by above function
            asset_pic = nullptr;
            continue;
        }
        ERROR_CORRUPT();
    }

    // EOF
    int magic_number = 0;
    if (fread(&magic_number, 1, 4, h) != 4) {
        ERROR_CORRUPT();
    }
    if (magic_number != MAGIC_NUMBER) {
        ERROR_CORRUPT();
    }

#undef ERROR_CORRUPT

    fclose(h);
    h = nullptr;

    // Check that the LGR is complete
    if (texture_count < 2) {
        external_error("There must be at least two textures in LGR file!", lgrname);
    }

#define ASSERT_EXISTS(var, name)                                                                   \
    if (!var) {                                                                                    \
        external_error("Picture not found in LGR file!: ", name, path);                            \
    }

    ASSERT_EXISTS(bike1.body, "q1body.pcx");
    ASSERT_EXISTS(bike1.thigh, "q1thigh.pcx");
    ASSERT_EXISTS(bike1.leg, "q1leg.pcx");
    ASSERT_EXISTS(q1bike, "q1bike.pcx");
    ASSERT_EXISTS(bike1.wheel, "q1wheel.pcx");
    ASSERT_EXISTS(bike1.susp1, "q1susp1.pcx");
    ASSERT_EXISTS(bike1.susp2, "q1susp2.pcx");
    ASSERT_EXISTS(bike1.forarm, "q1forarm.pcx");
    ASSERT_EXISTS(bike1.up_arm, "q1up_arm.pcx");
    ASSERT_EXISTS(bike1.head, "q1head.pcx");

    ASSERT_EXISTS(bike2.body, "q2body.pcx");
    ASSERT_EXISTS(bike2.thigh, "q2thigh.pcx");
    ASSERT_EXISTS(bike2.leg, "q2leg.pcx");
    ASSERT_EXISTS(q2bike, "q2bike.pcx");
    ASSERT_EXISTS(bike2.wheel, "q2wheel.pcx");
    ASSERT_EXISTS(bike2.susp1, "q2susp1.pcx");
    ASSERT_EXISTS(bike2.susp2, "q2susp2.pcx");
    ASSERT_EXISTS(bike2.forarm, "q2forarm.pcx");
    ASSERT_EXISTS(bike2.up_arm, "q2up_arm.pcx");
    ASSERT_EXISTS(bike2.head, "q2head.pcx");

    ASSERT_EXISTS(flag, "qflag.pcx");

    ASSERT_EXISTS(killer, "qkiller.pcx");
    ASSERT_EXISTS(exit, "qexit.pcx");

    ASSERT_EXISTS(qframe, "qframe.pcx");
    ASSERT_EXISTS(qcolors, "qcolors.pcx");
#undef ASSERT_EXISTS

    // Create the bike affine_pic
    chop_bike(q1bike, &bike1);
    delete q1bike;
    q1bike = nullptr;
    chop_bike(q2bike, &bike2);
    delete q2bike;
    q2bike = nullptr;

    // Parse QCOLORS
    minimap_foreground_palette_id = qcolors->gpixel(6, 6 + 0 * 12);
    minimap_background_palette_id = qcolors->gpixel(6, 6 + 1 * 12);
    minimap_border_palette_id = qcolors->gpixel(6, 6 + 2 * 12);
    minimap_bike1_palette_id = qcolors->gpixel(6, 6 + 4 * 12);
    minimap_bike2_palette_id = qcolors->gpixel(6, 6 + 5 * 12);
    minimap_exit_palette_id = qcolors->gpixel(6, 6 + 6 * 12);
    minimap_food_palette_id = qcolors->gpixel(6, 6 + 7 * 12);
    minimap_killer_palette_id[0] = qcolors->gpixel(6, 6 + 8 * 12);
    minimap_killer_palette_id[2] = minimap_killer_palette_id[1] = minimap_killer_palette_id[0];
    delete qcolors;
    qcolors = nullptr;

    // Horizontally tile textures to a minimum width for faster rendering
    for (int i = 0; i < texture_count; i++) {
        texture* text = &textures[i];
        if (!text->pic) {
            internal_error("lgrfile::lgrfile texture missing pic!");
        }
        text->original_width = text->pic->get_width();
        constexpr int TEXTURE_MIN_WIDTH = 600;
        if (text->pic->get_width() >= TEXTURE_MIN_WIDTH) {
            continue;
        }
        int tiles = (TEXTURE_MIN_WIDTH + text->pic->get_width() - 1) / text->pic->get_width();
        pic8* tiled = new pic8(tiles * text->pic->get_width(), text->pic->get_height());
        for (int j = 0; j < tiles; j++) {
            blit8(tiled, text->pic, j * text->pic->get_width(), 0);
        }
        delete text->pic;
        text->pic = tiled;
    }

    // Sort pictures, masks and textures alphabetically (qgrass at the end)
    std::sort(pictures, pictures + picture_count,
              [](const picture& a, const picture& b) { return strcmpi(a.name, b.name) < 0; });
    std::sort(masks, masks + mask_count,
              [](const mask& a, const mask& b) { return strcmpi(a.name, b.name) < 0; });
    std::sort(textures, textures + texture_count, [](const texture& a, const texture& b) {
        // QGRASS should always be last
        if (strcmpi(a.name, "qgrass") == 0) {
            return false;
        }
        if (strcmpi(b.name, "qgrass") == 0) {
            return true;
        }
        return strcmpi(a.name, b.name) < 0;
    });

    // Check for duplicate names in pictures, masks and textures
    for (int i = 0; i < picture_count - 1; i++) {
        if (strcmpi(pictures[i].name, pictures[i + 1].name) == 0) {
            external_error("Picture name is duplicated in LGR file!: ", pictures[i].name);
        }
    }

    for (int i = 0; i < mask_count - 1; i++) {
        if (strcmpi(masks[i].name, masks[i + 1].name) == 0) {
            external_error("Mask name is duplicated in LGR file!: ", masks[i].name);
        }
    }

    for (int i = 0; i < texture_count - 1; i++) {
        if (strcmpi(textures[i].name, textures[i + 1].name) == 0) {
            external_error("Texture name is duplicated in LGR file!: ", textures[i].name);
        }
    }

    // Cleanup
    delete list;
    list = nullptr;

    if (PictureBuffer) {
        delete PictureBuffer;
        PictureBuffer = nullptr;
    }

    // Editor picture selection initialization
    editor_picture_name[0] = 0;
    editor_mask_name[0] = 0;
    editor_texture_name[0] = 0;

    food_count = 0;
    while (food_count < MAX_QFOOD && food[food_count]) {
        food_count++;
    }
    if (food_count < 1) {
        external_error("Picture is missing from LGR file:", "qfood1.pcx", path);
    }

    // Check grass
    has_grass = get_texture_index("qgrass") >= 0 && grass_pics->length >= 2;
}

static void delete_bike_pics(bike_pics* bp) {
    if (!bp->body || !bp->thigh || !bp->leg || !bp->bike_part1 || !bp->bike_part2 ||
        !bp->bike_part3 || !bp->bike_part4 || !bp->wheel || !bp->susp1 || !bp->susp2 ||
        !bp->forarm || !bp->up_arm || !bp->head) {
        internal_error("delete_bike_pics missing pic!");
    }

    delete bp->body;
    delete bp->thigh;
    delete bp->leg;
    delete bp->bike_part1;
    delete bp->bike_part2;
    delete bp->bike_part3;
    delete bp->bike_part4;
    delete bp->wheel;
    delete bp->susp1;
    delete bp->susp2;
    delete bp->forarm;
    delete bp->up_arm;
    delete bp->head;
}

lgrfile::~lgrfile() {
    for (int i = 0; i < picture_count; i++) {
        if (!pictures[i].data) {
            internal_error("lgrfile::~lgrfile !pictures[i].data");
        }
        delete pictures[i].data;
        pictures[i].data = nullptr;
    }
    for (int i = 0; i < mask_count; i++) {
        if (!masks[i].data) {
            internal_error("lgrfile::~lgrfile !masks[i].data");
        }
        delete masks[i].data;
        masks[i].data = nullptr;
    }
    for (int i = 0; i < texture_count; i++) {
        if (!textures[i].pic) {
            internal_error("lgrfile::~lgrfile !textures[i].pic");
        }
        delete textures[i].pic;
        textures[i].pic = nullptr;
    }

    if (!grass_pics) {
        internal_error("lgrfile::~lgrfile !grasses");
    }
    delete grass_pics;
    grass_pics = nullptr;

    picture_count = 0;
    mask_count = 0;
    texture_count = 0;

    if (!pal || !palette_data || !timer_palette_map) {
        internal_error("lgrfile::~lgrfile !palette || !pal_data || !timer_palette_map!");
    }
    delete pal;
    delete palette_data;
    delete timer_palette_map;
    pal = nullptr;
    palette_data = nullptr;
    timer_palette_map = nullptr;

    delete_bike_pics(&bike1);
    delete_bike_pics(&bike2);

    if (!flag || !qframe) {
        internal_error("lgrfile::~lgrfile !flag || !frame!");
    }
    delete flag;
    delete qframe;
    flag = nullptr;
    qframe = nullptr;

    if (!killer || !exit) {
        internal_error("lgrfile::~lgrfile !killer || !exit");
    }

    delete killer;
    killer = nullptr;
    delete exit;
    exit = nullptr;

    food_count = 0;
    for (int i = 0; i < MAX_QFOOD; i++) {
        if (food[i]) {
            delete food[i];
            food[i] = nullptr;
        }
    }

    if (background) {
        delete background;
        background = nullptr;
    }
    if (foreground) {
        delete foreground;
        foreground = nullptr;
    }
}

void lgrfile::reload_default_textures() {
    if (!Ptop->foreground_name[0] || !Ptop->background_name[0]) {
        internal_error("!Ptop->foreground_name[0] || !Ptop->background_name[0]");
    }

    // Recreate background texture
    if (strcmpi(background_name, Ptop->background_name) != 0) {
        strcpy(background_name, Ptop->background_name);
        if (background) {
            delete background;
        }
        background = nullptr;

        int index = get_texture_index(background_name);
        if (index < 0) {
            internal_error("reload_default_textures index not found!");
        }
        texture* text = &textures[index];
        background_original_width = text->original_width;
        background = generate_default_texture(text);
    }

    // Recreate foreground texture
    if (strcmpi(foreground_name, Ptop->foreground_name) != 0) {
        strcpy(foreground_name, Ptop->foreground_name);
        if (foreground) {
            delete foreground;
        }
        foreground = nullptr;

        int index = get_texture_index(foreground_name);
        if (index < 0) {
            internal_error("reload_default_textures index not found!");
        }
        texture* text = &textures[index];
        foreground_original_width = text->original_width;
        foreground = generate_default_texture(text);
    }
}

int lgrfile::get_picture_index(const char* name) {
    if (!name[0]) {
        return -1;
    }
    for (int i = 0; i < picture_count; i++) {
        if (strcmpi(pictures[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int lgrfile::get_mask_index(const char* name) {
    if (!name[0]) {
        return -1;
    }
    for (int i = 0; i < mask_count; i++) {
        if (strcmpi(masks[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int lgrfile::get_texture_index(const char* name) {
    if (!name[0]) {
        return -1;
    }
    for (int i = 0; i < texture_count; i++) {
        if (strcmpi(textures[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}
