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

#define IDOEGYEDISZAM1 (6754362)
#define IDOEGYEDISZAM2 (8674642)

// Ertekeik kommentkent kint is vannak:

#define BELYEGSZORZO (3247.764325643)

// Ha define-olva van, save belso lev formatumban, .leb kiterjesztessel
// ment mintha shareware lenne, plusz hakellmentbelleveket fuggveny
// osszes levelt igy ki is menti:
// #define BELSOTIR

static char Levelnevek[STATE_LEVEL_COUNT + 2][14] = {
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

int Leveldescinited = 0;
#define MAXBEEPLEVELSZAM (55)
#define DESCNEVHOSSZ (30)
char Leveldescek[(DESCNEVHOSSZ + 2) * MAXBEEPLEVELSZAM + 6];

static char Sor[110];

// Van ugyanilyen neven state.cpp-ben is ket fv.:

// Sikerest adja vissza:
static int titkosread(void* mut, int hossz, FILE* h) {
    if (fread(mut, 1, hossz, h) != hossz) {
        return 0;
    }
    unsigned char* pc = (unsigned char*)mut;
    short a = 21, b = 9783, c = 3389;

    for (int i = 0; i < hossz; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
    return 1;
}

// Sikerest adja vissza:
static int titkoswrite(void* mut, int hossz, FILE* h) {
    unsigned char* pc = (unsigned char*)mut;
    short a = 21, b = 9783, c = 3389;

    for (int i = 0; i < hossz; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
    if (fwrite(mut, 1, hossz, h) != hossz) {
        return 0;
    }
    a = 21, b = 9783, c = 3389;

    for (int i = 0; i < hossz; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
    return 1;
}

const char* get_internal_level_name(int index) {
    if (!Leveldescinited) {
        Leveldescinited = 1;
        FILE* h = qopen("desclist.txt", "r");
        if (!h) {
            internal_error("desclist.txt missing from resource file!");
        }

        for (int i = 0; i < MAXBEEPLEVELSZAM; i++) {
            if (!fgets(Sor, 100, h)) {
                internal_error("7de2y7");
            }
            if (strchr(Sor, '\n')) {
                *strchr(Sor, '\n') = 0;
            }
            if (strchr(Sor, '\r')) {
                *strchr(Sor, '\r') = 0;
            }
            if (strlen(Sor) > DESCNEVHOSSZ) {
                internal_error("tct86");
            }
            strcpy(&Leveldescek[i * (DESCNEVHOSSZ + 2)], Sor);
        }

        qclose(h);
    }
    if (index < 0 || index >= MAXBEEPLEVELSZAM) {
        internal_error("8u9r5tg");
    }
    if (index == 30) { // Animal Farm
        return "Animal Farm";
    }
    return &Leveldescek[index * (DESCNEVHOSSZ + 2)];
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
    int volttorles = 0;
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (sprites[i]) {
            sprite* psp = sprites[i];
            // Default meret:
            psp->wireframe_width = PixelsToMeters * DEFAULT_SPRITE_WIREFRAME;
            psp->wireframe_height = PixelsToMeters * DEFAULT_SPRITE_WIREFRAME;

            if (psp->picture_name[0] && (psp->mask_name[0] || psp->texture_name[0])) {
                internal_error("discard_missing_lgr_assets-ben sok nev egy spriteban!");
            }
            if (psp->picture_name[0]) {
                int index = lgr->getkepindex(psp->picture_name);
                if (index < 0) {
                    psp->picture_name[0] = 0;
                    // Toroljuk:
                    delete sprites[i];
                    sprites[i] = NULL;
                    volttorles = 1;
                    continue;
                }
                // Megvan kep:
                psp->wireframe_width = lgr->kepek[index].xsize;
                psp->wireframe_height = lgr->kepek[index].ysize;
                psp->wireframe_width *= PixelsToMeters;
                psp->wireframe_height *= PixelsToMeters;
            } else {
                // Nem kep sprite:
                if (psp->mask_name[0]) {
                    int index = lgr->getmaszkindex(psp->mask_name);
                    if (index < 0) {
                        psp->mask_name[0] = 0;
                        // Toroljuk:
                        delete sprites[i];
                        sprites[i] = NULL;
                        volttorles = 1;
                    } else {
                        psp->wireframe_width = lgr->maszkok[index].xsize;
                        psp->wireframe_height = lgr->maszkok[index].ysize;
                        psp->wireframe_width *= PixelsToMeters;
                        psp->wireframe_height *= PixelsToMeters;
                    }
                }
                if (psp->texture_name[0]) {
                    int index = lgr->gettexturaindex(psp->texture_name);
                    if (index < 0) {
                        psp->texture_name[0] = 0;
                        // Toroljuk:
                        delete sprites[i];
                        sprites[i] = NULL;
                        volttorles = 1;
                    }
                }
            }
        }
    }

    // Most meg spritetomb elejere kompaktaljuk spriteokat:
    int voltvaltozas = 1;
    while (voltvaltozas) {
        voltvaltozas = 0;
        for (int i = 0; i < MAX_SPRITES - 1; i++) {
            if (!sprites[i] && sprites[i + 1]) {
                // Kovetkezo mutatot ide hozzuk:
                sprites[i] = sprites[i + 1];
                sprites[i + 1] = NULL;
                voltvaltozas = 1;
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
        internal_error("8u9yr");
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
    return volttorles;
}

polygon* level::get_closest_vertex(double x, double y, int* vertex_index, double* distance,
                                   polygon* skip) {
    if (distance) {
        *distance = 1000000000.0;
    }

    double tav = 1000000.0;
    polygon* pgy = NULL;
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

        int ujk;
        double ujtav = polygons[i]->get_closest_vertex(x, y, &ujk);
        if (ujtav < tav) {
            tav = ujtav;
            *vertex_index = ujk;
            pgy = polygons[i];
        }
    }
    if (tav > max_grab_distance()) {
        *vertex_index = 0;
        return NULL;
    }
    if (distance) {
        *distance = tav;
    }
    return pgy;
}

object* level::get_closest_object(double x, double y, double* distance) {
    if (distance) {
        *distance = 1000000000.0;
    }

    double tav = 1000000.0;
    object* pker = NULL;
    vect2 r(x, y);
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i]) {
            double ujtav = (objects[i]->r - r).length();
            if (ujtav < tav) {
                tav = ujtav;
                pker = objects[i];
            }
        }
    }
    if (tav > max_grab_distance()) {
        return NULL;
    }
    if (distance) {
        *distance = tav;
    }
    return pker;
}

sprite* level::get_closest_sprite(double x, double y, double* distance) {
    if (distance) {
        *distance = 1000000000.0;
    }

    double tav = 1000000.0;
    sprite* psp = NULL;
    vect2 r(x, y);
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (!Rajzolkepek) {
            continue;
        }
        if (!sprites[i]) {
            continue;
        }

        double ujtav = (sprites[i]->r - r).length();
        if (ujtav < tav) {
            tav = ujtav;
            psp = sprites[i];
        }
    }
    if (tav > max_grab_distance()) {
        return NULL;
    }
    if (distance) {
        *distance = tav;
    }
    return psp;
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
        internal_error("level::is_sky-ben paramok nem stimmelnek!");
    }

    vect2 r;
    if (poly) {
        r = poly->vertices[0]; // poly->center();
    } else {
        r = *point;
    }
    vect2 v = vect2(27654.475374565578, 37850.5364775);
    // Vegigmegyunk az egeszen:
    int metszes = 0;
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (polygons[i] && !polygons[i]->is_grass && polygons[i] != poly) {
            metszes += polygons[i]->count_intersections(r, v);
        }
    }
    if (metszes % 2) {
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

    const char* fut = &filename[5];
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum *= 10;
        int add = *fut - '0';
        if (add < 0 || add > 9) {
            return 0;
        }
        sum += add;
        fut++;
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
    int belsopalyaszam = get_internal_index(filename);
    if (belsopalyaszam > 0) {
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
        internal_error("4534516");
    }

    int belsopalyaszam = get_internal_index(filename);
    topten_set* idok = NULL;
    if (belsopalyaszam > 0) {
        belsopalyaszam--; // Mivel eggyel nagyobb tomb indexenel
                          // Belso palya:
        if (belsopalyaszam >= INTERNAL_LEVEL_COUNT) {
            internal_error("67467524");
        }

        idok = &State->toptens[belsopalyaszam];
    } else {
        // Kulso palya:
        if (!Ptop) {
            internal_error("78236");
        }
        idok = &Ptop->toptens;
    }

    topten* egyfeleidok = NULL;
    if (single) {
        egyfeleidok = &idok->single;
    } else {
        egyfeleidok = &idok->multi;
    }

    if (egyfeleidok->times_count > 0) {
        centiseconds_to_string(egyfeleidok->times[0], BestTime);
    } else {
        BestTime[0] = 0;
    }
}

