#include "TOPOL.H"
#include "best_times.h"
#include "ED_CHECK.H"
#include "editor_canvas.h"
#include "editor_dialog.h"
#include "EDITTOLT.H"
#include "EDITUJ.H"
#include "polygon.h"
#include "LGRFILE.H"
#include "LOAD.H"
#include "main.h"
#include "object.h"
#include "physics_init.h"
#include "platform_utils.h"
#include "sprite.h"
#include "qopen.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>

constexpr int TOP_TEN_HEADER = 6754362;
constexpr int TOP_TEN_FOOTER = 8674642;

static char InternalFilePaths[STATE_LEVEL_COUNT + 2][14] = {
    "nulla.leb",    "a01.leb",     "a02.leb",      "a03.leb",      "a04.leb",     "a05.leb",
    "a06.leb",      "a07.leb",     "ujtag.leb",    "a08.leb",      "a09.leb",     "ujgrav.leb",
    "a10.leb",      "a11.leb",     "a12.leb",      "a13.leb",      "a14.leb",     "a15.leb",
    "a16.leb",      "a17.leb",     "ujupdown.leb", "a18.leb",      "a19.leb",     "a20.leb",
    "a21.leb",      "a22.leb",     "a23.leb",      "a24.leb",      "a25.leb",     "a26.leb",
    "ujkomb.leb",   "a27.leb",     "ujtolcs.leb",  "a28.leb",      "ujzuhan.leb", "a29.leb",
    "a30.leb",      "a31.leb",     "a32.leb",      "ujvissza.leb", "a33.leb",     "a34.leb",
    "a35.leb",      "a36.leb",     "a37.leb",      "ujcsab.leb",   "Mate.leb",    "a38.leb",
    "ujdownhi.leb", "ujcsomo.leb", "a39.leb",      "a40.leb",      "a41.leb",     "ujhook.leb",
    "a42.leb",      "visit.leb",   "a04.leb",      "a05.leb",      "a06.leb",     "a07.leb",
    "a08.leb",      "a09.leb",     "a10.leb"};

static char FgetsBuffer[110];

static bool read_encrypted(void* buffer, int length, FILE* h) {
    if (fread(buffer, 1, length, h) != length) {
        return false;
    }
    unsigned char* pc = (unsigned char*)buffer;
    short a = 21;
    short b = 9783;
    short c = 3389;
    for (int i = 0; i < length; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
    return true;
}

static bool write_encrypted(void* buffer, int length, FILE* h) {
    unsigned char* pc = (unsigned char*)buffer;
    short a = 21;
    short b = 9783;
    short c = 3389;
    for (int i = 0; i < length; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
    if (fwrite(buffer, 1, length, h) != length) {
        return false;
    }
    a = 21;
    b = 9783;
    c = 3389;
    for (int i = 0; i < length; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
    return true;
}

const char* get_internal_level_name(int index) {
    constexpr int INTERNAL_MAX_NAME_LENGTH = 30;
    static char InternalLevelNames[INTERNAL_LEVEL_COUNT][INTERNAL_MAX_NAME_LENGTH + 2] = {};
    static bool DescListLoaded = false;
    if (!DescListLoaded) {
        // Initialize on first call from desclist.txt
        DescListLoaded = true;
        FILE* h = qopen("desclist.txt", "r");
        if (!h) {
            internal_error("desclist.txt missing from resource file!");
        }
        for (int i = 0; i < INTERNAL_LEVEL_COUNT; i++) {
            // Each row contains the level name in plaintext
            if (!fgets(FgetsBuffer, 100, h)) {
                internal_error("Failed to read desclist.txt");
            }
            if (strchr(FgetsBuffer, '\n')) {
                *strchr(FgetsBuffer, '\n') = 0;
            }
            if (strchr(FgetsBuffer, '\r')) {
                *strchr(FgetsBuffer, '\r') = 0;
            }
            if (strlen(FgetsBuffer) > INTERNAL_MAX_NAME_LENGTH) {
                internal_error("desclist.txt name is too long!");
            }
            strcpy(InternalLevelNames[i], FgetsBuffer);
        }
        qclose(h);
    }
    if (index < 0 || index >= INTERNAL_LEVEL_COUNT) {
        internal_error("get_internal_level_name index out of bounds");
    }
    // Hardcoded Animal Fram fix
    if (strcmp(InternalLevelNames[index], "Animal Fram") == 0) {
        return "Animal Farm";
    }
    return InternalLevelNames[index];
}

level::level() {
    level_id = 0;
    lgr_not_found = false;
    objects_flipped = false;
    topology_errors = false;
    locked = false;
    topten_file_offset = 0;
    for (int i = 0; i < MAX_POLYGONS; i++) {
        polygons[i] = nullptr;
    }
    for (int i = 0; i < MAX_OBJECTS; i++) {
        objects[i] = nullptr;
    }
    for (int i = 0; i < MAX_SPRITES; i++) {
        sprites[i] = nullptr;
    }
    memset(&toptens, 0, sizeof(toptens));

    polygons[0] = new polygon;
    objects[0] = new object(-2.0, 0.5, object::Type::Exit);
    objects[1] = new object(2.0, 0.5, object::Type::Start);
    strcpy(level_name, "Unnamed");
    strcpy(lgr_name, "DEFAULT");
    strcpy(foreground_name, "ground");
    strcpy(background_name, "sky");
}

level::~level() {
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (polygons[i]) {
            delete polygons[i];
        }
        polygons[i] = nullptr;
    }
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i]) {
            delete objects[i];
        }
        objects[i] = nullptr;
    }
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (sprites[i]) {
            delete sprites[i];
        }
        sprites[i] = nullptr;
    }
}

