#ifndef RECORDER_H
#define RECORDER_H

#include "sound_engine.h"
#include "vect2.h"
#include <cstdio>

struct motorst;

struct bike_sound {
    double motor_frequency;
    char gas;
    double friction_volume;
};

struct event {
    double time;
    short object_id;
    WavEvent event_id;
    float volume;
};

int load_replays(const char* filename, int demo);
void save_replays(const char* filename, int level_id, int flagtag);

class recorder {
    // demo eseten resource file-bol nyit:
    // belyeg-et adja vissza, vagy 0-at:
    friend int load_replays(const char* filename, int demo);
    friend void save_replays(const char* filename, int level_id, int flagtag);
    friend void replay(void); // Hogy betolthoz hozzaferjen

    float *bike_x, *bike_y;
    short *left_wheel_x, *left_wheel_y, *right_wheel_x, *right_wheel_y;
    short *body_x, *body_y;
    unsigned char *left_wheel_rotation, *right_wheel_rotation;
    short* bike_rotation;
    unsigned char *motor_frequency, *friction_volume;
    unsigned char* flags;

    int frame_count;
    int finished;

    event* events;
    int event_count;
    int current_event_index;

    vect2 previous_bike_r;
    double previous_frame_time, next_frame_time;
    int next_frame_index;

    int flagtag_;
    // demo eseten resource file-bol nyit:
    // belyeg-et adja vissza, vagy 0-at:
    int load(const char* filename, FILE* h, int demo);
    void save(const char* filename, FILE* h, int level_id, int flagtag);

  public:
    char level_filename[16];

    recorder(void);
    ~recorder(void);
    void erase(char* lev_filename);
    void rewind(void);
    // Ez a ket fuggveny  meg ezt a ket globalis valtozot is
    // beveszi es allitja: FlagTagAHasFlag FlagTagImmunity:
    int recall_frame(motorst* mot, double time, bike_sound* sound);
    void store_frames(motorst* mot, double time, bike_sound* sound);

    void store_event(double time, WavEvent event_id, double volume, int object_id);
    // Ha van hang, akkor igazzal ter vissza:
    int recall_event(double time, WavEvent* event_id, double* volume, int* object_id);

    int flagtag(void);
    void set_flagtag(int flagtag);

    void encode_frame_count();
    bool frame_count_integrity();
};

extern recorder *Rec1, *Rec2;
// Ha multiplayer a bentlevo rec, akkor ez igaz:
// long lejatszo( void ) allithatja, vagy loadrecek:
extern int MultiplayerRec;

void add_event_buffer(WavEvent event_id, double volume, int object_id);
void reset_event_buffer(void);
// Igaz, ha van bent wav:
int get_event_buffer(WavEvent* event_id, double* volume, int* object_id);

#endif
