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
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>

#define TOP_TEN_HEADER (6754362)
#define TOP_TEN_FOOTER (8674642)

// Ertekeik kommentkent kint is vannak:

#define CHECKSUM_MULTIPLIER (3247.764325643)

// Ha define-olva van, save belso lev formatumban, .leb kiterjesztessel
// ment mintha shareware lenne, plusz hakellmentbelleveket fuggveny
// osszes levelt igy ki is menti:
// #define BELSOTIR

static char InternalFilePaths[STATE_LEVEL_COUNT + 2][14] = {
    "nulla.leb",    "a01.leb",    "a02.leb",      "a03.leb",   "a04.leb",     "a05.leb",
    "a06.leb",      "a07.leb",    "ujtag.leb",    "a08.leb",   "a09.leb",     "ujgrav.leb",
    "a10.leb",

    "a11.leb",      "a12.leb",    "a13.leb",      "a14.leb",   "a15.leb",     "a16.leb",
    "a17.leb",

    "ujupdown.leb", "a18.leb",    "a19.leb",      "a20.leb",

    "a21.leb",      "a22.leb",    "a23.leb",      "a24.leb",   "a25.leb",     "a26.leb",
    "ujkomb.leb",   "a27.leb",    "ujtolcs.leb",  "a28.leb",   "ujzuhan.leb", "a29.leb",
    "a30.leb",

    "a31.leb",      "a32.leb",    "ujvissza.leb", "a33.leb",   "a34.leb",     "a35.leb",
    "a36.leb",      "a37.leb",    "ujcsab.leb",   "Mate.leb",  "a38.leb",     "ujdownhi.leb",
    "ujcsomo.leb",  "a39.leb",    "a40.leb",

    "a41.leb",      "ujhook.leb", "a42.leb",      "visit.leb", "a04.leb",     "a05.leb",
    "a06.leb",      "a07.leb",    "a08.leb",      "a09.leb",   "a10.leb"};

int DescListLoaded = 0;
#define TOPOL_INTERNAL_LEVEL_COUNT (55)
#define INTERNAL_MAX_NAME_LENGTH (30)
char InternalLevelNames[(INTERNAL_MAX_NAME_LENGTH + 2) * TOPOL_INTERNAL_LEVEL_COUNT + 6];

static char FgetsBuffer[110];

// Van ugyanilyen neven state.cpp-ben is ket fv.:

// Sikerest adja vissza:
static int read_encrypted(void* buffer, int length, FILE* h) {
    if (fread(buffer, 1, length, h) != length) {
        return 0;
    }
    unsigned char* pc = (unsigned char*)buffer;
    short a = 21, b = 9783, c = 3389;

    for (int i = 0; i < length; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
    return 1;
}

// Sikerest adja vissza:
static int write_encrypted(void* buffer, int length, FILE* h) {
    unsigned char* pc = (unsigned char*)buffer;
    short a = 21, b = 9783, c = 3389;

    for (int i = 0; i < length; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
    if (fwrite(buffer, 1, length, h) != length) {
        return 0;
    }
    a = 21, b = 9783, c = 3389;

    for (int i = 0; i < length; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
    return 1;
}

const char* get_internal_level_name(int index) {
    if (!DescListLoaded) {
        DescListLoaded = 1;
        FILE* h = qopen("desclist.txt", "r");
        if (!h) {
            internal_error("desclist.txt missing from resource file!");
        }

        for (int i = 0; i < TOPOL_INTERNAL_LEVEL_COUNT; i++) {
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
            strcpy(&InternalLevelNames[i * (INTERNAL_MAX_NAME_LENGTH + 2)], FgetsBuffer);
        }

        qclose(h);
    }
    if (index < 0 || index >= TOPOL_INTERNAL_LEVEL_COUNT) {
        internal_error("get_internal_level_name index out of bounds");
    }
    if (index == 30) { // Animal Farm
        return "Animal Farm";
    }
    return &InternalLevelNames[index * (INTERNAL_MAX_NAME_LENGTH + 2)];
}

// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL

level::level(void) {
    level_id = 0;
    lgr_not_found = 0;
    objects_flipped = 0;
    topology_errors = 0;
    locked = 0;
    topten_file_offset = 0;

    for (int i = 0; i < MAX_POLYGONS; i++) {
        polygons[i] = NULL;
    }
    for (int i = 0; i < MAX_OBJECTS; i++) {
        objects[i] = NULL;
    }
    for (int i = 0; i < MAX_SPRITES; i++) {
        sprites[i] = NULL;
    }

    memset(&toptens, 0, sizeof(toptens));

    polygons[0] = new polygon;
    objects[0] = new object(-2, 0.5, object::Type::Exit);
    objects[1] = new object(2, 0.5, object::Type::Start);
    strcpy(level_name, "Unnamed");
    strcpy(lgr_name, "DEFAULT");
    strcpy(foreground_name, "ground");
    strcpy(background_name, "sky");
}

level::~level(void) {
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (polygons[i]) {
            delete polygons[i];
        }
        polygons[i] = NULL;
    }
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i]) {
            delete objects[i];
        }
        objects[i] = NULL;
    }
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (sprites[i]) {
            delete sprites[i];
        }
        sprites[i] = NULL;
    }
}