// Elso palya indexe 1:
static char* Lgrtabla[110] = {NULL, NULL}; // 1-es indexut figyeli hogy init-e

static char Szo[50] = "";

static void makelgrtabla(void) {
    if (INTERNAL_LEVEL_COUNT > 105) {
        internal_error("uierty895");
    }

    for (int i = 0; i < 110; i++) {
        Lgrtabla[i] = NULL;
    }

    static char* chartomb = NULL;

    chartomb = new char[10 * (INTERNAL_LEVEL_COUNT + 2)];
    if (!chartomb) {
        internal_error("memory");
    }

    for (int i = 1; i <= INTERNAL_LEVEL_COUNT; i++) {
        Lgrtabla[i] = &chartomb[10 * (i - 1)];
        strcpy(Lgrtabla[i], "default");
    }

    // Most fel van toltve default ertekekkel, beolvassuk file-t:
    FILE* h = fopen("lgr/lgrlist.txt", "r");
    if (!h) {
        return;
    }

    while (1) {
        // Beolvasunk egy sort:
        if (!fgets(Sor, 40, h)) {
            break;
        }
        int levelszam = 0;
        if (sscanf(Sor, "%d%s", &levelszam, Szo) != 2) {
            continue;
        }
        if (levelszam < 1 || levelszam > INTERNAL_LEVEL_COUNT) {
            continue;
        }
        strlwr(Szo);
        if (!strstr(Szo, ".lgr")) {
            continue;
        }
        *strstr(Szo, ".lgr") = 0;
        char* szo = Szo;
        // Levagjuk elejerol space-eket es tabokat:
        while ((*szo == ' ') || (*szo == 9)) {
            szo++;
        }
        if ((strlen(szo) > 8) || (strlen(szo) < 1)) {
            continue;
        }
        strcpy(Lgrtabla[levelszam], szo);
    }

    fclose(h);
}

