#include "pic/lgr.h"
#include "debug/profiler.h"
#include "editor/dialog.h"
#include "editor/editor.h"
#include "eol/settings.h"
#include "level/level.h"
#include "level/sprite.h"
#include "main.h"
#include "menu/pic.h"
#include "pic/anim.h"
#include "pic/pic8.h"
#include "pic/piclist.h"
#include "pic/surface.h"
#include "platform/implementation.h"
#include "platform/utils.h"
#include "renderer/canvas.h"
#include "renderer/grass.h"
#include "renderer/render.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <vector>

constexpr int MAGIC_NUMBER = 187565543;

lgrfile* Lgr = nullptr;
static char CurrentLgrName[30] = "";

bike_box BikeBox1 = {3, 36, 147, 184};
bike_box BikeBox2 = {32, 183, 147, 297};
bike_box BikeBox3 = {146, 141, 273, 264};
bike_box BikeBox4 = {272, 181, 353, 244};

constexpr char FANCYBOOST_NAMES[256][10] = {
    "zz000000", "zz783000", "zz200000", "zzB4C4AC", "zz9C0000", "zzA40000", "zz606038", "zz000808",
    "zz080008", "zz380000", "zz686068", "zz606868", "zz9C1808", "zz500848", "zzF47000", "zz500000",
    "zz9C2808", "zzB40000", "zz9C2800", "zz780000", "zz884010", "zz9C1008", "zz280800", "zz008008",
    "zz008000", "zzF4E410", "zz800000", "zz383030", "zz202030", "zz006000", "zzBCCCBC", "zz000030",
    "zz882800", "zz401808", "zzB4C4B4", "zz381000", "zz2870B4", "zzC46010", "zz000800", "zz88B4D4",
    "zz006800", "zz682800", "zz9C6808", "zz881800", "zz9C9C48", "zzDCE4DC", "zz006008", "zz600000",
    "zz808080", "zz888860", "zz788094", "zz484020", "zz00D418", "zz606060", "zz686060", "zz943000",
    "zz8830C4", "zz782000", "zz9C9C60", "zz889C50", "zz88A4CC", "zz003078", "zz005000", "zz309C30",
    "zz687060", "zz484848", "zz687070", "zzBCC4B4", "zzFC7000", "zz008018", "zz6094C4", "zz6094CC",
    "zz609494", "zz1060AC", "zzA44008", "zz94B4D4", "zzC40000", "zz003000", "zz940000", "zzAC0000",
    "zz883000", "zz002000", "zz401000", "zzB4BCBC", "zz008808", "zz486048", "zz00AC00", "zz9410C4",
    "zzF4D410", "zz783010", "zz942800", "zzC40008", "zz808094", "zz941000", "zz180800", "zz501800",
    "zz080850", "zz103818", "zzB4B4AC", "zz1860AC", "zz9C2088", "zz380800", "zz283840", "zz803008",
    "zz78A4CC", "zz78A470", "zz100000", "zzBCB440", "zzECBC38", "zz202020", "zzA4B4AC", "zz007808",
    "zz505050", "zz007800", "zz880000", "zz1860B4", "zz007810", "zz689CCC", "zz107000", "zz689CB4",
    "zzCCDCC4", "zz107010", "zz580800", "zz680008", "zz087000", "zz007008", "zz580000", "zz482800",
    "zz007000", "zz686860", "zz086000", "zz687878", "zzC40808", "zz384040", "zz004000", "zz946860",
    "zz384020", "zz6000A4", "zzE46008", "zz101008", "zz101010", "zz2068B4", "zz709CCC", "zzB49C30",
    "zz101810", "zz9410CC", "zz200008", "zz943808", "zz181048", "zz5088C4", "zz505858", "zz3878BC",
    "zz781000", "zz4080BC", "zz4880BC", "zz94BCD4", "zz308828", "zz3078BC", "zz782008", "zz780800",
    "zz505038", "zz480000", "zz404040", "zz606058", "zz881808", "zz102008", "zz280000", "zz485040",
    "zz283028", "zz107808", "zz3068BC", "zz001008", "zz5888C4", "zz9C0800", "zz608050", "zz787878",
    "zz4888C4", "zz381800", "zzB40808", "zz787848", "zz786020", "zz08B408", "zz4880C4", "zz884820",
    "zz007020", "zz1868B4", "zz581810", "zz604018", "zzBCCCB4", "zz888894", "zz3070B4", "zz181818",
    "zz103800", "zz78ACCC", "zz081018", "zz50D428", "zz68D4F4", "zz0060CC", "zz683010", "zz6060CC",
    "zz680000", "zz5088BC", "zzA49CA4", "zz940050", "zzBC2030", "zz0858AC", "zz207018", "zz384838",
    "zz0050AC", "zzFCA420", "zzDCA418", "zz106070", "zz0858B4", "zz201818", "zz88FC00", "zz2870BC",
    "zz788880", "zzECDC48", "zz202840", "zz783008", "zz68A4C4", "zzF4A478", "zzEC9C78", "zz784010",
    "zzBC1008", "zz601800", "zz281008", "zz4078BC", "zz001000", "zz40D418", "zz48E408", "zz3828D4",
    "zz20E428", "zz6894C4", "zz0058AC", "zz108010", "zzC4CCC4", "zz085010", "zzDCF4DC", "zzEC1030",
    "zz281000", "zz2868B4", "zz789CDC", "zz5810D4", "zz303050", "zz5894C4", "zzDC0000", "zzD4D4D4",
    "zz00089C", "zz0094C4", "zz585050", "zz48DC28", "zz1050AC", "zzE48060", "zzCC4018", "zzFCFCFC"};