bool level::discard_missing_lgr_assets(lgrfile* lgr) {
    bool sprites_deleted = false;
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (!sprites[i]) {
            continue;
        }

        sprite* spr = sprites[i];
        // Set default size for editor rendering purposes
        spr->wireframe_width = PixelsToMeters * DEFAULT_SPRITE_WIREFRAME;
        spr->wireframe_height = PixelsToMeters * DEFAULT_SPRITE_WIREFRAME;

        if (spr->picture_name[0] && (spr->mask_name[0] || spr->texture_name[0])) {
            internal_error("discard_missing_lgr_assets invalid pic/mask/text combination!");
        }

        // Delete any sprite not existing in lgr, and also set the size of the asset
        if (spr->picture_name[0]) {
            int index = lgr->getkepindex(spr->picture_name);
            if (index < 0) {
                spr->picture_name[0] = 0;
                delete sprites[i];
                sprites[i] = nullptr;
                sprites_deleted = true;
                continue;
            }

            spr->wireframe_width = lgr->kepek[index].xsize * PixelsToMeters;
            spr->wireframe_height = lgr->kepek[index].ysize * PixelsToMeters;
        } else {
            if (spr->mask_name[0]) {
                int index = lgr->getmaszkindex(spr->mask_name);
                if (index < 0) {
                    spr->mask_name[0] = 0;
                    delete sprites[i];
                    sprites[i] = nullptr;
                    sprites_deleted = true;
                } else {
                    spr->wireframe_width = lgr->maszkok[index].xsize * PixelsToMeters;
                    spr->wireframe_height = lgr->maszkok[index].ysize * PixelsToMeters;
                }

                if (spr->texture_name[0]) {
                    int index = lgr->gettexturaindex(spr->texture_name);
                    if (index < 0) {
                        spr->texture_name[0] = 0;
                        delete sprites[i];
                        sprites[i] = nullptr;
                        sprites_deleted = true;
                    }
                }
            }
        }
    }

    bool sprites_shifted = true;
    while (sprites_shifted) {
        sprites_shifted = false;
        for (int i = 0; i < MAX_SPRITES - 1; i++) {
            if (!sprites[i] && sprites[i + 1]) {
                sprites[i] = sprites[i + 1];
                sprites[i + 1] = nullptr;
                sprites_shifted = true;
            }
        }
    }

    // Disallow identical foreground/background textures
    if (strcmpi(foreground_name, background_name) == 0) {
        background_name[0] = 0;
    }

    // Erase missing texture names
    if (Plgr->gettexturaindex(foreground_name) < 0) {
        foreground_name[0] = 0;
    }

    if (Plgr->gettexturaindex(background_name) < 0) {
        background_name[0] = 0;
    }

    if (Plgr->texturaszam < 2) {
        internal_error("Lgr must have at least 2 textures!");
    }

    // If we have missing/invalid texture name, replace the name with a texture from the list
    // We grab the lowest index texture where foreground_name != background_name
    if (!foreground_name[0]) {
        if (strcmpi(background_name, Plgr->texturak[0].nev) != 0) {
            strcpy(foreground_name, Plgr->texturak[0].nev);
        } else {
            strcpy(foreground_name, Plgr->texturak[1].nev);
        }
    }

    if (!background_name[0]) {
        if (strcmpi(foreground_name, Plgr->texturak[0].nev) != 0) {
            strcpy(background_name, Plgr->texturak[0].nev);
        } else {
            strcpy(background_name, Plgr->texturak[1].nev);
        }
    }

    return sprites_deleted;
}

polygon* level::get_closest_vertex(double x, double y, int* vertex_index, double* distance,
                                   polygon* skip) {
    if (distance) {
        *distance = 1000000000.0;
    }
    double closest_distance = 1000000.0;
    polygon* poly = nullptr;
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (!polygons[i]) {
            continue;
        }

        if (polygons[i] == skip) {
            continue;
        }

        // Ignore polygons that aren't rendered in the editor
        if (polygons[i]->is_grass && !Rajzolkoveto) {
            continue;
        }

        if (!polygons[i]->is_grass && !Rajzolpoligon) {
            continue;
        }

        int new_index;
        double new_distance = polygons[i]->get_closest_vertex(x, y, &new_index);
        if (new_distance < closest_distance) {
            closest_distance = new_distance;
            *vertex_index = new_index;
            poly = polygons[i];
        }
    }

    if (closest_distance > max_grab_distance()) {
        *vertex_index = 0;
        return nullptr;
    }

    if (distance) {
        *distance = closest_distance;
    }

    return poly;
}