// QWQUU001 elso palya:
level::level(const char* filename) {
    if (!Lgrtabla[1]) {
        makelgrtabla();
    }

    int kulso = 1;

    char filenev[21];
    if (strlen(filename) > 20) {
        internal_error("7y537yf");
    }
    strcpy(filenev, filename);

    int belsopalyaszam = get_internal_index(filenev);
    if (belsopalyaszam > 0) {
        kulso = 0;
        strcpy(filenev, Levelnevek[belsopalyaszam]);
    }

    // Innentol kezdve ket kulon rutin fogja beolvasni kulso es belso file-okat
    // a kulso beolvaso a jo, a belso beolvaso egy kicsit meg van valtoztatva
    // es #define BELSOTIR eseten igy menti ki file-t topol.

    // loadbelso beolvassa kerekek szamat is sok gyuru elott, mig kulso
    // beolvassa sok gyurut es csak utana olvassa kerekek szamat es kerekeket

    if (kulso) {
        load_external(filenev);
    } else {
        load_internal(filenev);
        // Sajat tablabol vesszuk lgr neveket:
        // #ifdef DEBUG 		Demo miatt most nem ellenorizzuk
        //	if( belsopalyaszam < 0 || belsopalyaszam > INTERNAL_LEVEL_COUNT )
        //		internal_error( "4i4fyuif" );
        // #endif
        char tmp[40];
        sprintf(tmp, "lgr/%s.lgr", Lgrtabla[belsopalyaszam]);
        if (access(tmp, 0) == 0) { // Csak ha letezik file:
            strcpy(lgr_name, Lgrtabla[belsopalyaszam]);
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
    char tmpnev[40];
    sprintf(tmpnev, "lev/%s", filename);
    h = fopen(tmpnev, "rb");
    if (!h) {
        internal_error("level::level-ban nem tudta nyitni file-t!: ", tmpnev);
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
        internal_error("Nem tudott olvasni txt file-bol!");
    }
    if (strncmp(tmp, "POT", 3) != 0) {
        internal_error("level::level( char* filename )-ben nem stimmel eleje!");
    }

    level_id = 0;
    double belyeg_d = 0;
    int verzio = tmp[4] - '0' + 10 * (tmp[3] - '0');

    if (verzio > 14) {
        external_error("Level file's version is too new!: ", filename);
    }

    // Most csak ezt a ket verziot tudja olvasni, de eleg kikommentezni
    // hogy mindet olvassa:
    if (verzio != 6 && verzio != 14) {
        external_error("Corrupt level file!: ", filename);
    }

    // verzio 6: 1.2-es verzio levele
    // verzio 7: uj verzio
    // verzio 8: uj verzio kovetovel es uj gyurukkel( maszk, texturanev)
    // verzio 10: benne van idotabla is file vegen
    // verzio 11: kerek-ben benne van foodsorszam ertek is
    // verzio 12: uj koveto (csak koveto tenye van tarolva gyuru-ben)
    // verzio 13: plusz 2 byte-ot beolvas meg belyeg elott
    // verzio 14: LEVELNAME (14)-rol (50)-re emelkedik, nem seek-el 100-ra

    int dummy;
    if (verzio >= 13) {
        if (fread(&dummy, 1, 2, h) != 2) {
            internal_error("Nem tudott olvasni lev file-bol!");
        }
    }

    // Belyeg:
    if (fread(&level_id, 1, 4, h) != 4) {
        internal_error("Nem tudott olvasni lev file-bol!");
    }

    if (fread(&belyeg_d, 1, sizeof(belyeg_d), h) != sizeof(double)) {
        internal_error("Nem tudott olvasni lev file-bol double belyeget!");
    }

    // Beolvassuk hogy shareware-e:
    double sarvarertek;
    if (fread(&sarvarertek, 1, sizeof(sarvarertek), h) != sizeof(sarvarertek)) {
        internal_error("Nem tudott olvasni lev file-bol!");
    }
    if (sarvarertek + belyeg_d < 9786.0 || sarvarertek + belyeg_d > 36546.0) {
        internal_error("level::level-ban sarvarertek kivul esik tartomanyon!");
    }

    // Levels that can only be played in Registered would meet the condition: sarvarertek + belyeg_d
    // <= 20000.0

    // Beolvassuk hogy hibas-e:
    double hibaertek;
    if (fread(&hibaertek, 1, sizeof(hibaertek), h) != sizeof(hibaertek)) {
        internal_error("Nem tudott olvasni lev file-bol!");
    }
    if (hibaertek + belyeg_d < 9786.0 || hibaertek + belyeg_d > 36546.0) {
        internal_error("level::level-ban hibaertek kivul esik tartomanyon!");
    }
    if (hibaertek + belyeg_d > 20000.0) {
        topology_errors = 1;
    }

    // Beolvassuk hogy lezart-e:
    double lezartertek;
    if (fread(&lezartertek, 1, sizeof(lezartertek), h) != sizeof(lezartertek)) {
        internal_error("Nem tudott olvasni lev file-bol!");
    }
    if (lezartertek + belyeg_d < 9875.0 || lezartertek + belyeg_d > 32345.0) {
        internal_error("level::level-ban lezartertek kivul esik tartomanyon!");
    }
    if (lezartertek + belyeg_d > 20000.0) {
        locked = 1;
    }

    // lezart = 0; // Ha ez nincs kikomentezve, unlockol minden file-t

    // Beolvassuk level nevet:
    int tmpnamehossz = LEVEL_NAME_LENGTH;
    if (verzio < 14) {
        tmpnamehossz = 14;
    }
    fread(level_name, 1, tmpnamehossz + 1, h);
    level_name[tmpnamehossz] = 0; // Ez csak biztonsagert

    // Beolvassuk lgr nevet:
    if (verzio > 6) {
        fread(lgr_name, 1, 16, h);
        lgr_name[15] = 0; // Ez csak biztonsagert
    } else {
        strcpy(lgr_name, "default");
    }

    if (verzio >= 8) {
        fread(foreground_name, 1, 10, h);
        foreground_name[9] = 0; // Ez csak biztonsagert
        fread(background_name, 1, 10, h);
        foreground_name[9] = 0; // Ez csak biztonsagert
    } else {
        strcpy(foreground_name, "ground");
        strcpy(background_name, "sky");
    }

    // Gyuruk szamat 100 byte utan kezdjuk:
    if (verzio < 14) {
        ////if( kulso )
        fseek(h, 100, SEEK_SET);
    }
    ////else
    ////    qseek( h, 100, SEEK_SET );

    double gyuruolvszam = 0;
    if (fread(&gyuruolvszam, 1, sizeof(gyuruolvszam), h) != sizeof(gyuruolvszam)) {
        internal_error("level::level-ban nem tudta olvasni lebego gyuruszamot!");
    }

    int gyuruszam = (int)(gyuruolvszam);
    if (gyuruszam > MAX_POLYGONS) {
        internal_error("level::level-ban gyuruszam > MAX_POLYGONS!: ");
    }
    if (gyuruszam <= 0) {
        internal_error("level::level-ban gyuruszam <= 0!");
    }

    // Beolvassa sok gyurut:
    for (int i = 0; i < gyuruszam; i++) {
        polygons[i] = new polygon(h, verzio);
    }

    // Kerekek szama:
    double kerekolvszam = 0;
    if (fread(&kerekolvszam, 1, sizeof(kerekolvszam), h) != sizeof(kerekolvszam)) {
        internal_error("level::level-ban nem tudta olvasni lebego kerekszamot!");
    }

    int kerekszam = (int)(kerekolvszam);
    if (kerekszam > MAX_OBJECTS) {
        internal_error("level::level-ban kerekszam > MAX_OBJECTS!: ");
    }
    if (kerekszam <= 0) {
        internal_error("level::level-ban kerekszam <= 0!");
    }

    for (int i = 0; i < kerekszam; i++) {
        // Beolvassa sok kereket:
        objects[i] = new object(h, verzio);
    }

    if (verzio > 6) {
        // Most van SPRITE beolvasas is:
        // Kerekek szama:
        double spriteolvszam = 0;
        if (fread(&spriteolvszam, 1, sizeof(spriteolvszam), h) != sizeof(spriteolvszam)) {
            internal_error("level::level-ban nem tudta olvasni lebego spriteszamot!");
        }

        int spriteszam = (int)(spriteolvszam);
        if (spriteszam > MAX_SPRITES) {
            internal_error("level::level-ban spriteszam > MAX_SPRITES!: ");
        }
        if (spriteszam < 0) {
            internal_error("level::level-ban spriteszam < 0!");
        }

        if (verzio < 8 && spriteszam > 0) {
            internal_error("verzio < 8 && spriteszam > 0!", filename);
        }

        for (int i = 0; i < spriteszam; i++) {
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
        internal_error("dddhhhew");
    }

    int tmpszam = 0;
    if (fread(&tmpszam, 1, 4, h) != 4 || tmpszam != IDOEGYEDISZAM1) {
        // if( verzio >= 10 )
        //   external_error( "Corrupt file:", filenev );
    } else {
        if (!titkosread(&toptens, sizeof(toptens), h) || fread(&tmpszam, 1, 4, h) != 4 ||
            tmpszam != IDOEGYEDISZAM2) {
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
    double szamitottbelyeg = checksum(verzio > 6);
    if (fabs(szamitottbelyeg - belyeg_d) > 0.01) {
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
        internal_error("level::level-ban nem tudta nyitni file-t!: ", filename);
    }

    // Verzio beolvasasa:
    char tmp[10] = "AAAAA";
    if (fread(tmp, 1, 5, h) != 5) {
        internal_error("Nem tudott olvasni txt file-bol!");
    }
    if (strncmp(tmp, "@@^!@", 3) != 0) {
        internal_error("level::level( char* filename )-ben nem stimmel eleje!");
    }

    level_id = 0;
    double belyeg_d = 0;
    /*int verzio = tmp[4] - '0' + 10*(tmp[3] - '0');

    if( verzio < 6 )
        external_error( "Corrupt level file!: ", filename );

    if( verzio > 14 )
        external_error( "Level file's version is too new!: ", filename );
    */
    int verzio = 14; // Mivel leb file-okat mindig igy mentjuk

    // verzio 6: 1.2-es verzio levele
    // verzio 7: uj verzio
    // verzio 8: uj verzio kovetovel es uj gyurukkel( maszk, texturanev)
    // verzio 10: benne van idotabla is file vegen
    // verzio 11: kerek-ben benne van foodsorszam ertek is
    // verzio 12: uj koveto (csak koveto tenye van tarolva gyuru-ben)
    // verzio 13: plusz 2 byte-ot beolvas meg belyeg elott
    // verzio 14: LEVELNAME (14)-rol (50)-re emelkedik, nem seek-el 100-ra

    int dummy;
    if (verzio >= 13) {
        if (fread(&dummy, 1, 2, h) != 2) {
            internal_error("Nem tudott olvasni lev file-bol!");
        }
    }

    // Belyeg:
    if (fread(&level_id, 1, 4, h) != 4) {
        internal_error("Nem tudott olvasni lev file-bol!");
    }

    if (fread(&belyeg_d, 1, sizeof(belyeg_d), h) != sizeof(double)) {
        internal_error("Nem tudott olvasni lev file-bol double belyeget!");
    }

    // Beolvassuk hogy shareware-e:
    double sarvarertek;
    if (fread(&sarvarertek, 1, sizeof(sarvarertek), h) != sizeof(sarvarertek)) {
        internal_error("Nem tudott olvasni lev file-bol!");
    }
    if (sarvarertek + belyeg_d < 9786.0 || sarvarertek + belyeg_d > 36546.0) {
        internal_error("level::level-ban sarvarertek kivul esik tartomanyon!");
    }

    // Beolvassuk hogy hibas-e:
    double hibaertek;
    if (fread(&hibaertek, 1, sizeof(hibaertek), h) != sizeof(hibaertek)) {
        internal_error("Nem tudott olvasni lev file-bol!");
    }
    if (hibaertek + belyeg_d < 9786.0 || hibaertek + belyeg_d > 36546.0) {
        internal_error("level::level-ban hibaertek kivul esik tartomanyon!");
    }
    if (hibaertek + belyeg_d > 20000.0) {
        topology_errors = 1;
    }

    // Beolvassuk hogy lezart-e:
    double lezartertek;
    if (fread(&lezartertek, 1, sizeof(lezartertek), h) != sizeof(lezartertek)) {
        internal_error("Nem tudott olvasni lev file-bol!");
    }
    if (lezartertek + belyeg_d < 9875.0 || lezartertek + belyeg_d > 32345.0) {
        internal_error("level::level-ban lezartertek kivul esik tartomanyon!");
    }
    if (lezartertek + belyeg_d > 20000.0) {
        locked = 1;
    }

    // if( lezart ) {
    //  Ez csak debug-hoz:
    //  dialog( "Le volt zarva!" );
    //}

    // lezart = 0; // Ha ez nincs kikomentezve, unlockol minden file-t

    // Beolvassuk level nevet:
    int tmpnamehossz = LEVEL_NAME_LENGTH;
    if (verzio < 14) {
        tmpnamehossz = 14;
    }
    fread(level_name, 1, tmpnamehossz + 1, h);
    level_name[tmpnamehossz] = 0; // Ez csak biztonsagert

    // Beolvassuk lgr nevet:
    if (verzio > 6) {
        fread(lgr_name, 1, 16, h);
        lgr_name[15] = 0; // Ez csak biztonsagert
    } else {
        strcpy(lgr_name, "default");
    }

    if (verzio >= 8) {
        fread(foreground_name, 1, 10, h);
        foreground_name[9] = 0; // Ez csak biztonsagert
        fread(background_name, 1, 10, h);
        background_name[9] = 0; // Ez csak biztonsagert
    } else {
        strcpy(foreground_name, "ground");
        strcpy(background_name, "sky");
    }

    // Gyuruk szamat 100 byte utan kezdjuk:
    if (verzio < 14) {
        qseek(h, 100, SEEK_SET);
    }
    double gyuruolvszam = 0;
    if (fread(&gyuruolvszam, 1, sizeof(gyuruolvszam), h) != sizeof(gyuruolvszam)) {
        internal_error("level::level-ban nem tudta olvasni lebego gyuruszamot!");
    }

    int gyuruszam = (int)(gyuruolvszam);
    if (gyuruszam > MAX_POLYGONS) {
        internal_error("level::level-ban gyuruszam > MAX_POLYGONS!: ");
    }
    if (gyuruszam <= 0) {
        internal_error("level::level-ban gyuruszam <= 0!");
    }

    // Kerekek szama:
    double kerekolvszam = 0;
    if (fread(&kerekolvszam, 1, sizeof(kerekolvszam), h) != sizeof(kerekolvszam)) {
        internal_error("level::level-ban nem tudta olvasni lebego kerekszamot!");
    }

    int kerekszam = (int)(kerekolvszam);
    if (kerekszam > MAX_OBJECTS) {
        internal_error("level::level-ban kerekszam > MAX_OBJECTS!: ");
    }
    if (kerekszam <= 0) {
        internal_error("level::level-ban kerekszam <= 0!");
    }

    // Beolvassa sok gyurut:
    for (int i = 0; i < gyuruszam; i++) {
        polygons[i] = new polygon(h, verzio);
    }

    // Beolvassa sok kereket:
    for (int i = 0; i < kerekszam; i++) {
        objects[i] = new object(h, verzio);
    }

    if (verzio > 6) {
        // Most van SPRITE beolvasas is:
        // Kerekek szama:
        double spriteolvszam = 0;
        if (fread(&spriteolvszam, 1, sizeof(spriteolvszam), h) != sizeof(spriteolvszam)) {
            internal_error("level::level-ban nem tudta olvasni lebego spriteszamot!");
        }

        int spriteszam = (int)(spriteolvszam);
        if (spriteszam > MAX_SPRITES) {
            internal_error("level::level-ban spriteszam > MAX_SPRITES!: ");
        }
        if (spriteszam < 0) {
            internal_error("level::level-ban spriteszam < 0!");
        }

        if (verzio < 8 && spriteszam > 0) {
            internal_error("verzio < 8 && spriteszam > 0!", filename);
        }

        for (int i = 0; i < spriteszam; i++) {
            // Beolvassa sok kereket:
            sprites[i] = new sprite(h);
        }
    }

    // Ezekutan mar csak idotabla lehet, de mivel belso file, az sincs.
    qclose(h);

    // Most megnezzuk, hogy belyeg stimmel-e:
    double szamitottbelyeg = checksum(verzio > 6);
    if (fabs(szamitottbelyeg - belyeg_d) > 0.01) {
        external_error("Corrupt .LEV file!: ", filename);
    }
}

static long longosit(double belyeg) {
    srand(clock());
    unsigned long veletlen = random_range(6542);
    veletlen *= random_range(7042);
    veletlen += random_range(4542);
    veletlen *= random_range(3042);
    veletlen *= random_range(3742);
    veletlen += random_range(9187);

    belyeg = sin(belyeg);
    belyeg *= (belyeg + 1.0001) * 40000;
    unsigned long l = belyeg;

    unsigned long belyeg_l = (l & 0x0000ffff) | (veletlen & 0xffff0000);
    return belyeg_l;
}

void level::save(const char* filename, int skip_topology) {
    memset(&toptens, 0, sizeof(toptens));
    // if( lezart )
    //   internal_error( "level::save-ben lezart!" );

    Volttopsave = 1; // load.cpp-nek kell ez
    if (objects_flipped) {
        internal_error("level::save-ben objects_flipped!");
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

    char tmpnev[40];
    sprintf(tmpnev, "lev/%s", filename);
#ifdef BELSOTIR
    if (!strstr(tmpnev, ".lev") && !strstr(tmpnev, ".LEV")) {
        internal_error("78t4678r4");
    }
    if (strstr(tmpnev, ".lev")) {
        strcpy(strstr(tmpnev, ".lev"), ".leb");
    }
    if (strstr(tmpnev, ".LEV")) {
        strcpy(strstr(tmpnev, ".LEV"), ".leb");
    }
#endif
    FILE* h = fopen(tmpnev, "wb");
    if (!h) {
        internal_error("level::save-ben nem tudta nyitni file-t!: ", tmpnev);
    }

#ifdef BELSOTIR
    fwrite("@@^!@", 1, 5, h);
#else
    fwrite("POT14", 1, 5, h);
#endif

    // Kiszamitjuk es kiirjuk ketfele belyeget:
    double belyeg_d = checksum(1);
    level_id = longosit(belyeg_d);

    fwrite(&level_id, 1, 2, h); // Kiirunk ket tetszoleges byte-ot

    fwrite(&level_id, 1, 4, h);
    fwrite(&belyeg_d, 1, sizeof(belyeg_d), h);

    // Kiirjuk hogy sarvari-e:
    // Shareware levels would meet the criteria sarvarertek + double(belyeg_l) > 20000.0
    // which originally was calculated as 20961.0 + random_range( 4982 ) - belyeg_d;
    double sarvarertek = 11877.0 + random_range(5871) - belyeg_d;

// Regisztralt verzioju file-t is shareware-nek tunteti fol:
#ifdef BELSOTIR
    sarvarertek = 20961.0 + random_range(4982) - belyeg_d;
#endif

    fwrite(&sarvarertek, 1, sizeof(sarvarertek), h);

    // Kiirjuk hogy hibas-e:
    // Ha hibaertek + double(belyeg_l) > 20000.0, akkor hibas:
    double hibaertek = 11877.0 + random_range(5871) - belyeg_d;
    if (topology_errors) { // ezt mar elejen meghataroztuk
        hibaertek = 20961.0 + random_range(4982) - belyeg_d;
    }
    fwrite(&hibaertek, 1, sizeof(hibaertek), h);

    // Kiirjuk hogy lezart-e:
    double lezartertek = 12112.0 + random_range(6102) - belyeg_d;
    if (locked) {
        lezartertek = 23090.0 + random_range(6310) - belyeg_d;
    }
    fwrite(&lezartertek, 1, sizeof(hibaertek), h);

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

    int gyuruszam = 0;
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (polygons[i]) {
            gyuruszam++;
        }
    }
    double gyuruirtszam = gyuruszam + 0.4643643;

    int kerekszam = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i]) {
            kerekszam++;
        }
    }
    double kerekirtszam = kerekszam + 0.4643643;

    // Kimentjuk gyuruk szamat elejetol kezdve 100 byte utan:
    fwrite(&gyuruirtszam, 1, sizeof(gyuruirtszam), h);

// Ha belso file-kent mentjuk, most megy kerekek szama:
#ifdef BELSOTIR
    fwrite(&kerekirtszam, 1, sizeof(kerekirtszam), h);
#endif

    // Kimentjuk sok gyurut:
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (polygons[i]) {
            polygons[i]->save(h, this);
        }
    }

// Ha kulso file-kent mentjuk, most megy kerekek szama:
#ifndef BELSOTIR
    fwrite(&kerekirtszam, 1, sizeof(kerekirtszam), h);
#endif

    // Kimentjuk sok kereket:
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i]) {
            objects[i]->save(h);
        }
    }
    // Kimentjuk spriteokat:
    int spriteszam = 0;
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (sprites[i]) {
            spriteszam++;
        }
    }
    double spriteirtszam = spriteszam + 0.2345672;
    fwrite(&spriteirtszam, 1, sizeof(spriteirtszam), h);
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (sprites[i]) {
            sprites[i]->save(h);
        }
    }

    // Kiirjuk ido tablat:
    int tmpszo = IDOEGYEDISZAM1;
    fwrite(&tmpszo, 1, 4, h);
    titkoswrite(&toptens, sizeof(toptens), h);
    tmpszo = IDOEGYEDISZAM2;
    fwrite(&tmpszo, 1, 4, h);

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
        internal_error("hjdvgyj");
    }

    // Hozzacsapjuk nevhez LEV direktorit is:
    char filenev[40];
    sprintf(filenev, "lev/%s", filename);

    FILE* h = fopen(filenev, "r+b");
    if (!h) {
        external_error("Could not open file!", filenev);
    }

    if (fseek(h, topten_file_offset, SEEK_SET) != 0) {
        external_error("Could not write to file:", filenev);
    }
    int tmpszo = IDOEGYEDISZAM1;
    if (fwrite(&tmpszo, 1, 4, h) != 4) {
        external_error("Could not write file:", filenev);
    }
    if (!titkoswrite(&toptens, sizeof(toptens), h)) {
        external_error("Could not write file:", filenev);
    }
    tmpszo = IDOEGYEDISZAM2;
    if (fwrite(&tmpszo, 1, 4, h) != 4) {
        external_error("Could not write file:", filenev);
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
    double belyeg = 0.0;
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (polygons[i]) {
            belyeg += polygons[i]->checksum();
        }
    }
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objects[i]) {
            belyeg += objects[i]->checksum();
        }
    }
    if (check_sprites) {
        for (int i = 0; i < MAX_SPRITES; i++) {
            if (sprites[i]) {
                belyeg += sprites[i]->checksum();
            }
        }
    }

    return BELYEGSZORZO * belyeg;
}