void lgrfile::invalidate_lgr_cache() { CurrentLgrName[0] = '\0'; }

void lgrfile::recreate_lgr_if_needed() {
    if (CurrentLgrName[0]) {
        return;
    }

    lgrfile::load_lgr_file(Level->lgr_name, false);

    Lgr->pal->set();
    canvas::invalidate_canvases();
    reset_game_background();
}

const char* lgrfile::current_lgr_name() { return CurrentLgrName; }

// If `lgr_name` does not exist, only show a dialog if `backup_lgr` is not NULL.
static bool lgr_exists(const char* lgr_name, const char* backup_lgr) {
    filepath path;
    sprintf(path, "lgr/%s.lgr", lgr_name);
    if (std::filesystem::exists(path)) {
        return true;
    }

    if (!backup_lgr) {
        return false;
    }

    // Display warning
    finame filename;
    strcpy(filename, lgr_name);
    strcat(filename, ".lgr");
    blit8(BufferBall, BufferMain);
    BufferMain->fill_box(EditorPaletteId::BACKGROUND);
    blit_to_screen(BufferMain);
    if (!InEditor) {
        EditorPalette->set();
    }
    char backup_text[100];
    sprintf(backup_text, "This file doesn't exist in the LGR directory, so %s will be loaded.",
            backup_lgr);
    dialog("LGR file not found!",
           "The level file uses the pictures that are stored in this LGR file:", filename,
           backup_text,
           "This level file will look now different from that it was designed to look.");
    if (!InEditor) {
        MenuPalette->set();
        blit8(BufferMain, BufferBall);
        blit_to_screen(BufferMain);
    }
    return false;
}

bool lgrfile::try_load_lgr(const char* name, const char* desc) {
    if (!lgr_exists(name, desc)) {
        return false;
    }

    if (strcmpi(name, CurrentLgrName) == 0) {
        return true;
    }

    strcpy(CurrentLgrName, name);
    delete Lgr;
    Lgr = new lgrfile(CurrentLgrName);
    return true;
}