object* level::get_closest_object(double x, double y, double* distance) {
    if (distance) {
        *distance = 1000000000.0;
    }
    double closest_distance = 1000000.0;
    object* obj = nullptr;
    vect2 r(x, y);
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i]) {
            double new_distance = (objects[i]->r - r).length();
            if (new_distance < closest_distance) {
                closest_distance = new_distance;
                obj = objects[i];
            }
        }
    }

    if (closest_distance > max_grab_distance()) {
        return nullptr;
    }

    if (distance) {
        *distance = closest_distance;
    }

    return obj;
}

sprite* level::get_closest_sprite(double x, double y, double* distance) {
    if (distance) {
        *distance = 1000000000.0;
    }
    double closest_distance = 1000000.0;
    sprite* spr = nullptr;
    vect2 r(x, y);
    for (int i = 0; i < MAX_SPRITES; i++) {
        // Skip all sprites if they aren't rendered in editor
        if (!Rajzolkepek) {
            continue;
        }

        if (!sprites[i]) {
            continue;
        }

        double new_distance = (sprites[i]->r - r).length();
        if (new_distance < closest_distance) {
            closest_distance = new_distance;
            spr = sprites[i];
        }
    }

    if (closest_distance > max_grab_distance()) {
        return nullptr;
    }

    if (distance) {
        *distance = closest_distance;
    }

    return spr;
}

void level::render() {
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (polygons[i]) {
            if (polygons[i]->is_grass) {
                if (Rajzolkoveto) {
                    polygons[i]->render_outline();
                }
            } else {
                if (Rajzolpoligon) {
                    polygons[i]->render_outline();
                }
            }
        }
    }

    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i]) {
            objects[i]->render();
        }
    }

    for (int i = 0; i < MAX_SPRITES; i++) {
        if (sprites[i]) {
            if (Rajzolkepek) {
                sprites[i]->render();
            }
        }
    }
}

bool level::is_sky(polygon* poly, vect2* point) {
    if ((poly && point) || (!poly && !point)) {
        internal_error("level::is_sky only one parameter may be used!");
    }

    // Grab the first vertex of polygon or just take the input point
    vect2 r;
    if (poly) {
        r = poly->vertices[0];
    } else {
        r = *point;
    }

    // See how many times a line drawn from here to outside of the map will intersect
    vect2 v = vect2(27654.475374565578, 37850.5364775); // Possible polarity inversion bug here
    int intersections = 0;
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (polygons[i] && !polygons[i]->is_grass && polygons[i] != poly) {
            intersections += polygons[i]->count_intersections(r, v);
        }
    }

    // Based on even/odd number of intersections we can tell if it is ground or sky
    return intersections % 2;
}

int get_internal_index(const char* filename) {
    // QWQUU
    if (strnicmp(filename, "QWQUU", 5) != 0) {
        return 0;
    }
    // 001
    const char* number_string = &filename[5];
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum *= 10;
        int add = *number_string - '0';
        if (add < 0 || add > 9) {
            return 0;
        }
        sum += add;
        number_string++;
    }
    // .lev
    if (strcmpi(&filename[8], ".lev") != 0) {
        return 0;
    }
    // Valid number
    if (sum <= INTERNAL_LEVEL_COUNT) {
        return sum;
    }
    return 0;
}

int access_level_file(const char* filename) {
    // Internals always accessible
    int internal_index = get_internal_index(filename);
    if (internal_index > 0) {
        return 0;
    }
    // For externals, normal access function call
    char tmp[30];
    sprintf(tmp, "lev/%s", filename);
    return access(tmp, 0);
}

char BestTime[30] = "";

void load_best_time(const char* filename, int single) {
    if (strlen(filename) > 20) {
        internal_error("load_best_time strlen(filename) > 20");
    }
    // Get the topten table
    int internal_index = get_internal_index(filename);
    topten_set* tten_set = nullptr;
    if (internal_index > 0) {
        internal_index--; // Zero-indexed
        if (internal_index >= INTERNAL_LEVEL_COUNT) {
            internal_error("load_best_time internal_index out of range!");
        }
        tten_set = &State->toptens[internal_index];
    } else {
        if (!Ptop) {
            internal_error(
                "load_best_time external level must be loaded before getting best time!");
        }
        tten_set = &Ptop->toptens;
    }
    topten* tten = nullptr;
    if (single) {
        tten = &tten_set->single;
    } else {
        tten = &tten_set->multi;
    }

    // Write the best time or null string if no best time
    if (tten->times_count > 0) {
        centiseconds_to_string(tten->times[0], BestTime);
    } else {
        BestTime[0] = 0;
    }
}

static char InternalLevelLgrs[INTERNAL_LEVEL_COUNT + 1][10]; // 1-indexed
static char StrlwrBuffer[50] = "";

