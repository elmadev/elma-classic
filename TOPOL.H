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
#define LEVEL_NAME_LENGTH (50)

class lgrfile;

class level {
    friend int menu_exit(void);
    friend int floadlevel_e(const char* nev, int forceload);
    friend int floadlevel_p(const char* nev);
    friend void invalidate_ptop(void);

    double checksum(int check_sprites);

    // Create a default level
    level(void);
    // Load level from file
    level(const char* filename);
    ~level(void);
    void load_external(const char* filename);
    void load_internal(const char* filename);

  public:
    long level_id;
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
    int topten_file_offset; // 0 if internal level

    // Delete sprites that don't exist in current lgr, and make sure default ground / sky textures
    // are different.
    //
    // Returns true if any sprites were deleted.
    int discard_missing_lgr_assets(lgrfile* lgr);
    // Get closest vertex to (x, y) in the editor, if the distance is less than 10 pixels. A polygon
    // can `skip` can be passed to skip that polygon.
    polygon* get_closest_vertex(double x, double y, int* vertex_index, double* distance = NULL,
                                polygon* skip = NULL);
    // Get closest object to (x, y) in editor, if the distance is less than 10 pixels.
    object* get_closest_object(double x, double y, double* distance = NULL);
    // Get closest sprite to (x, y) in editor, if the distance is less than 10 pixels.
    sprite* get_closest_sprite(double x, double y, double* distance = NULL);
    void save(const char* filename, int skip_topology = 0);
    // Update top ten of an external level
    void save_topten(const char* filename);
    // Render the level in the internal editor.
    void render(void);
    // Check to see if either a polygon or point is within the sky or ground (only provide one)
    int is_sky(polygon* poly, vect2* point = NULL);
    // Get the min/max dimensions of level, optionally including objects/sprites
    void get_boundaries(double* x1, double* y1, double* x2, double* y2,
                        int check_objects_and_sprites);
    // Returns apple count.
    int initialize_objects(motorst* mot);
    void sort_objects(void);
    object* get_object(int index);

    int objects_flipped;
    // Invert the y of all objects (needs to be inverted for in-game, and uninverted in editor)
    void flip_objects(void);
    void unflip_objects(void);
};

extern vect2 BikeStartOffset;

// Similar to access(). Return 0 if level exists. Internal levels always exist.
int access_level_file(const char* filename);

// Indexed from 0
const char* get_internal_level_name(int index);

extern char BestTime[30];
// Store the best time of the level to display while doing a run
void load_best_time(const char* filename, int single);

// Returns internal level index (1-55), or 0 if external level
int get_internal_index(const char* filename);

#endif