void lgrfile::load_lgr_file(const char* lgr_name, bool warn_missing) {
    if (strlen(lgr_name) > MAX_FILENAME_LEN) {
        internal_error("load_lgr_file strlen( lgr_name ) > MAX_FILENAME_LEN!");
    }

    char lgr_load_name[MAX_FILENAME_LEN + 1] = {};
    strcpy(lgr_load_name, lgr_name);
    strlwr(lgr_load_name);

    // There are 3 possible LGRs this function will try to load in order:
    //   - `lgr_name`
    //   - `eol_settings::default_lgr_name()`
    //   - "default"
    std::string default_override = EolSettings->default_lgr_name();
    const bool is_default = strcmp(lgr_load_name, "default") == 0;
    const bool override_is_same = strcmpi(default_override.c_str(), lgr_load_name) == 0;
    const bool override_is_default = strcmpi(default_override.c_str(), "default") == 0;

    if (!is_default && !override_is_same) {
        const char* desc = override_is_default ? "default.lgr" : "the default lgr file";
        if (try_load_lgr(lgr_load_name, warn_missing ? desc : nullptr)) {
            return;
        }
    }

    if (!override_is_default) {
        if (try_load_lgr(default_override.c_str(), warn_missing ? "default.lgr" : nullptr)) {
            return;
        }
    }

    if (try_load_lgr("default", nullptr)) {
        return;
    }

    external_error("Could not open file lgr/default.lgr!");
}

static void bike_slice(pic8* bike, pic8** ret, bike_box* bbox) {
    pic8* slice = new pic8(bbox->x2 - bbox->x1 + 1, bbox->y2 - bbox->y1 + 1);
    blit8(slice, bike, -bbox->x1, -bbox->y1);
    *ret = slice;
}

// Slice the bike into 4 sub-components
void lgrfile::chop_bike(pic8* bike, bike_pics* bp) {
    bike_slice(bike, &bp->bike_part1, &BikeBox1);
    bike_slice(bike, &bp->bike_part2, &BikeBox2);
    bike_slice(bike, &bp->bike_part3, &BikeBox3);
    bike_slice(bike, &bp->bike_part4, &BikeBox4);
}

// Tile a texture horizontally to fit width.
static pic8* generate_default_texture(texture* text, int width) {
    int original_width = text->original_width;
    int tiles = (width + original_width - 1) / original_width + 1;

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

static std::vector<unsigned char> PictureBuffer;

// Encodes integers >= -1 by storing (value + 1) as unsigned varint.
constexpr int VARINT_MINIMUM = -1;
constexpr int VARINT_SHIFT = 7;
constexpr int VARINT_TAG = 1 << VARINT_SHIFT;
constexpr int VARINT_MASK = VARINT_TAG - 1;
constexpr int VARINT_MAX_BYTES = 5;

static void write_varint(std::vector<unsigned char>& buffer, int value) {
    if (value < VARINT_MINIMUM) {
        internal_error("write_varint() value below VARINT_MINIMUM!");
    }

    value -= VARINT_MINIMUM;
    while (value > VARINT_MASK) {
        buffer.push_back(VARINT_TAG | (value & VARINT_MASK));
        value >>= VARINT_SHIFT;
    }
    buffer.push_back(value & VARINT_MASK);
}

int read_varint(const unsigned char* buffer, int& offset) {
    int value = 0;
    int shift = 0;
    for (int i = 0; i < VARINT_MAX_BYTES; ++i) {
        unsigned char c = buffer[offset++];
        value |= (c & VARINT_MASK) << shift;

        if ((c & VARINT_TAG) == 0) {
            return value + VARINT_MINIMUM;
        }

        shift += VARINT_SHIFT;
    }

    internal_error("read_varint() value too large!");
    return -1;
}

// Store a picture into the lgr.
// Compression format:
//  {
//   Varint: transparent length or -1 if end of row,
//   Varint: non-transparent length, (skipped if end of row)
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

    int transparency = get_transparency_palette_id(list->transparency[index], pic);
    if (transparency < 0) {
        external_error(std::string("Picture must be transparent in lgr file! ") + new_pic->name);
    }

    PictureBuffer.resize(0);
    for (int i = 0; i < new_pic->height; i++) {
        unsigned char* row = pic->get_row(i);

        int x = 0;
        while (true) {
            // Skip pixels
            int skip =
                consecutive_transparent_pixels(x, new_pic->width, row, (unsigned char)transparency);
            x += skip;
            if (x >= new_pic->width) {
                // End of line
                write_varint(PictureBuffer, -1);
                break;
            }

            write_varint(PictureBuffer, skip);

            // Solid pixels
            int count =
                consecutive_solid_pixels(x, new_pic->width, row, (unsigned char)transparency);
            if (count <= 0) {
                internal_error("add_picture count width negative!");
            }
            write_varint(PictureBuffer, count);
            PictureBuffer.insert(PictureBuffer.end(), row + x, row + x + count);
            x += count;
        }
    }

    new_pic->data = new unsigned char[PictureBuffer.size()];
    if (!new_pic->data) {
        internal_error("Not enough memory!");
    }
    std::copy(PictureBuffer.begin(), PictureBuffer.end(), new_pic->data);

    picture_count++;
}