static void load_internal_lgr_filenames() {
    // Initialize all levels with lgr name "default".lgr
    for (int i = 1; i <= INTERNAL_LEVEL_COUNT; i++) {
        strcpy(InternalLevelLgrs[i], "default");
    }

    // Try to load lgrlist.txt to override default.lgr
    FILE* h = fopen("lgr/lgrlist.txt", "r");
    if (!h) {
        return;
    }

    while (true) {
        // Read each row from lgrlist.txt
        if (!fgets(FgetsBuffer, 40, h)) {
            break;
        }

        // Parse the line as NUMBER LGR
        // e.g. 10 fancy.lgr
        int internal_index = 0;
        if (sscanf(FgetsBuffer, "%d%s", &internal_index, StrlwrBuffer) != 2) {
            continue;
        }

        if (internal_index < 1 || internal_index > INTERNAL_LEVEL_COUNT) {
            continue;
        }

        strlwr(StrlwrBuffer);
        if (!strstr(StrlwrBuffer, ".lgr")) {
            continue;
        }

        *strstr(StrlwrBuffer, ".lgr") = 0;
        char* lgr_filename = StrlwrBuffer;
        while ((*lgr_filename == ' ') || (*lgr_filename == '\t')) {
            lgr_filename++;
        }

        if ((strlen(lgr_filename) > 8) || (strlen(lgr_filename) < 1)) {
            continue;
        }

        // Store lgr name if valid
        strcpy(InternalLevelLgrs[internal_index], lgr_filename);
    }

    fclose(h);
}

level::level(const char* filename) {
    static bool InternalLgrNamesLoaded = false;
    if (!InternalLgrNamesLoaded) {
        InternalLgrNamesLoaded = true;
        load_internal_lgr_filenames();
    }

    if (strlen(filename) > 20) {
        internal_error("level::level strlen(filename) > 20");
    }

    int internal_index = get_internal_index(filename);
    if (internal_index > 0) {
        load_internal(InternalFilePaths[internal_index]);
        // Override lgr name from lgrlist.txt
        char lgrpath[40];
        const char* lgrname = InternalLevelLgrs[internal_index];
        sprintf(lgrpath, "lgr/%s.lgr", lgrname);
        if (access(lgrpath, 0) == 0) {
            strcpy(lgr_name, lgrname);
        }
    } else {
        load_external(filename);
    }
}