// Igazzal ter vissza, ha volt valtozas:
int level::discard_missing_lgr_assets(lgrfile* lgr) {
    // Most minden sprite-ot kitorol, ha nincs meg hozza LGR-file-ban
    // kep, maszk, vagy textura nev:

    // Ezt lehetne hasznalni dialogushoz:
    int sprites_deleted = 0;
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (sprites[i]) {
            sprite* spr = sprites[i];
            // Default meret:
            spr->wireframe_width = PixelsToMeters * DEFAULT_SPRITE_WIREFRAME;
            spr->wireframe_height = PixelsToMeters * DEFAULT_SPRITE_WIREFRAME;

            if (spr->picture_name[0] && (spr->mask_name[0] || spr->texture_name[0])) {
                internal_error("discard_missing_lgr_assets invalid pic/mask/text combination!");
            }
            if (spr->picture_name[0]) {
                int index = lgr->getkepindex(spr->picture_name);
                if (index < 0) {
                    spr->picture_name[0] = 0;
                    // Toroljuk:
                    delete sprites[i];
                    sprites[i] = NULL;
                    sprites_deleted = 1;
                    continue;
                }
                // Megvan kep:
                spr->wireframe_width = lgr->kepek[index].xsize;
                spr->wireframe_height = lgr->kepek[index].ysize;
                spr->wireframe_width *= PixelsToMeters;
                spr->wireframe_height *= PixelsToMeters;
            } else {
                // Nem kep sprite:
                if (spr->mask_name[0]) {
                    int index = lgr->getmaszkindex(spr->mask_name);
                    if (index < 0) {
                        spr->mask_name[0] = 0;
                        // Toroljuk:
                        delete sprites[i];
                        sprites[i] = NULL;
                        sprites_deleted = 1;
                    } else {
                        spr->wireframe_width = lgr->maszkok[index].xsize;
                        spr->wireframe_height = lgr->maszkok[index].ysize;
                        spr->wireframe_width *= PixelsToMeters;
                        spr->wireframe_height *= PixelsToMeters;
                    }
                }
                if (spr->texture_name[0]) {
                    int index = lgr->gettexturaindex(spr->texture_name);
                    if (index < 0) {
                        spr->texture_name[0] = 0;
                        // Toroljuk:
                        delete sprites[i];
                        sprites[i] = NULL;
                        sprites_deleted = 1;
                    }
                }
            }
        }
    }

    // Most meg spritetomb elejere kompaktaljuk spriteokat:
    int sprites_shifted = 1;
    while (sprites_shifted) {
        sprites_shifted = 0;
        for (int i = 0; i < MAX_SPRITES - 1; i++) {
            if (!sprites[i] && sprites[i + 1]) {
                // Kovetkezo mutatot ide hozzuk:
                sprites[i] = sprites[i + 1];
                sprites[i + 1] = NULL;
                sprites_shifted = 1;
            }
        }
    }

    // Kiselejtezzuk es ha kell ujra is toltjuk fore es background neveket:
    if (strcmpi(Ptop->foreground_name, Ptop->background_name) == 0) {
        Ptop->background_name[0] = 0;
    }
    if (Plgr->gettexturaindex(Ptop->foreground_name) < 0) {
        Ptop->foreground_name[0] = 0;
    }
    if (Plgr->gettexturaindex(Ptop->background_name) < 0) {
        Ptop->background_name[0] = 0;
    }

    if (Plgr->texturaszam < 2) {
        internal_error("Lgr must have at least 2 textures!");
    }

    if (!Ptop->foreground_name[0]) {
        if (strcmpi(Ptop->background_name, Plgr->texturak[0].nev) != 0) {
            strcpy(Ptop->foreground_name, Plgr->texturak[0].nev);
        } else {
            strcpy(Ptop->foreground_name, Plgr->texturak[1].nev);
        }
    }

    if (!Ptop->background_name[0]) {
        if (strcmpi(Ptop->foreground_name, Plgr->texturak[0].nev) != 0) {
            strcpy(Ptop->background_name, Plgr->texturak[0].nev);
        } else {
            strcpy(Ptop->background_name, Plgr->texturak[1].nev);
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
    polygon* poly = NULL;
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (!polygons[i]) {
            continue;
        }
        if (polygons[i] == skip) {
            continue;
        }
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
        return NULL;
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
    object* obj = NULL;
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
        return NULL;
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
    sprite* spr = NULL;
    vect2 r(x, y);
    for (int i = 0; i < MAX_SPRITES; i++) {
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
        return NULL;
    }
    if (distance) {
        *distance = closest_distance;
    }
    return spr;
}

void level::render(void) {
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (polygons[i]) {
            if (polygons[i]->is_grass) {
                // Kovetok:
                if (Rajzolkoveto) {
                    polygons[i]->render_outline();
                }
            } else {
                // Normal poligonok:
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

// Igaz, ha bent van:
int level::is_sky(polygon* poly, vect2* point) {
    if ((poly && point) || (!poly && !point)) {
        internal_error("level::is_sky only one parameter may be used!");
    }

    vect2 r;
    if (poly) {
        r = poly->vertices[0]; // poly->center();
    } else {
        r = *point;
    }
    vect2 v = vect2(27654.475374565578, 37850.5364775);
    // Vegigmegyunk az egeszen:
    int intersections = 0;
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (polygons[i] && !polygons[i]->is_grass && polygons[i] != poly) {
            intersections += polygons[i]->count_intersections(r, v);
        }
    }
    if (intersections % 2) {
        return 1;
    } else {
        return 0;
    }
}

int get_internal_index(const char* filename) {
    // (rekorder.cpp-ben is be van huzalozva ez a string:)
    if (strnicmp(filename, "QWQUU", 5) != 0) {
        return 0;
    }

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

    if (strcmpi(&filename[8], ".lev") != 0) {
        return 0;
    }

    if (sum <= INTERNAL_LEVEL_COUNT) {
        return sum;
    }
    return 0;
}

// Ha letezik file, akkor 0-t ad vissza!:
int access_level_file(const char* filename) {
    int internal_index = get_internal_index(filename);
    if (internal_index > 0) {
        return 0;
    }
    char tmp[30];
    sprintf(tmp, "lev/%s", filename);
    return access(tmp, 0);
}

char BestTime[30] = "";

// Azt a nevet keri, amit topol::topol kap:
void load_best_time(const char* filename, int single) {
    if (strlen(filename) > 20) {
        internal_error("load_best_time strlen(filename) > 20");
    }

    int internal_index = get_internal_index(filename);
    topten_set* tten_set = NULL;
    if (internal_index > 0) {
        internal_index--; // Mivel eggyel nagyobb tomb indexenel
                          // Belso palya:
        if (internal_index >= INTERNAL_LEVEL_COUNT) {
            internal_error("load_best_time internal_index out of range!");
        }

        tten_set = &State->toptens[internal_index];
    } else {
        // Kulso palya:
        if (!Ptop) {
            internal_error(
                "load_best_time external level must be loaded before getting best time!");
        }
        tten_set = &Ptop->toptens;
    }

    topten* tten = NULL;
    if (single) {
        tten = &tten_set->single;
    } else {
        tten = &tten_set->multi;
    }

    if (tten->times_count > 0) {
        centiseconds_to_string(tten->times[0], BestTime);
    } else {
        BestTime[0] = 0;
    }
}

// Elso palya indexe 1:
static char* InternalLevelLgrs[110] = {NULL, NULL}; // 1-es indexut figyeli hogy init-e

static char StrlwrBuffer[50] = "";

static void load_internal_lgr_filenames(void) {
    if (INTERNAL_LEVEL_COUNT > 105) {
        internal_error("load_internal_lgr_filenames too many internal levels!");
    }

    for (int i = 0; i < 110; i++) {
        InternalLevelLgrs[i] = NULL;
    }

    static char* internal_lgrs_data = NULL;

    internal_lgrs_data = new char[10 * (INTERNAL_LEVEL_COUNT + 2)];
    if (!internal_lgrs_data) {
        internal_error("load_internal_lgr_filenames out of memory!");
    }

    for (int i = 1; i <= INTERNAL_LEVEL_COUNT; i++) {
        InternalLevelLgrs[i] = &internal_lgrs_data[10 * (i - 1)];
        strcpy(InternalLevelLgrs[i], "default");
    }

    // Most fel van toltve default ertekekkel, beolvassuk file-t:
    FILE* h = fopen("lgr/lgrlist.txt", "r");
    if (!h) {
        return;
    }

    while (1) {
        // Beolvasunk egy sort:
        if (!fgets(FgetsBuffer, 40, h)) {
            break;
        }
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
        // Levagjuk elejerol space-eket es tabokat:
        while ((*lgr_filename == ' ') || (*lgr_filename == 9)) {
            lgr_filename++;
        }
        if ((strlen(lgr_filename) > 8) || (strlen(lgr_filename) < 1)) {
            continue;
        }
        strcpy(InternalLevelLgrs[internal_index], lgr_filename);
    }

    fclose(h);
}

// QWQUU001 elso palya:
level::level(const char* filename) {
    if (!InternalLevelLgrs[1]) {
        load_internal_lgr_filenames();
    }

    int is_external = 1;

    char filepath[21];
    if (strlen(filename) > 20) {
        internal_error("level::level strlen(filename) > 20");
    }
    strcpy(filepath, filename);

    int internal_index = get_internal_index(filepath);
    if (internal_index > 0) {
        is_external = 0;
        strcpy(filepath, InternalFilePaths[internal_index]);
    }

    // Innentol kezdve ket kulon rutin fogja beolvasni kulso es belso file-okat
    // a kulso beolvaso a jo, a belso beolvaso egy kicsit meg van valtoztatva
    // es #define BELSOTIR eseten igy menti ki file-t topol.

    // loadbelso beolvassa kerekek szamat is sok gyuru elott, mig kulso
    // beolvassa sok gyurut es csak utana olvassa kerekek szamat es kerekeket

    if (is_external) {
        load_external(filepath);
    } else {
        load_internal(filepath);
        // Sajat tablabol vesszuk lgr neveket:
        // #ifdef DEBUG 		Demo miatt most nem ellenorizzuk
        //	if( internal_index < 0 || internal_index > INTERNAL_LEVEL_COUNT )
        //		internal_error( "4i4fyuif" );
        // #endif
        char lgrpath[40];
        sprintf(lgrpath, "lgr/%s.lgr", InternalLevelLgrs[internal_index]);
        if (access(lgrpath, 0) == 0) { // Csak ha letezik file:
            strcpy(lgr_name, InternalLevelLgrs[internal_index]);
        }
    }
}

void level::load_external(const char* filename) {
    ////int kulso = 1;

    lgr_not_found = 0;
    objects_flipped = 0;
    topology_errors = 0;
    locked = 0;

    topten_file_offset = 0;
    memset(&toptens, 0, sizeof(toptens));

    for (int i = 0; i < MAX_POLYGONS; i++) {
        polygons[i] = NULL;
    }
    for (int i = 0; i < MAX_OBJECTS; i++) {
        objects[i] = NULL;
    }
    for (int i = 0; i < MAX_SPRITES; i++) {
        sprites[i] = NULL;
    }

    FILE* h = NULL;
    ////if( kulso ) {
    char path[40];
    sprintf(path, "lev/%s", filename);
    h = fopen(path, "rb");
    if (!h) {
        internal_error("Failed to open level file: ", path);
    }
    /*////}
    else {
        h = qopen( filenev, "rb" );
        if( !h )
            internal_error( "level::level-ban nem tudta nyitni file-t!: ", filenev );
    } */////

    // Verzio beolvasasa:
    char tmp[10] = "AAAAA";
    if (fread(tmp, 1, 5, h) != 5) {
        internal_error("Error reading level file!");
    }
    if (strncmp(tmp, "POT", 3) != 0) {
        internal_error("Corrupt .LEV file!");
    }

    level_id = 0;
    double integrity_checksum = 0;
    int version = tmp[4] - '0' + 10 * (tmp[3] - '0');

    if (version > 14) {
        external_error("Level file's version is too new!: ", filename);
    }

    // Most csak ezt a ket verziot tudja olvasni, de eleg kikommentezni
    // hogy mindet olvassa:
    if (version != 6 && version != 14) {
        external_error("Corrupt .LEV file!", filename);
    }

    // verzio 6: 1.2-es verzio levele
    // verzio 7: uj verzio
    // verzio 8: uj verzio kovetovel es uj gyurukkel( maszk, texturanev)
    // verzio 10: benne van idotabla is file vegen
    // verzio 11: kerek-ben benne van foodsorszam ertek is
    // verzio 12: uj koveto (csak koveto tenye van tarolva gyuru-ben)
    // verzio 13: plusz 2 byte-ot beolvas meg belyeg elott
    // verzio 14: LEVELNAME (14)-rol (50)-re emelkedik, nem seek-el 100-ra

    int level_id_checksum;
    if (version >= 13) {
        if (fread(&level_id_checksum, 1, 2, h) != 2) {
            internal_error("Error reading level file!");
        }
    }

    // Belyeg:
    if (fread(&level_id, 1, 4, h) != 4) {
        internal_error("Error reading level file!");
    }

    if (fread(&integrity_checksum, 1, sizeof(integrity_checksum), h) != sizeof(double)) {
        internal_error("Corrupt .LEV file!");
    }

    // Beolvassuk hogy shareware-e:
    double integrity_shareware;
    if (fread(&integrity_shareware, 1, sizeof(integrity_shareware), h) !=
        sizeof(integrity_shareware)) {
        internal_error("Error reading level file!");
    }
    if (integrity_shareware + integrity_checksum < 9786.0 ||
        integrity_shareware + integrity_checksum > 36546.0) {
        internal_error("Corrupt .LEV file!");
    }

    // Levels that can only be played in Registered would meet the condition: integrity_shareware +
    // belyeg_d
    // <= 20000.0

    // Beolvassuk hogy hibas-e:
    double integrity_topology_errors;
    if (fread(&integrity_topology_errors, 1, sizeof(integrity_topology_errors), h) !=
        sizeof(integrity_topology_errors)) {
        internal_error("Error reading level file!");
    }
    if (integrity_topology_errors + integrity_checksum < 9786.0 ||
        integrity_topology_errors + integrity_checksum > 36546.0) {
        internal_error("Corrupt .LEV file!");
    }
    if (integrity_topology_errors + integrity_checksum > 20000.0) {
        topology_errors = 1;
    }

    // Beolvassuk hogy lezart-e:
    double integrity_locked;
    if (fread(&integrity_locked, 1, sizeof(integrity_locked), h) != sizeof(integrity_locked)) {
        internal_error("Error reading level file!");
    }
    if (integrity_locked + integrity_checksum < 9875.0 ||
        integrity_locked + integrity_checksum > 32345.0) {
        internal_error("Corrupt .LEV file!");
    }
    if (integrity_locked + integrity_checksum > 20000.0) {
        locked = 1;
    }

    // lezart = 0; // Ha ez nincs kikomentezve, unlockol minden file-t

    // Beolvassuk level nevet:
    int level_name_length = LEVEL_NAME_LENGTH;
    if (version < 14) {
        level_name_length = 14;
    }
    fread(level_name, 1, level_name_length + 1, h);
    level_name[level_name_length] = 0; // Ez csak biztonsagert

    // Beolvassuk lgr nevet:
    if (version > 6) {
        fread(lgr_name, 1, 16, h);
        lgr_name[15] = 0; // Ez csak biztonsagert
    } else {
        strcpy(lgr_name, "default");
    }

    if (version >= 8) {
        fread(foreground_name, 1, 10, h);
        foreground_name[9] = 0; // Ez csak biztonsagert
        fread(background_name, 1, 10, h);
        foreground_name[9] = 0; // Ez csak biztonsagert
    } else {
        strcpy(foreground_name, "ground");
        strcpy(background_name, "sky");
    }

    // Gyuruk szamat 100 byte utan kezdjuk:
    if (version < 14) {
        ////if( kulso )
        fseek(h, 100, SEEK_SET);
    }
    ////else
    ////    qseek( h, 100, SEEK_SET );

    double encrypted_polygon_count = 0;
    if (fread(&encrypted_polygon_count, 1, sizeof(encrypted_polygon_count), h) !=
        sizeof(encrypted_polygon_count)) {
        internal_error("Error reading level file!");
    }

    int polygon_count = (int)(encrypted_polygon_count);
    if (polygon_count > MAX_POLYGONS) {
        internal_error("Too many polygons in level file!");
    }
    if (polygon_count <= 0) {
        internal_error("Corrupt .LEV file!");
    }

    // Beolvassa sok gyurut:
    for (int i = 0; i < polygon_count; i++) {
        polygons[i] = new polygon(h, version);
    }

    // Kerekek szama:
    double encrypted_object_count = 0;
    if (fread(&encrypted_object_count, 1, sizeof(encrypted_object_count), h) !=
        sizeof(encrypted_object_count)) {
        internal_error("Error reading level file!");
    }

    int object_count = (int)(encrypted_object_count);
    if (object_count > MAX_OBJECTS) {
        internal_error("Too many objects in level file!");
    }
    if (object_count <= 0) {
        internal_error("Corrupt .LEV file!");
    }

    for (int i = 0; i < object_count; i++) {
        // Beolvassa sok kereket:
        objects[i] = new object(h, version);
    }

    if (version > 6) {
        // Most van SPRITE beolvasas is:
        // Kerekek szama:
        double encrypted_sprite_count = 0;
        if (fread(&encrypted_sprite_count, 1, sizeof(encrypted_sprite_count), h) !=
            sizeof(encrypted_sprite_count)) {
            internal_error("Error reading level file!");
        }

        int sprite_count = (int)(encrypted_sprite_count);
        if (sprite_count > MAX_SPRITES) {
            internal_error("Too many pictures in level file!");
        }
        if (sprite_count < 0) {
            internal_error("Corrupt .LEV file!");
        }

        if (version < 8 && sprite_count > 0) {
            internal_error("version < 8 && sprite_count > 0!", filename);
        }

        for (int i = 0; i < sprite_count; i++) {
            // Beolvassa sok kereket:
            sprites[i] = new sprite(h);
        }
    }

    // Ezekutan mar csak idotabla lehet!!!

    // Meg beolvassuk ido tablat is kulso file eseten:
    // idokhelye es idok tartalma eddig 0:
    ////if( kulso ) {
    topten_file_offset = ftell(h);
    if (topten_file_offset < 6) {
        internal_error("topten_file_offset < 6");
    }

    int magic_number = 0;
    if (fread(&magic_number, 1, 4, h) != 4 || magic_number != TOP_TEN_HEADER) {
        // if( verzio >= 10 )
        //   external_error( "Corrupt file:", filenev );
    } else {
        if (!read_encrypted(&toptens, sizeof(toptens), h) || fread(&magic_number, 1, 4, h) != 4 ||
            magic_number != TOP_TEN_FOOTER) {
            memset(&toptens, 0, sizeof(toptens));
            // external_error( "Corrupt file:", filenev );
        }
    }
    ////}

    ////if( kulso )
    fclose(h);
    ////else
    ////    qclose( h );

    // Most megnezzuk, hogy belyeg stimmel-e:
    double calculated_checksum = checksum(version > 6);
    if (fabs(calculated_checksum - integrity_checksum) > 0.01) {
        external_error("Corrupt .LEV file!: ", filename);
    }
}

// Ezt ha ujra kell kepezni loadkulso atalakitasaval kell:
void level::load_internal(const char* filename) {
    lgr_not_found = 0;
    objects_flipped = 0;
    topology_errors = 0;
    locked = 0;

    topten_file_offset = 0;
    memset(&toptens, 0, sizeof(toptens));

    for (int i = 0; i < MAX_POLYGONS; i++) {
        polygons[i] = NULL;
    }
    for (int i = 0; i < MAX_OBJECTS; i++) {
        objects[i] = NULL;
    }
    for (int i = 0; i < MAX_SPRITES; i++) {
        sprites[i] = NULL;
    }

    FILE* h = NULL;
    h = qopen(filename, "rb");
    if (!h) {
        internal_error("Failed to open level file: ", filename);
    }

    // Verzio beolvasasa:
    char tmp[10] = "AAAAA";
    if (fread(tmp, 1, 5, h) != 5) {
        internal_error("Error reading level file!");
    }
    if (strncmp(tmp, "@@^!@", 3) != 0) {
        internal_error("Corrupt .LEV file!");
    }

    level_id = 0;
    double integrity_checksum = 0;
    /*int verzio = tmp[4] - '0' + 10*(tmp[3] - '0');

    if( verzio < 6 )
        external_error( "Corrupt level file!: ", filename );

    if( verzio > 14 )
        external_error( "Level file's version is too new!: ", filename );
    */
    int version = 14; // Mivel leb file-okat mindig igy mentjuk

    // verzio 6: 1.2-es verzio levele
    // verzio 7: uj verzio
    // verzio 8: uj verzio kovetovel es uj gyurukkel( maszk, texturanev)
    // verzio 10: benne van idotabla is file vegen
    // verzio 11: kerek-ben benne van foodsorszam ertek is
    // verzio 12: uj koveto (csak koveto tenye van tarolva gyuru-ben)
    // verzio 13: plusz 2 byte-ot beolvas meg belyeg elott
    // verzio 14: LEVELNAME (14)-rol (50)-re emelkedik, nem seek-el 100-ra

    int level_id_checksum;
    if (version >= 13) {
        if (fread(&level_id_checksum, 1, 2, h) != 2) {
            internal_error("Error reading level file!");
        }
    }

    // Belyeg:
    if (fread(&level_id, 1, 4, h) != 4) {
        internal_error("Error reading level file!");
    }

    if (fread(&integrity_checksum, 1, sizeof(integrity_checksum), h) != sizeof(double)) {
        internal_error("Corrupt .LEV file!");
    }

    // Beolvassuk hogy shareware-e:
    double integrity_shareware;
    if (fread(&integrity_shareware, 1, sizeof(integrity_shareware), h) !=
        sizeof(integrity_shareware)) {
        internal_error("Error reading level file!");
    }
    if (integrity_shareware + integrity_checksum < 9786.0 ||
        integrity_shareware + integrity_checksum > 36546.0) {
        internal_error("Corrupt .LEV file!");
    }

    // Beolvassuk hogy hibas-e:
    double integrity_topology_errors;
    if (fread(&integrity_topology_errors, 1, sizeof(integrity_topology_errors), h) !=
        sizeof(integrity_topology_errors)) {
        internal_error("Error reading level file!");
    }
    if (integrity_topology_errors + integrity_checksum < 9786.0 ||
        integrity_topology_errors + integrity_checksum > 36546.0) {
        internal_error("Corrupt .LEV file!");
    }
    if (integrity_topology_errors + integrity_checksum > 20000.0) {
        topology_errors = 1;
    }

    // Beolvassuk hogy lezart-e:
    double integrity_locked;
    if (fread(&integrity_locked, 1, sizeof(integrity_locked), h) != sizeof(integrity_locked)) {
        internal_error("Error reading level file!");
    }
    if (integrity_locked + integrity_checksum < 9875.0 ||
        integrity_locked + integrity_checksum > 32345.0) {
        internal_error("Corrupt .LEV file!");
    }
    if (integrity_locked + integrity_checksum > 20000.0) {
        locked = 1;
    }

    // if( lezart ) {
    //  Ez csak debug-hoz:
    //  dialog( "Le volt zarva!" );
    //}

    // lezart = 0; // Ha ez nincs kikomentezve, unlockol minden file-t

    // Beolvassuk level nevet:
    int level_name_length = LEVEL_NAME_LENGTH;
    if (version < 14) {
        level_name_length = 14;
    }
    fread(level_name, 1, level_name_length + 1, h);
    level_name[level_name_length] = 0; // Ez csak biztonsagert

    // Beolvassuk lgr nevet:
    if (version > 6) {
        fread(lgr_name, 1, 16, h);
        lgr_name[15] = 0; // Ez csak biztonsagert
    } else {
        strcpy(lgr_name, "default");
    }

    if (version >= 8) {
        fread(foreground_name, 1, 10, h);
        foreground_name[9] = 0; // Ez csak biztonsagert
        fread(background_name, 1, 10, h);
        background_name[9] = 0; // Ez csak biztonsagert
    } else {
        strcpy(foreground_name, "ground");
        strcpy(background_name, "sky");
    }

    // Gyuruk szamat 100 byte utan kezdjuk:
    if (version < 14) {
        qseek(h, 100, SEEK_SET);
    }
    double encrypted_polygon_count = 0;
    if (fread(&encrypted_polygon_count, 1, sizeof(encrypted_polygon_count), h) !=
        sizeof(encrypted_polygon_count)) {
        internal_error("Error reading level file!");
    }

    int polygon_count = (int)(encrypted_polygon_count);
    if (polygon_count > MAX_POLYGONS) {
        internal_error("Too many polygons in level file!");
    }
    if (polygon_count <= 0) {
        internal_error("Corrupt .LEV file!");
    }

    // Kerekek szama:
    double encrypted_object_count = 0;
    if (fread(&encrypted_object_count, 1, sizeof(encrypted_object_count), h) !=
        sizeof(encrypted_object_count)) {
        internal_error("Error reading level file!");
    }

    int object_count = (int)(encrypted_object_count);
    if (object_count > MAX_OBJECTS) {
        internal_error("Too many objects in level file!");
    }
    if (object_count <= 0) {
        internal_error("Corrupt .LEV file!");
    }

    // Beolvassa sok gyurut:
    for (int i = 0; i < polygon_count; i++) {
        polygons[i] = new polygon(h, version);
    }

    // Beolvassa sok kereket:
    for (int i = 0; i < object_count; i++) {
        objects[i] = new object(h, version);
    }

    if (version > 6) {
        // Most van SPRITE beolvasas is:
        // Kerekek szama:
        double encrypted_sprite_count = 0;
        if (fread(&encrypted_sprite_count, 1, sizeof(encrypted_sprite_count), h) !=
            sizeof(encrypted_sprite_count)) {
            internal_error("Error reading level file!");
        }

        int sprite_count = (int)(encrypted_sprite_count);
        if (sprite_count > MAX_SPRITES) {
            internal_error("Too many pictures in level file!");
        }
        if (sprite_count < 0) {
            internal_error("Corrupt .LEV file!");
        }

        if (version < 8 && sprite_count > 0) {
            internal_error("version < 8 && sprite_count > 0!", filename);
        }

        for (int i = 0; i < sprite_count; i++) {
            // Beolvassa sok kereket:
            sprites[i] = new sprite(h);
        }
    }

    // Ezekutan mar csak idotabla lehet, de mivel belso file, az sincs.
    qclose(h);

    // Most megnezzuk, hogy belyeg stimmel-e:
    double calculated_checksum = checksum(version > 6);
    if (fabs(calculated_checksum - integrity_checksum) > 0.01) {
        external_error("Corrupt .LEV file!: ", filename);
    }
}

static long generate_level_id(double checksum) {
    srand(clock());
    unsigned long random = random_range(6542);
    random *= random_range(7042);
    random += random_range(4542);
    random *= random_range(3042);
    random *= random_range(3742);
    random += random_range(9187);

    checksum = sin(checksum);
    checksum *= (checksum + 1.0001) * 40000;
    unsigned long level_id_checksum = checksum;

    unsigned long level_id = (level_id_checksum & 0x0000ffff) | (random & 0xffff0000);
    return level_id;
}

void level::save(const char* filename, int skip_topology) {
    memset(&toptens, 0, sizeof(toptens));
    // if( lezart )
    //   internal_error( "level::save-ben lezart!" );

    Volttopsave = 1; // load.cpp-nek kell ez
    if (objects_flipped) {
        internal_error("level::save objects_flipped!");
    }
    // Mivel ezzel meg valtoztathatunk polygonokon
    // (polygon korbejaras megvaltoztatas, egymason fekvo pontok
    // megszuntetese, tul hegyes szogek eltolasa), ezert ezt elore tesszuk:
    if (skip_topology) {
        topology_errors = 0;
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

    // Kiszamitjuk es kiirjuk ketfele belyeget:
    double integrity_checksum = checksum(1);
    level_id = generate_level_id(integrity_checksum);

    fwrite(&level_id, 1, 2, h); // Kiirunk ket tetszoleges byte-ot

    fwrite(&level_id, 1, 4, h);
    fwrite(&integrity_checksum, 1, sizeof(integrity_checksum), h);

    // Kiirjuk hogy sarvari-e:
    // Shareware levels would meet the criteria integrity_shareware + double( integrity_checksum ) >
    // 20000.0 which originally was calculated as 20961.0 + random_range( 4982 ) -
    // integrity_checksum;
    double integrity_shareware = 11877.0 + random_range(5871) - integrity_checksum;

// Regisztralt verzioju file-t is shareware-nek tunteti fol:
#ifdef BELSOTIR
    integrity_shareware = 20961.0 + random_range(4982) - integrity_checksum;
#endif

    fwrite(&integrity_shareware, 1, sizeof(integrity_shareware), h);

    // Kiirjuk hogy hibas-e:
    // Ha hibaertek + double(integrity_checksum) > 20000.0, akkor hibas:
    double integrity_topology_errors = 11877.0 + random_range(5871) - integrity_checksum;
    if (topology_errors) { // ezt mar elejen meghataroztuk
        integrity_topology_errors = 20961.0 + random_range(4982) - integrity_checksum;
    }
    fwrite(&integrity_topology_errors, 1, sizeof(integrity_topology_errors), h);

    // Kiirjuk hogy lezart-e:
    double integrity_locked = 12112.0 + random_range(6102) - integrity_checksum;
    if (locked) {
        integrity_locked = 23090.0 + random_range(6310) - integrity_checksum;
    }
    fwrite(&integrity_locked, 1, sizeof(integrity_topology_errors), h);

// Kiirjuk level nevet:
#ifdef BELSOTIR
    // leb-nel toroljuk nevet:
    for (int iiii = 0; iiii < LEVEL_NAME_LENGTH; iiii++) {
        levelname[iiii] = 0;
    }
#endif
    fwrite(level_name, 1, LEVEL_NAME_LENGTH + 1, h);

    // Kiirjuk level nevet:
    fwrite(lgr_name, 1, 16, h);
    // Kiirjuk hatter es eloter nevet:
    fwrite(foreground_name, 1, 10, h);
    fwrite(background_name, 1, 10, h);

    // Megszamoljuk gyuruket es kerekeket:
    // fseek( h, 100, SEEK_SET ); 14-es verzio ota mar nem seek-el

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

    // Kimentjuk gyuruk szamat elejetol kezdve 100 byte utan:
    fwrite(&polygon_count_encrypted, 1, sizeof(polygon_count_encrypted), h);

// Ha belso file-kent mentjuk, most megy kerekek szama:
#ifdef BELSOTIR
    fwrite(&object_count_encrypted, 1, sizeof(object_count_encrypted), h);
#endif

    // Kimentjuk sok gyurut:
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (polygons[i]) {
            polygons[i]->save(h, this);
        }
    }

// Ha kulso file-kent mentjuk, most megy kerekek szama:
#ifndef BELSOTIR
    fwrite(&object_count_encrypted, 1, sizeof(object_count_encrypted), h);
#endif

    // Kimentjuk sok kereket:
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i]) {
            objects[i]->save(h);
        }
    }
    // Kimentjuk spriteokat:
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

    // Kiirjuk ido tablat:
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

// Csak eredetileg beolvasott filenev-vel szabad meghivni:
void level::save_topten(const char* filename) {
    if (topten_file_offset < 6) { // a 6-os talalomra ertek
        internal_error("save_topten cannot be used with internal levels!");
    }

    // Hozzacsapjuk nevhez LEV direktorit is:
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
                           int check_objects_and_sprites) {
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

double level::checksum(int check_sprites) {
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
    if (check_sprites) {
        for (int i = 0; i < MAX_SPRITES; i++) {
            if (sprites[i]) {
                sum += sprites[i]->checksum();
            }
        }
    }

    return CHECKSUM_MULTIPLIER * sum;
}

// KEREKKEL kapcsolatos play elotti inicializalasok:

// lejatszo.cpp-ben Falg Tag ennek alapjan teszi vissza motorost:
vect2 BikeStartOffset;

// Motor helyzetet is o allitja be, kajaszamot adja vissza:
// Eheto kerekeket aktivizalja:
int level::initialize_objects(motorst* mot) {
    int apple_count = 0;
    int start_found = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* obj = Ptop->objects[i];
        if (obj) {
            obj->floating_phase = random_range(1000) * 2.0 * PI / 1000.0;

            obj->active = true;
            if (obj->type == object::Type::Food) {
                apple_count++;
            }
            if (obj->type == object::Type::Start) {
                if (start_found) {
                    internal_error("Level can only have one Start object!");
                }
                start_found = 1;
                obj->active = false;
                BikeStartOffset = obj->r - mot->left_wheel.r;
                mot->bike.r = mot->bike.r + BikeStartOffset;
                mot->left_wheel.r = mot->left_wheel.r + BikeStartOffset;
                mot->right_wheel.r = mot->right_wheel.r + BikeStartOffset;
                mot->body_r = mot->body_r + BikeStartOffset;
                // Fejr = Kor1.r;
            }
        }
    }
    if (!start_found) {
        internal_error("Start object not found in level file!");
    }
    return apple_count;
}

// Kerekeket rendezi:
void level::sort_objects(void) {
    int count = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* obj = Ptop->objects[i];
        if (obj) {
            count++;
        }
    }
    for (int i = 0; i < count; i++) {
        object* obj = Ptop->objects[i];
        if (!obj) {
            internal_error("level::sort_objects has a gap!");
        }
    }

    if (count < 2) {
        internal_error("level::sort_objects count < 2!");
    }
    for (int j = 0; j < count + 4; j++) {
        for (int i = 0; i < count - 1; i++) {
            object::Type type1 = Ptop->objects[i]->type;
            object::Type type2 = Ptop->objects[i + 1]->type;

            // Sullyozzuk tipusokat hogy sorrendiseget lehessen megallapitani:
            int score1 = 10;
            if (type1 == object::Type::Killer) {
                score1 = 1;
            }
            if (type1 == object::Type::Food) {
                score1 = 2;
            }
            if (type1 == object::Type::Exit) {
                score1 = 3;
            }

            int score2 = 10;
            if (type2 == object::Type::Killer) {
                score2 = 1;
            }
            if (type2 == object::Type::Food) {
                score2 = 2;
            }
            if (type2 == object::Type::Exit) {
                score2 = 3;
            }

            if (score1 > score2) {
                // Cserelnunk kell:
                object tmp = *Ptop->objects[i];
                *Ptop->objects[i] = *Ptop->objects[i + 1];
                *Ptop->objects[i + 1] = tmp;
            }
        }
    }
}

object* level::get_object(int index) {
    if (index < 0 || index >= MAX_OBJECTS) {
        internal_error("level::get_object index < 0 || index >= MAX_OBJECTS!");
    }
    object* obj = Ptop->objects[index];
    if (!obj) {
        internal_error("level::get_object !obj!");
    }
    return obj;
}

// kerekekfejjellefele;

void level::flip_objects(void) {
    if (objects_flipped) {
        return;
    }
    objects_flipped = 1;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* obj = Ptop->objects[i];
        if (obj) {
            obj->r.y = -obj->r.y;
        }
    }
}

void level::unflip_objects(void) {
    if (!objects_flipped) {
        return;
    }
    objects_flipped = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* obj = Ptop->objects[i];
        if (obj) {
            obj->r.y = -obj->r.y;
        }
    }
}
