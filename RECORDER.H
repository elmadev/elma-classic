#ifndef RECORDER_H
#define RECORDER_H

#include "sound_engine.h"
#include "vect2.h"
#include <cstdio>

struct motorst;

struct bike_sound {
    double motor_frequency; // Playback speed of gas sound: 1.0-2.0
    char gas;               // 1 if throttling else 0
    double friction_volume; // Bike squeak volume
};

struct event {
    double time;
    short object_id; // apple being eaten if event_id is none
    WavEvent event_id;
    float volume; // (0.0 - 0.99)
};

// Load a singleplayer or multiplayer replay
int load_replays(const char* filename, int demo);
// Save a singleplayer or multiplayer replay
void save_replays(const char* filename, int level_id, int flagtag);

class recorder {
    friend int load_replays(const char* filename, int demo);
    friend void save_replays(const char* filename, int level_id, int flagtag);
    friend void replay(void);

    float *bike_x, *bike_y;
    short *left_wheel_x, *left_wheel_y, *right_wheel_x, *right_wheel_y;
    short *body_x, *body_y;
    // (0-249, glitched 0-255 when brake-stretching)
    unsigned char *left_wheel_rotation, *right_wheel_rotation;
    short* bike_rotation; // (0-9999)
    unsigned char *motor_frequency, *friction_volume;
    unsigned char* flags;

    int frame_count;
    int finished;

    event* events;
    int event_count;
    int current_event_index;

    vect2 previous_bike_r; // bike position during previous frame
    double previous_frame_time, next_frame_time;
    int next_frame_index;

    int flagtag_;

    // Load replay of one bike
    int load(const char* filename, FILE* h, int demo);
    // Save replay of one bike
    void save(const char* filename, FILE* h, int level_id, int flagtag);

  public:
    char level_filename[16];

    recorder(void);
    ~recorder(void);
    void erase(char* lev_filename);
    void rewind(void);
    int recall_frame(motorst* mot, double time, bike_sound* sound);
    void store_frames(motorst* mot, double time, bike_sound* sound);

    void store_event(double time, WavEvent event_id, double volume, int object_id);
    // Return true if a new event has occurred
    int recall_event(double time, WavEvent* event_id, double* volume, int* object_id);

    int flagtag(void);
    void set_flagtag(int flagtag);

    // Encode framecount into MSB of the flags
    void encode_frame_count();
    // Check that the framecount matches with the MSB of the flags
    bool frame_count_integrity();
};

extern recorder *Rec1, *Rec2;
extern int MultiplayerRec;

void add_event_buffer(WavEvent event_id, double volume, int object_id);
void reset_event_buffer(void);
// Return true if a new event has been obtained
int get_event_buffer(WavEvent* event_id, double* volume, int* object_id);

#endif