void level::load_external(const char* filename) {
    lgr_not_found = false;
    objects_flipped = false;
    topology_errors = false;
    locked = false;
    topten_file_offset = 0;
    memset(&toptens, 0, sizeof(toptens));
    for (int i = 0; i < MAX_POLYGONS; i++) {
        polygons[i] = nullptr;
    }
    for (int i = 0; i < MAX_OBJECTS; i++) {
        objects[i] = nullptr;
    }
    for (int i = 0; i < MAX_SPRITES; i++) {
        sprites[i] = nullptr;
    }

    FILE* h = nullptr;
    char path[40];
    sprintf(path, "lev/%s", filename);
    h = fopen(path, "rb");
    if (!h) {
        internal_error("Failed to open level file: ", path);
    }

    char tmp[10] = "AAAAA";
    if (fread(tmp, 1, 5, h) != 5) {
        internal_error("Error reading level file!", filename);
    }
    if (strncmp(tmp, "POT", 3) != 0) {
        internal_error("Corrupt .LEV file!", filename);
    }

    level_id = 0;
    double integrity_checksum = 0;
    int version = tmp[4] - '0' + 10 * (tmp[3] - '0');

    if (version > 14) {
        external_error("Level file's version is too new!: ", filename);
    }

    if (version != 6 && version != 14) {
        external_error("Corrupt .LEV file!", filename);
    }

    int level_id_checksum;
    if (version >= 13) {
        if (fread(&level_id_checksum, 1, 2, h) != 2) {
            internal_error("Error reading level file!", filename);
        }
    }

    if (fread(&level_id, 1, sizeof(level_id), h) != 4) {
        internal_error("Error reading level file!", filename);
    }

    if (fread(&integrity_checksum, 1, sizeof(integrity_checksum), h) != 8) {
        internal_error("Corrupt .LEV file!", filename);
    }

    double integrity_shareware;
    if (fread(&integrity_shareware, 1, sizeof(integrity_shareware), h) != 8) {
        internal_error("Error reading level file!", filename);
    }
    if (integrity_shareware + integrity_checksum < 9786.0 ||
        integrity_shareware + integrity_checksum > 36546.0) {
        internal_error("Corrupt .LEV file!", filename);
    }

    // Levels that can only be played in Registered would meet the condition:
    // integrity_shareware + integrity_checksum <= 20000.0

    double integrity_topology_errors;
    if (fread(&integrity_topology_errors, 1, sizeof(integrity_topology_errors), h) != 8) {
        internal_error("Error reading level file!", filename);
    }
    if (integrity_topology_errors + integrity_checksum < 9786.0 ||
        integrity_topology_errors + integrity_checksum > 36546.0) {
        internal_error("Corrupt .LEV file!", filename);
    }
    if (integrity_topology_errors + integrity_checksum > 20000.0) {
        topology_errors = true;
    }

    double integrity_locked;
    if (fread(&integrity_locked, 1, sizeof(integrity_locked), h) != 8) {
        internal_error("Error reading level file!", filename);
    }
    if (integrity_locked + integrity_checksum < 9875.0 ||
        integrity_locked + integrity_checksum > 32345.0) {
        internal_error("Corrupt .LEV file!", filename);
    }
    if (integrity_locked + integrity_checksum > 20000.0) {
        locked = true;
    }

    int level_name_length = LEVEL_NAME_LENGTH;
    if (version < 14) {
        level_name_length = LEVEL_NAME_LENGTH_OLD;
    }
    fread(level_name, 1, level_name_length + 1, h);
    level_name[level_name_length] = 0;

    if (version > 6) {
        fread(lgr_name, 1, 16, h);
        lgr_name[15] = 0;
    } else {
        strcpy(lgr_name, "default");
    }

    if (version >= 8) {
        fread(foreground_name, 1, 10, h);
        foreground_name[9] = 0;
        fread(background_name, 1, 10, h);
        foreground_name[9] = 0;
    } else {
        strcpy(foreground_name, "ground");
        strcpy(background_name, "sky");
    }

    if (version < 14) {
        fseek(h, 100, SEEK_SET);
    }

    double encrypted_polygon_count = 0;
    if (fread(&encrypted_polygon_count, 1, sizeof(encrypted_polygon_count), h) != 8) {
        internal_error("Error reading level file!", filename);
    }

    int polygon_count = (int)(encrypted_polygon_count);
    if (polygon_count > MAX_POLYGONS) {
        internal_error("Too many polygons in level file!", filename);
    }
    if (polygon_count <= 0) {
        internal_error("Corrupt .LEV file!", filename);
    }

    for (int i = 0; i < polygon_count; i++) {
        polygons[i] = new polygon(h, version);
    }

    double encrypted_object_count = 0;
    if (fread(&encrypted_object_count, 1, sizeof(encrypted_object_count), h) != 8) {
        internal_error("Error reading level file!", filename);
    }

    int object_count = (int)(encrypted_object_count);
    if (object_count > MAX_OBJECTS) {
        internal_error("Too many objects in level file!", filename);
    }
    if (object_count <= 0) {
        internal_error("Corrupt .LEV file!", filename);
    }

    for (int i = 0; i < object_count; i++) {
        objects[i] = new object(h, version);
    }

    if (version > 6) {
        double encrypted_sprite_count = 0;
        if (fread(&encrypted_sprite_count, 1, sizeof(encrypted_sprite_count), h) != 8) {
            internal_error("Error reading level file!", filename);
        }

        int sprite_count = (int)(encrypted_sprite_count);
        if (sprite_count > MAX_SPRITES) {
            internal_error("Too many pictures in level file!", filename);
        }
        if (sprite_count < 0) {
            internal_error("Corrupt .LEV file!", filename);
        }

        if (version < 8 && sprite_count > 0) {
            internal_error("version < 8 && sprite_count > 0!", filename);
        }

        for (int i = 0; i < sprite_count; i++) {
            sprites[i] = new sprite(h);
        }
    }

    // Top ten (optional)
    // Store the position of the topten so we can write to it later
    topten_file_offset = ftell(h);
    if (topten_file_offset < 6) {
        internal_error("topten_file_offset < 6");
    }

    int magic_number = 0;
    if (fread(&magic_number, 1, sizeof(magic_number), h) != 4 || magic_number != TOP_TEN_HEADER) {
    } else {
        if (!read_encrypted(&toptens, sizeof(toptens), h) ||
            fread(&magic_number, 1, sizeof(magic_number), h) != 4 ||
            magic_number != TOP_TEN_FOOTER) {
            memset(&toptens, 0, sizeof(toptens));
        }
    }
    fclose(h);

    // Verify checksum
    double calculated_checksum = checksum();
    if (fabs(calculated_checksum - integrity_checksum) > 0.01) {
        external_error("Corrupt .LEV file!", filename);
    }
}

