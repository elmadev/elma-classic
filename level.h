#ifndef LEVEL_H
#define LEVEL_H

#include "state.h"
#include "vect2.h"

class lgrfile;
struct motorst;
class object;
class polygon;
class sprite;

constexpr int MAX_POLYGONS = 300;
constexpr int MAX_VERTICES = 5000;
constexpr int MAX_OBJECTS = 52;
constexpr int MAX_SPRITES = 5000;

constexpr int LEVEL_NAME_LENGTH = 50;
constexpr int LEVEL_NAME_LENGTH_OLD = 14;

class level {
    double checksum();
    void from_file(const char* filename, bool internal);

  public:
    int level_id;
    bool lgr_not_found;
    bool topology_errors;
    polygon* polygons[MAX_POLYGONS];
    object* objects[MAX_OBJECTS];
    sprite* sprites[MAX_SPRITES];
    char level_name[LEVEL_NAME_LENGTH + 1];
    char lgr_name[16];
    char foreground_name[10];
    char background_name[10];

    topten_set toptens;
    int topten_file_offset; // 0 if internal level

    // Create a default level
    level();
    // Load level from file
    level(const char* filename);
    ~level();

    // Delete sprites that don't exist in current lgr, and make sure default ground / sky textures
    // are different.
    //
    // Returns true if any sprites were deleted.
    bool discard_missing_lgr_assets(lgrfile* lgr);
    // Get closest vertex to (x, y) in the editor, if the distance is less than 10 pixels. A polygon
    // can `skip` can be passed to skip that polygon.
    polygon* get_closest_vertex(double x, double y, int* vertex_index, double* distance = nullptr,
                                polygon* skip = nullptr);
    // Get closest object to (x, y) in editor, if the distance is less than 10 pixels.
    object* get_closest_object(double x, double y, double* distance = nullptr);
    // Get closest sprite to (x, y) in editor, if the distance is less than 10 pixels.
    sprite* get_closest_sprite(double x, double y, double* distance = nullptr);
    void save(const char* filename, bool skip_topology = false);
    // Update top ten of an external level
    void save_topten(const char* filename);
    // Render the level in the internal editor.
    void render();
    // Check to see if either a polygon or point is within the sky or ground (only provide one)
    bool is_sky(polygon* poly, vect2* point = nullptr);
    // Get the min/max dimensions of level, optionally including objects/sprites
    void get_boundaries(double* x1, double* y1, double* x2, double* y2,
                        bool check_objects_and_sprites);
    // Returns apple count.
    int initialize_objects(motorst* mot);
    void sort_objects();
    object* get_object(int index);

    bool objects_flipped;
    // Invert the y of all objects (needs to be inverted for in-game, and uninverted in editor)
    void flip_objects();
    void unflip_objects();
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
