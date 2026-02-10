#ifndef RECORDER_H
#define RECORDER_H

#include "sound_engine.h"
#include "vect2.h"
#include <cstdio>
#include <vector>

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
static_assert(sizeof(event) == 0x10);

struct frame_data {
    float bike_x;
    float bike_y;
    short left_wheel_x;
    short left_wheel_y;
    short right_wheel_x;
    short right_wheel_y;
    short body_x;
    short body_y;
    short bike_rotation; // (0-9999)
    // (0-249, glitched 0-255 when brake-stretching)
    unsigned char left_wheel_rotation;
    unsigned char right_wheel_rotation;
    unsigned char flags;
    unsigned char motor_frequency;
    unsigned char friction_volume;
};
static_assert(sizeof(frame_data) == 28);

class recorder {
    friend void menu_replay();

    int frame_count;

    std::vector<frame_data> frames;

    int event_count;
    std::vector<event> events;

    // store/recall vars
    bool finished;
    vect2 previous_bike_r; // bike position during previous frame
    double previous_frame_time;
    double next_frame_time;
    int next_frame_index;
    int current_event_index;

    int flagtag_;

    // Load replay of one bike
    int load(const char* filename, FILE* h, int demo, bool is_first_replay);
    // Save replay of one bike
    void save(const char* filename, FILE* h, int level_id, int flagtag);

  public:
    char level_filename[16];

    recorder();
    ~recorder();

    // Load a singleplayer or multiplayer replay
    static int load_rec_file(const char* filename, int demo);
    // Save a singleplayer or multiplayer replay
    static void save_rec_file(const char* filename, int level_id, int flagtag);

    bool is_empty() { return frame_count == 0; }
    void erase(char* lev_filename);
    void rewind();
    bool recall_frame(motorst* mot, double time, bike_sound* sound);
    void store_frames(motorst* mot, double time, bike_sound* sound);

    void store_event(double time, WavEvent event_id, double volume, int object_id);
    // Return true if a new event has occurred
    bool recall_event(double time, WavEvent* event_id, double* volume, int* object_id);

    bool flagtag();
    void set_flagtag(bool flagtag);

    // Encode framecount into MSB of the flags
    void encode_frame_count();
    // Check that the framecount matches with the MSB of the flags
    bool frame_count_integrity();
};

extern recorder* Rec1;
extern recorder* Rec2;
extern int MultiplayerRec;

void add_event_buffer(WavEvent event_id, double volume, int object_id);
void reset_event_buffer();
// Return true if a new event has been obtained
bool get_event_buffer(WavEvent* event_id, double* volume, int* object_id);

#endif