void level::load_internal(const char* filename) {
    lgr_not_found = false;
    objects_flipped = false;
    topology_errors = false;
    locked = false;

    topten_file_offset = 0;
    memset(&toptens, 0, sizeof(toptens));

    for (int i = 0; i < MAX_POLYGONS; i++) {
        polygons[i] = nullptr;
    }
    for (int i = 0; i < MAX_OBJECTS; i++) {
        objects[i] = nullptr;
    }
    for (int i = 0; i < MAX_SPRITES; i++) {
        sprites[i] = nullptr;
    }

    FILE* h = nullptr;
    h = qopen(filename, "rb");
    if (!h) {
        internal_error("Failed to open level file: ", filename);
    }

    char tmp[10] = "AAAAA";
    if (fread(tmp, 1, 5, h) != 5) {
        internal_error("Error reading level file!", filename);
    }
    if (strncmp(tmp, "@@^!@", 5) != 0) {
        internal_error("Corrupt .LEV file!", filename);
    }

    level_id = 0;
    double integrity_checksum = 0;
    /*int verzio = tmp[4] - '0' + 10*(tmp[3] - '0');

    if( verzio < 6 )
        external_error( "Corrupt level file!: ", filename );

    if( verzio > 14 )
        external_error( "Level file's version is too new!: ", filename );
    */
    int version = 14;

    int level_id_checksum;
    if (version >= 13) {
        if (fread(&level_id_checksum, 1, 2, h) != 2) {
            internal_error("Error reading level file!", filename);
        }
    }

    if (fread(&level_id, 1, sizeof(level_id), h) != 4) {
        internal_error("Error reading level file!", filename);
    }

    if (fread(&integrity_checksum, 1, sizeof(integrity_checksum), h) != 8) {
        internal_error("Corrupt .LEV file!", filename);
    }

    double integrity_shareware;
    if (fread(&integrity_shareware, 1, sizeof(integrity_shareware), h) != 8) {
        internal_error("Error reading level file!", filename);
    }
    if (integrity_shareware + integrity_checksum < 9786.0 ||
        integrity_shareware + integrity_checksum > 36546.0) {
        internal_error("Corrupt .LEV file!", filename);
    }

    double integrity_topology_errors;
    if (fread(&integrity_topology_errors, 1, sizeof(integrity_topology_errors), h) != 8) {
        internal_error("Error reading level file!", filename);
    }
    if (integrity_topology_errors + integrity_checksum < 9786.0 ||
        integrity_topology_errors + integrity_checksum > 36546.0) {
        internal_error("Corrupt .LEV file!", filename);
    }
    if (integrity_topology_errors + integrity_checksum > 20000.0) {
        topology_errors = true;
    }

    double integrity_locked;
    if (fread(&integrity_locked, 1, sizeof(integrity_locked), h) != 8) {
        internal_error("Error reading level file!", filename);
    }
    if (integrity_locked + integrity_checksum < 9875.0 ||
        integrity_locked + integrity_checksum > 32345.0) {
        internal_error("Corrupt .LEV file!", filename);
    }
    if (integrity_locked + integrity_checksum > 20000.0) {
        locked = true;
    }

    int level_name_length = LEVEL_NAME_LENGTH;
    if (version < 14) {
        level_name_length = LEVEL_NAME_LENGTH_OLD;
    }
    fread(level_name, 1, level_name_length + 1, h);
    level_name[level_name_length] = 0;

    if (version > 6) {
        fread(lgr_name, 1, 16, h);
        lgr_name[15] = 0;
    } else {
        strcpy(lgr_name, "default");
    }

    if (version >= 8) {
        fread(foreground_name, 1, 10, h);
        foreground_name[9] = 0;
        fread(background_name, 1, 10, h);
        background_name[9] = 0;
    } else {
        strcpy(foreground_name, "ground");
        strcpy(background_name, "sky");
    }

    if (version < 14) {
        qseek(h, 100, SEEK_SET);
    }
    double encrypted_polygon_count = 0;
    if (fread(&encrypted_polygon_count, 1, sizeof(encrypted_polygon_count), h) != 8) {
        internal_error("Error reading level file!", filename);
    }

    int polygon_count = (int)(encrypted_polygon_count);
    if (polygon_count > MAX_POLYGONS) {
        internal_error("Too many polygons in level file!", filename);
    }
    if (polygon_count <= 0) {
        internal_error("Corrupt .LEV file!", filename);
    }

    double encrypted_object_count = 0;
    if (fread(&encrypted_object_count, 1, sizeof(encrypted_object_count), h) != 8) {
        internal_error("Error reading level file!", filename);
    }

    int object_count = (int)(encrypted_object_count);
    if (object_count > MAX_OBJECTS) {
        internal_error("Too many objects in level file!", filename);
    }
    if (object_count <= 0) {
        internal_error("Corrupt .LEV file!");
    }

    for (int i = 0; i < polygon_count; i++) {
        polygons[i] = new polygon(h, version);
    }

    for (int i = 0; i < object_count; i++) {
        objects[i] = new object(h, version);
    }

    if (version > 6) {
        double encrypted_sprite_count = 0;
        if (fread(&encrypted_sprite_count, 1, sizeof(encrypted_sprite_count), h) != 8) {
            internal_error("Error reading level file!", filename);
        }

        int sprite_count = (int)(encrypted_sprite_count);
        if (sprite_count > MAX_SPRITES) {
            internal_error("Too many pictures in level file!", filename);
        }
        if (sprite_count < 0) {
            internal_error("Corrupt .LEV file!", filename);
        }

        if (version < 8 && sprite_count > 0) {
            internal_error("version < 8 && sprite_count > 0!", filename);
        }

        for (int i = 0; i < sprite_count; i++) {
            sprites[i] = new sprite(h);
        }
    }

    qclose(h);

    double calculated_checksum = checksum();
    if (fabs(calculated_checksum - integrity_checksum) > 0.01) {
        external_error("Corrupt .LEV file!", filename);
    }
}

// Generate random level ID.
// Upper 16 bits are random.
// Lower 16 bits are a hashed checksum.
static int generate_level_id(double checksum) {
    srand(clock());
    unsigned int random = random_range(6542);
    random *= random_range(7042);
    random += random_range(4542);
    random *= random_range(3042);
    random *= random_range(3742);
    random += random_range(9187);

    checksum = sin(checksum);
    checksum *= (checksum + 1.0001) * 40000;
    unsigned int level_id_checksum = (unsigned int)(checksum);

    unsigned int level_id = (level_id_checksum & 0x0000ffff) | (random & 0xffff0000);
    return level_id;
}