// KEREKKEL kapcsolatos play elotti inicializalasok:

// lejatszo.cpp-ben Falg Tag ennek alapjan teszi vissza motorost:
vect2 BikeStartOffset;

// Motor helyzetet is o allitja be, kajaszamot adja vissza:
// Eheto kerekeket aktivizalja:
int level::initialize_objects(motorst* mot) {
    int kajaszam = 0;
    int voltkezdo = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* pker = Ptop->objects[i];
        if (pker) {
            pker->floating_phase = random_range(1000) * 2.0 * PI / 1000.0;

            pker->active = true;
            if (pker->type == object::Type::Food) {
                kajaszam++;
            }
            if (pker->type == object::Type::Start) {
                if (voltkezdo) {
                    internal_error("level::initialize_objects-ban palyan ket kezdo is van!");
                }
                voltkezdo = 1;
                pker->active = false;
                BikeStartOffset = pker->r - mot->left_wheel.r;
                mot->bike.r = mot->bike.r + BikeStartOffset;
                mot->left_wheel.r = mot->left_wheel.r + BikeStartOffset;
                mot->right_wheel.r = mot->right_wheel.r + BikeStartOffset;
                mot->body_r = mot->body_r + BikeStartOffset;
                // Fejr = Kor1.r;
            }
        }
    }
    if (!voltkezdo) {
        internal_error("level::initialize_objects-ban palyan nincs kezdo!");
    }
    return kajaszam;
}

