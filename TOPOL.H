#ifndef TOPOL_H
#define TOPOL_H

#include "state.h"
#include "vect2.h"

class object;
class polygon;
class sprite;
struct motorst;

void set_gyuru_attributes(polygon* pgy);

#define MAX_OBJECTS (52)
constexpr int MAX_VERTICES = 5000;

#define MAX_SPRITES (5000)

#define MAX_POLYGONS (300)

typedef polygon* ptrgyuru;
#define LEVEL_NAME_LENGTH (50) // 13-as verzioig (14) volt

class lgrfile;

class level {
    friend int menu_exit(void);
    friend int floadlevel_e(const char* nev, int forceload);
    friend int floadlevel_p(const char* nev);
    friend void invalidate_ptop(void);

    double checksum(int check_sprites);

    level(void);
    // QWQUU001 elso palya:
    level(const char* filename);
    ~level(void);
    void load_external(const char* filename);
    void load_internal(const char* filename);

  public:
    // Beolvasas utan ervenyes, csak rec file ellenorzesere hasznalando
    long level_id;
    // loadlgrfile allitja 1-be, ha nincs meg lgr es default.lgr-t hasznal:
    int lgr_not_found;

    int topology_errors, locked;
    ptrgyuru polygons[MAX_POLYGONS]; // Ez csak gany miatt public
    object* objects[MAX_OBJECTS];
    sprite* sprites[MAX_SPRITES];
    char level_name[LEVEL_NAME_LENGTH + 1];
    char lgr_name[16];
    char foreground_name[10];
    char background_name[10];

    topten_set toptens;
    int topten_file_offset;

    // Igazzal ter vissza, ha volt valtozas:
    int discard_missing_lgr_assets(lgrfile* lgr);
    polygon* get_closest_vertex(double x, double y, int* vertex_index, double* distance = NULL,
                                polygon* skip = NULL);
    object* get_closest_object(double x, double y, double* distance = NULL);
    sprite* get_closest_sprite(double x, double y, double* distance = NULL);
    // Ha saveidok hivja meg, csak akkor nem kell check:
    void save(const char* filename, int skip_topology = 0);
    // Csak eredetileg beolvasott filenev-vel szabad meghivni:
    void save_topten(const char* filename);
    void render(void);
    // pgy levegoben van-e. Akkor folddarab:
    int is_sky(polygon* poly, vect2* point = NULL);
    void get_boundaries(double* x1, double* y1, double* x2, double* y2,
                        int check_objects_and_sprites);

    // Kerekkel kapcsolatos dolgok:
    // Motor helyzetet is o allitja be, kajaszamot adja vissza:
    int initialize_objects(motorst* mot); // eheto kerekeket aktivizalja
    void sort_objects(void);              // kerekeket rendezi
    object* get_object(int index);

    // Ha igaz, y negalva van (ez kell lejatszasnal):
    int objects_flipped;
    void flip_objects(void);
    void unflip_objects(void);
};

// lejatszo.cpp-ben Falg Tag ennek alapjan teszi vissza motorost:
extern vect2 BikeStartOffset; // setallaktiv allitja be

// Ha letezik file, akkor 0-t ad vissza!:
// Ugyanugy viselkedik nevvel, mint topol konstruktora:
int access_level_file(const char* filename);

// Csak external file-hoz:
// Ha NULL, akkor valami baj volt vele:
// char* getlevelname( char* nev ); mar nem hasznalt

// 0-tol kezdodnek:
const char* get_internal_level_name(int index);

// Belso belyegszamitasokhoz kell:
/*unsigned long double2belyeg( double* px );
unsigned long double2belyeg( long l );*/

extern char BestTime[30];
// Azt a nevet keri, amit topol::topol kap:
void load_best_time(const char* filename, int single);

int get_internal_index(const char* filename);

#endif