void level::save(const char* filename, bool skip_topology) {
    memset(&toptens, 0, sizeof(toptens));

    Volttopsave = 1;
    if (objects_flipped) {
        internal_error("level::save objects_flipped!");
    }
    if (skip_topology) {
        topology_errors = false;
    } else {
        topology_errors = check_topology(0);
        if (topology_errors && locked) {
            dialog("You cannot save this file as a locked file because there are",
                   "some topology errors in the design and you could not play on it!");
            return;
        }
    }

    char path[40];
    sprintf(path, "lev/%s", filename);
#ifdef BELSOTIR
    if (!strstr(path, ".lev") && !strstr(path, ".LEV")) {
        internal_error("Internal level save filename must end in .LEV!");
    }
    if (strstr(path, ".lev")) {
        strcpy(strstr(path, ".lev"), ".leb");
    }
    if (strstr(path, ".LEV")) {
        strcpy(strstr(path, ".LEV"), ".leb");
    }
#endif
    FILE* h = fopen(path, "wb");
    if (!h) {
        internal_error("Failed to open file for writing: ", path);
    }

#ifdef BELSOTIR
    fwrite("@@^!@", 1, 5, h);
#else
    fwrite("POT14", 1, 5, h);
#endif

    // Generate checksum and level id
    double integrity_checksum = checksum();
    level_id = generate_level_id(integrity_checksum);
    fwrite(&level_id, 1, 2, h); // hashed checksum
    fwrite(&level_id, 1, 4, h);
    fwrite(&integrity_checksum, 1, sizeof(integrity_checksum), h);

    // Shareware levels would meet the criteria:
    // integrity_shareware + integrity_checksum > 20000.0
    // which originally was calculated as 20961.0 + random_range(4982) - integrity_checksum;
    double integrity_shareware = 11877.0 + random_range(5871) - integrity_checksum;
#ifdef BELSOTIR
    integrity_shareware = 20961.0 + random_range(4982) - integrity_checksum;
#endif

    fwrite(&integrity_shareware, 1, sizeof(integrity_shareware), h);

    double integrity_topology_errors = 11877.0 + random_range(5871) - integrity_checksum;
    if (topology_errors) {
        integrity_topology_errors = 20961.0 + random_range(4982) - integrity_checksum;
    }
    fwrite(&integrity_topology_errors, 1, sizeof(integrity_topology_errors), h);

    double integrity_locked = 12112.0 + random_range(6102) - integrity_checksum;
    if (locked) {
        integrity_locked = 23090.0 + random_range(6310) - integrity_checksum;
    }
    fwrite(&integrity_locked, 1, sizeof(integrity_topology_errors), h);

#ifdef BELSOTIR
    for (int iiii = 0; iiii < LEVEL_NAME_LENGTH; iiii++) {
        levelname[iiii] = 0;
    }
#endif
    fwrite(level_name, 1, LEVEL_NAME_LENGTH + 1, h);

    fwrite(lgr_name, 1, 16, h);
    fwrite(foreground_name, 1, 10, h);
    fwrite(background_name, 1, 10, h);

    int polygon_count = 0;
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (polygons[i]) {
            polygon_count++;
        }
    }
    double polygon_count_encrypted = polygon_count + 0.4643643;

    int object_count = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i]) {
            object_count++;
        }
    }
    double object_count_encrypted = object_count + 0.4643643;

    fwrite(&polygon_count_encrypted, 1, sizeof(polygon_count_encrypted), h);
#ifdef BELSOTIR
    fwrite(&object_count_encrypted, 1, sizeof(object_count_encrypted), h);
#endif

    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (polygons[i]) {
            polygons[i]->save(h, this);
        }
    }
#ifndef BELSOTIR
    fwrite(&object_count_encrypted, 1, sizeof(object_count_encrypted), h);
#endif

    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i]) {
            objects[i]->save(h);
        }
    }

    int sprite_count = 0;
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (sprites[i]) {
            sprite_count++;
        }
    }

    double sprite_count_encrypted = sprite_count + 0.2345672;
    fwrite(&sprite_count_encrypted, 1, sizeof(sprite_count_encrypted), h);
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (sprites[i]) {
            sprites[i]->save(h);
        }
    }

    int magic_number = TOP_TEN_HEADER;
    fwrite(&magic_number, 1, 4, h);
    write_encrypted(&toptens, sizeof(toptens), h);
    magic_number = TOP_TEN_FOOTER;
    fwrite(&magic_number, 1, 4, h);

    fclose(h);

    if (topology_errors) {
#ifdef BELSOTIR
        internal_error("#define BELSOTIR mellet hibas level file mentes!:", filename);
#endif
        dialog("Though the level file was successfully saved, there are some errors in the design.",
               "You cannot play on this level until you correct these problems. To see what the",
               "problems are, please push the Check Topology button in the editor.");
    }
}

// Update top ten of an external level
void level::save_topten(const char* filename) {
    if (topten_file_offset < 6) {
        internal_error("save_topten cannot be used with internal levels!");
    }

    char path[40];
    sprintf(path, "lev/%s", filename);

    FILE* h = fopen(path, "r+b");
    if (!h) {
        external_error("Could not open file!", path);
    }

    if (fseek(h, topten_file_offset, SEEK_SET) != 0) {
        external_error("Could not write to file:", path);
    }

    int magic_number = TOP_TEN_HEADER;
    if (fwrite(&magic_number, 1, 4, h) != 4) {
        external_error("Could not write to file:", path);
    }

    if (!write_encrypted(&toptens, sizeof(toptens), h)) {
        external_error("Could not write to file:", path);
    }

    magic_number = TOP_TEN_FOOTER;
    if (fwrite(&magic_number, 1, 4, h) != 4) {
        external_error("Could not write to file:", path);
    }

    fclose(h);
}