// Kerekeket rendezi:
void level::sort_objects(void) {
    int szam = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* pker = Ptop->objects[i];
        if (pker) {
            szam++;
        }
    }
    for (int i = 0; i < szam; i++) {
        object* pker = Ptop->objects[i];
        if (!pker) {
            internal_error("level::sort_objects-ben hezagos kerek tomb!");
        }
    }

    if (szam < 2) {
        internal_error("level::sort_objects-ben szam < 2!");
    }
    for (int j = 0; j < szam + 4; j++) {
        for (int i = 0; i < szam - 1; i++) {
            object::Type tipus1 = Ptop->objects[i]->type;
            object::Type tipus2 = Ptop->objects[i + 1]->type;

            // Sullyozzuk tipusokat hogy sorrendiseget lehessen megallapitani:
            int tip1 = 10;
            if (tipus1 == object::Type::Killer) {
                tip1 = 1;
            }
            if (tipus1 == object::Type::Food) {
                tip1 = 2;
            }
            if (tipus1 == object::Type::Exit) {
                tip1 = 3;
            }

            int tip2 = 10;
            if (tipus2 == object::Type::Killer) {
                tip2 = 1;
            }
            if (tipus2 == object::Type::Food) {
                tip2 = 2;
            }
            if (tipus2 == object::Type::Exit) {
                tip2 = 3;
            }

            if (tip1 > tip2) {
                // Cserelnunk kell:
                object tmpker = *Ptop->objects[i];
                *Ptop->objects[i] = *Ptop->objects[i + 1];
                *Ptop->objects[i + 1] = tmpker;
            }
        }
    }
}

object* level::get_object(int index) {
    if (index < 0 || index >= MAX_OBJECTS) {
        internal_error("level::get_object-ben index < 0 || index >= MAX_OBJECTS!");
    }
    object* pker = Ptop->objects[index];
    if (!pker) {
        internal_error("level::get_object-ben !pker!");
    }
    return pker;
}

// kerekekfejjellefele;

void level::flip_objects(void) {
    if (objects_flipped) {
        return;
    }
    objects_flipped = 1;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* pker = Ptop->objects[i];
        if (pker) {
            pker->r.y = -pker->r.y;
        }
    }
}

void level::unflip_objects(void) {
    if (!objects_flipped) {
        return;
    }
    objects_flipped = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* pker = Ptop->objects[i];
        if (pker) {
            pker->r.y = -pker->r.y;
        }
    }
}