void lgrfile::add_fancyboost(int size, int index) {
    if (picture_count >= MAX_PICTURES) {
        external_error("Too many pictures in lgr file!");
    }

    // Generate picture
    picture* new_pic = &pictures[picture_count];
    strcpy(new_pic->name, FANCYBOOST_NAMES[index]);
    new_pic->default_distance = 999;
    new_pic->default_clipping = Clipping::Ground;
    new_pic->width = size;
    new_pic->height = size;

    PictureBuffer.resize(0);
    for (int i = 0; i < size; i++) {
        write_varint(PictureBuffer, 0);
        write_varint(PictureBuffer, size);
        PictureBuffer.insert(PictureBuffer.end(), size, (unsigned char)index);
        write_varint(PictureBuffer, -1);
    }

    new_pic->data = new unsigned char[PictureBuffer.size()];
    if (!new_pic->data) {
        internal_error("Not enough memory!");
    }
    std::copy(PictureBuffer.begin(), PictureBuffer.end(), new_pic->data);

    picture_count++;
}

void lgrfile::add_texture(pic8* pic, piclist* list, int index) {
    if (texture_count >= MAX_TEXTURES) {
        external_error("Too many textures in lgr file!");
    }

    pic->vertical_flip();

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

static std::vector<mask_element> MaskBuffer;

static void create_mask(mask* dest, pic8* pic, int transparency) {
    dest->width = pic->get_width();
    dest->height = pic->get_height();

    // Special compression format type
    MaskBuffer.resize(0);
    if (transparency >= 0) {
        for (int i = 0; i < dest->height; i++) {
            unsigned char* row = pic->get_row(i);
            int j = 0;
            while (j < dest->width) {
                // Transparent data
                int skip = consecutive_transparent_pixels(j, dest->width, row,
                                                          (unsigned char)transparency);
                if (skip > 0) {
                    mask_element element;
                    element.type = MaskEncoding::Transparent;
                    element.length = skip;
                    MaskBuffer.push_back(element);
                }
                j += skip;

                // Solid data
                int count =
                    consecutive_solid_pixels(j, dest->width, row, (unsigned char)transparency);
                if (count > 0) {
                    mask_element element;
                    element.type = MaskEncoding::Solid;
                    element.length = count;
                    MaskBuffer.push_back(element);
                }
                j += count;
            }
            // End of row
            mask_element element;
            element.type = MaskEncoding::EndOfLine;
            element.length = 0;
            MaskBuffer.push_back(element);
        }
    } else {
        // Solid square special case
        for (int i = 0; i < dest->height; i++) {
            mask_element element;
            element.type = MaskEncoding::Solid;
            element.length = dest->width;
            MaskBuffer.push_back(element);
            element.type = MaskEncoding::EndOfLine;
            element.length = 0;
            MaskBuffer.push_back(element);
        }
    }

    dest->data = new mask_element[MaskBuffer.size()];
    if (!dest->data) {
        internal_error("Memory!");
    }
    std::copy(MaskBuffer.begin(), MaskBuffer.end(), dest->data);
    delete pic;
}

void create_grass_mask(mask& msk, int* heightmap, int skip_rows) {
    int width = msk.width;
    int height = msk.height;

    // Special compression format type
    MaskBuffer.resize(0);
    for (int i = 0; i < skip_rows; i++) {
        mask_element element;
        element.type = MaskEncoding::EndOfLine;
        element.length = 0;
        MaskBuffer.push_back(element);
    }

    for (int i = skip_rows; i < height; i++) {
        int j = 0;
        while (j < width) {
            // Transparent data
            int skip = 0;
            while (j < width && i >= heightmap[j]) {
                skip++;
                j++;
            }
            if (skip > 0) {
                mask_element element;
                element.type = MaskEncoding::Transparent;
                element.length = skip;
                MaskBuffer.push_back(element);
            }

            // Solid data
            int count = 0;
            while (j < width && i < heightmap[j]) {
                count++;
                j++;
            }
            if (count > 0) {
                mask_element element;
                element.type = MaskEncoding::Solid;
                element.length = count;
                MaskBuffer.push_back(element);
            }
        }
        // End of row
        mask_element element;
        element.type = MaskEncoding::EndOfLine;
        element.length = 0;
        MaskBuffer.push_back(element);
    }

    msk.data = new mask_element[MaskBuffer.size()];
    if (!msk.data) {
        internal_error("Memory!");
    }
    std::copy(MaskBuffer.begin(), MaskBuffer.end(), msk.data);
}

void lgrfile::add_mask(pic8* pic, piclist* list, int index) {
    if (mask_count >= MAX_MASKS) {
        external_error("Too many masks in lgr file!");
    }

    // Copy properties
    mask* dest = &masks[mask_count];
    strcpy(dest->name, list->name[index]);
    int transparency = get_transparency_palette_id(list->transparency[index], pic);
    create_mask(dest, pic, transparency);

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

#define ERROR_CORRUPT() external_error(std::string("Corrupt LGR file!: ") + path)

// Read "LGR12" or "LGR13"
static int read_version(FILE* h, const char* path) {
    char LGRXX[6] = {};
    if (fread(LGRXX, 1, 5, h) != 5) {
        ERROR_CORRUPT();
    }
    if (strncmp(LGRXX, "LGR", 3) != 0) {
        external_error(std::string("This is not an LGR file!: ") + path);
    }
    if (LGRXX[3] < '0' || LGRXX[3] > '9' || LGRXX[4] < '0' || LGRXX[4] > '9') {
        external_error(std::string("LGR file's version is too new!: ") + path);
    }
    int version = (LGRXX[3] - '0') * 10 + (LGRXX[4] - '0');
    if (version != 12 && version != 13) {
        external_error(std::string("LGR file's version is too new!: ") + path);
    }
    return version;
}

lgrfile::lgrfile(const char* lgrname) {
    START_TIME(lgr_timer);

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
    double texture_zoom = EolSettings->zoom_textures() ? zoom : 1.0;
    double qgrass_zoom = EolSettings->zoom_grass() ? texture_zoom : 1.0;
    double qupdown_zoom = EolSettings->zoom_grass() ? zoom : 1.0;

    // Load file
    filepath path;
    sprintf(path, "lgr/%s.lgr", lgrname);
    FILE* h = fopen(path, "rb");
    if (!h) {
        external_error(std::string("Cannot find file: ") + path);
    }

    int version = read_version(h, path);

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
    MaskBuffer.reserve(20000);
    PictureBuffer.reserve(40000);
    for (int i = 0; i < pcx_length; i++) {
        char asset_filename[30];
        if (fread(asset_filename, 1, 20, h) != 20) {
            ERROR_CORRUPT();
        }

        // width is unused
        short target_width = -1;
        short target_height = -1;
        if (version == 13) {
            if (fread(&target_width, 1, sizeof(target_width), h) != 2) {
                ERROR_CORRUPT();
            }
            if (fread(&target_height, 1, sizeof(target_height), h) != 2) {
                ERROR_CORRUPT();
            }
        }

        int asset_size = 0;
        if (fread(&asset_size, 1, sizeof(asset_size), h) != 4) {
            ERROR_CORRUPT();
        }
        if (asset_size < 1 || asset_size > 10000000) {
            ERROR_CORRUPT();
        }

        // Skip fancyboost-pattern files as we will internally construct them
        // "zzXXXXXX" where X is a hexadecimal number
        if (asset_filename[0] == 'z' && asset_filename[1] == 'z' && strlen(asset_filename) == 12) {
            fseek(h, asset_size, SEEK_CUR);
            continue;
        }

        int curpos = ftell(h);
        pic8* asset_pic = new pic8(asset_filename, h);
        fseek(h, curpos + asset_size, SEEK_SET);

        if (version == 12) {
            target_height = asset_pic->get_height();
        }

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
    if (strcmpi(asset_filename, (name)) == 0) {                                                    \
        (destination) = asset_pic;                                                                 \
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
            killer = new anim(asset_pic, "qkiller.pcx", target_height, zoom);
            delete asset_pic;
            asset_pic = nullptr;
            continue;
        }
        if (strcmpi(asset_filename, "qexit.pcx") == 0) {
            exit = new anim(asset_pic, "qexit.pcx", target_height, zoom);
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
                food[foodi] = new anim(asset_pic, qfood_name, target_height, zoom);
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
            asset_pic = pic8::resize(asset_pic, target_height);
            grass_pics->add(asset_pic, true, target_height, qupdown_zoom, zoom);
            continue;
        }
        if (strnicmp(asset_filename, "qdown_", 6) == 0) {
            asset_pic = pic8::resize(asset_pic, target_height);
            grass_pics->add(asset_pic, false, target_height, qupdown_zoom, zoom);
            continue;
        }

        // Truncate file extension
        if (!strchr(asset_filename, '.')) {
            external_error(std::string("Cannot find dot in name: ") + asset_filename);
        }
        *strchr(asset_filename, '.') = 0;
        if (strlen(asset_filename) > MAX_FILENAME_LEN) {
            external_error(std::string("Filename is too long in LGR file!: ") + asset_filename +
                           " " + path);
        }

        // QGRASS
        if (strcmpi(asset_filename, "qgrass") == 0) {
            asset_pic = pic8::resize(asset_pic, (int)(qgrass_zoom * target_height));
            add_texture(asset_pic, nullptr, 0);
            continue;
        }

        // Generic asset
        int index = list->get_index(asset_filename);
        if (index < 0) {
            external_error(
                std::string("There is no line in PICTURES.LST corresponding to picture: ") +
                asset_filename);
        }
        if (list->type[index] == piclist::Type::Picture) {
            asset_pic = pic8::resize(asset_pic, (int)(zoom * target_height));
            add_picture(asset_pic, list, index);
            delete asset_pic;
            asset_pic = nullptr;
            continue;
        }
        if (list->type[index] == piclist::Type::Texture) {
            asset_pic = pic8::resize(asset_pic, (int)(texture_zoom * target_height));
            add_texture(asset_pic, list, index);
            // Keep pic8
            continue;
        }
        if (list->type[index] == piclist::Type::Mask) {
            asset_pic = pic8::resize(asset_pic, (int)(zoom * target_height));
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

    // Create fancyboost pictures
    if (EolSettings->fancyboost()) {
        int fancyboost_size = (int)(zoom * 200);
        for (int i = 0; i < 256; i++) {
            add_fancyboost(fancyboost_size, i);
        }
    }

    // Check that the LGR is complete
    if (texture_count < 2) {
        external_error(std::string("There must be at least two textures in LGR file! ") + lgrname);
    }

#define ASSERT_EXISTS(var, name)                                                                   \
    if (!(var)) {                                                                                  \
        external_error(std::string("Picture not found in LGR file!: ") + (name) + " " + path);     \
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

    // Sort pictures, masks and textures alphabetically
    std::sort(pictures, pictures + picture_count,
              [](const picture& a, const picture& b) { return strcmpi(a.name, b.name) < 0; });
    std::sort(masks, masks + mask_count,
              [](const mask& a, const mask& b) { return strcmpi(a.name, b.name) < 0; });
    std::sort(textures, textures + texture_count,
              [](const texture& a, const texture& b) { return strcmpi(a.name, b.name) < 0; });

    // Check for duplicate names in pictures, masks and textures
    for (int i = 0; i < picture_count - 1; i++) {
        if (strcmpi(pictures[i].name, pictures[i + 1].name) == 0) {
            external_error(std::string("Picture name is duplicated in LGR file!: ") +
                           pictures[i].name);
        }
    }

    for (int i = 0; i < mask_count - 1; i++) {
        if (strcmpi(masks[i].name, masks[i + 1].name) == 0) {
            external_error(std::string("Mask name is duplicated in LGR file!: ") + masks[i].name);
        }
    }

    for (int i = 0; i < texture_count - 1; i++) {
        if (strcmpi(textures[i].name, textures[i + 1].name) == 0) {
            external_error(std::string("Texture name is duplicated in LGR file!: ") +
                           textures[i].name);
        }
    }

    // Cleanup
    delete list;
    list = nullptr;

    PictureBuffer.resize(0);
    PictureBuffer.shrink_to_fit();
    MaskBuffer.resize(0);
    MaskBuffer.shrink_to_fit();

    // Editor picture selection initialization
    editor_picture_name[0] = 0;
    editor_mask_name[0] = 0;
    editor_texture_name[0] = 0;

    food_count = 0;
    while (food_count < MAX_QFOOD && food[food_count]) {
        food_count++;
    }
    if (food_count < 1) {
        external_error(std::string("Picture is missing from LGR file: qfood1.pcx ") + path);
    }

    // Check grass
    has_grass = get_texture_index("qgrass") >= 0 && grass_pics->elements.size() >= 2;

    END_TIME(lgr_timer, std::format("{}.lgr (zoom={:.2f})", lgrname, zoom));
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

void lgrfile::sanitize_default_texture_names(char* fg_name, char* bg_name) {
    // Disallow identical foreground/background textures
    if (strcmpi(fg_name, bg_name) == 0) {
        if (EolSettings->default_sky() && !EolSettings->default_ground()) {
            // Prioritize texture override over level-defined texture
            fg_name[0] = 0;
        } else {
            // Default case
            bg_name[0] = 0;
        }
    }

    // Erase missing texture names
    if (get_texture_index(fg_name) < 0) {
        fg_name[0] = 0;
    }

    if (get_texture_index(bg_name) < 0) {
        bg_name[0] = 0;
    }

    if (texture_count < 2) {
        internal_error("Lgr must have at least 2 textures!");
    }

    // If we have missing/invalid texture name, replace the name with a texture from the list.
    // Skip qgrass since it isn't a typical foreground/background texture; fall back to it only
    // when no other texture is available.
    auto pick_default_texture = [&](const char* exclude_name) -> const char* {
        const char* qgrass_fallback = nullptr;
        for (int i = 0; i < texture_count; i++) {
            const char* name = textures[i].name;
            if (strcmpi(name, exclude_name) == 0) {
                continue;
            }
            if (textures[i].is_qgrass) {
                qgrass_fallback = name;
                continue;
            }
            return name;
        }
        return qgrass_fallback;
    };

    if (!fg_name[0]) {
        strcpy(fg_name, pick_default_texture(bg_name));
    }

    if (!bg_name[0]) {
        strcpy(bg_name, pick_default_texture(fg_name));
    }
}

void lgrfile::reload_default_textures(level& lev, int width) {
    char level_foreground_name[10] = {};
    char level_background_name[10] = {};
    if (EolSettings->default_ground()) {
        strcpy(level_foreground_name, "ground");
    } else {
        strncpy(level_foreground_name, lev.foreground_name, sizeof(level_foreground_name) - 1);
    }
    if (EolSettings->default_sky()) {
        strcpy(level_background_name, "sky");
    } else {
        strncpy(level_background_name, lev.background_name, sizeof(level_background_name) - 1);
    }
    sanitize_default_texture_names(level_foreground_name, level_background_name);

    // Recreate background texture
    if (strcmpi(background_name, level_background_name) != 0 || !background ||
        background->get_width() - background_original_width < width) {
        strcpy(background_name, level_background_name);
        delete background;
        background = nullptr;

        int index = get_texture_index(background_name);
        if (index < 0) {
            internal_error("reload_default_textures index not found!");
        }
        texture* text = &textures[index];
        background_original_width = text->original_width;
        background = generate_default_texture(text, width);
    }

    // Recreate foreground texture
    if (strcmpi(foreground_name, level_foreground_name) != 0 || !foreground ||
        foreground->get_width() - foreground_original_width < width) {
        strcpy(foreground_name, level_foreground_name);
        delete foreground;
        foreground = nullptr;

        int index = get_texture_index(foreground_name);
        if (index < 0) {
            internal_error("reload_default_textures index not found!");
        }
        texture* text = &textures[index];
        foreground_original_width = text->original_width;
        foreground = generate_default_texture(text, width);
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