void level::get_boundaries(double* x1, double* y1, double* x2, double* y2,
                           bool check_objects_and_sprites) {
    *x1 = 100000000000.0;
    *y1 = 100000000000.0;
    *x2 = -100000000000.0;
    *y2 = -100000000000.0;
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (polygons[i]) {
            polygons[i]->update_boundaries(x1, y1, x2, y2);
        }
    }
    if (check_objects_and_sprites) {
        for (int i = 0; i < MAX_OBJECTS; i++) {
            if (objects[i]) {
                if (*x1 > objects[i]->r.x) {
                    *x1 = objects[i]->r.x;
                }
                if (*x2 < objects[i]->r.x) {
                    *x2 = objects[i]->r.x;
                }
                if (*y1 > objects[i]->r.y) {
                    *y1 = objects[i]->r.y;
                }
                if (*y2 < objects[i]->r.y) {
                    *y2 = objects[i]->r.y;
                }
            }
        }
        for (int i = 0; i < MAX_SPRITES; i++) {
            if (sprites[i]) {
                if (*x1 > sprites[i]->r.x) {
                    *x1 = sprites[i]->r.x;
                }
                if (*x2 < sprites[i]->r.x) {
                    *x2 = sprites[i]->r.x;
                }
                if (*y1 > sprites[i]->r.y) {
                    *y1 = sprites[i]->r.y;
                }
                if (*y2 < sprites[i]->r.y) {
                    *y2 = sprites[i]->r.y;
                }
            }
        }
    }
}

double level::checksum() {
    double sum = 0.0;
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (polygons[i]) {
            sum += polygons[i]->checksum();
        }
    }
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i]) {
            sum += objects[i]->checksum();
        }
    }
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (sprites[i]) {
            sum += sprites[i]->checksum();
        }
    }

    constexpr double CHECKSUM_MULTIPLIER = 3247.764325643;
    return CHECKSUM_MULTIPLIER * sum;
}

vect2 BikeStartOffset;

int level::initialize_objects(motorst* mot) {
    int apple_count = 0;
    bool start_found = false;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* obj = objects[i];
        if (obj) {
            // Set a random phase to every object
            obj->floating_phase = random_range(1000) * 2.0 * PI / 1000.0;
            obj->active = true;
            if (obj->type == object::Type::Food) {
                apple_count++;
            }

            if (obj->type == object::Type::Start) {
                // Only allow one start object
                if (start_found) {
                    internal_error("Level can only have one Start object!");
                }
                start_found = true;
                // Hide start object. Store the bike position to respawn in flag tag mode
                obj->active = false;
                BikeStartOffset = obj->r - mot->left_wheel.r;
                mot->bike.r = mot->bike.r + BikeStartOffset;
                mot->left_wheel.r = mot->left_wheel.r + BikeStartOffset;
                mot->right_wheel.r = mot->right_wheel.r + BikeStartOffset;
                mot->body_r = mot->body_r + BikeStartOffset;
            }
        }
    }

    if (!start_found) {
        internal_error("Start object not found in level file!");
    }

    return apple_count;
}

inline static int object_order(object::Type type) {
    switch (type) {
    case object::Type::Killer:
        return 1;
    case object::Type::Food:
        return 2;
    case object::Type::Exit:
        return 3;
    default:
        return 10;
    }
}

// sort: Killer, Apple, Exit, Start
void level::sort_objects() {
    int count = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* obj = objects[i];
        if (obj) {
            count++;
        }
    }

    // Check there are no NULL objects
    for (int i = 0; i < count; i++) {
        object* obj = objects[i];
        if (!obj) {
            internal_error("level::sort_objects has a gap!");
        }
    }

    // We need a Start and Exit at least
    if (count < 2) {
        internal_error("level::sort_objects count < 2!");
    }

    // Sort in following priority: Killer, Apple, Exit, Start
    std::stable_sort(objects, objects + count, [](const object* a, const object* b) {
        return object_order(a->type) < object_order(b->type);
    });
}

object* level::get_object(int index) {
    if (index < 0 || index >= MAX_OBJECTS) {
        internal_error("level::get_object index < 0 || index >= MAX_OBJECTS!");
    }

    object* obj = objects[index];
    if (!obj) {
        internal_error("level::get_object !obj!");
    }

    return obj;
}

void level::flip_objects() {
    if (objects_flipped) {
        return;
    }
    objects_flipped = true;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* obj = objects[i];
        if (obj) {
            obj->r.y = -obj->r.y;
        }
    }
}

void level::unflip_objects() {
    if (!objects_flipped) {
        return;
    }

    objects_flipped = false;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* obj = objects[i];
        if (obj) {
            obj->r.y = -obj->r.y;
        }
    }
}
