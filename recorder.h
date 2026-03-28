#ifndef RECORDER_H
#define RECORDER_H

#include "sound_engine.h"
#include "vect2.h"
#include <cstdio>
#include <optional>
#include <string>
#include <vector>

constexpr const char LAST_REC_FILENAME[] = "!last.rec";

struct rec_header {
    int level_id;
    char level_filename[16];
};

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
    int frame_count_;

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
    int load(const char* filename, FILE* h, bool is_first_replay);
    // Save replay of one bike
    void save(const char* filename, FILE* h, int level_id);

  public:
    char level_filename[16];

    recorder();
    ~recorder();

    struct merge_result {
        int level_id;           // level_id from the first replay
        bool rec1_was_multi;    // first file was already a multiplayer replay
        bool rec2_was_multi;    // second file was already a multiplayer replay
        bool level_id_mismatch; // the two replays are from different levels
    };

    // Load a singleplayer or multiplayer replay
    static int load_rec_file(const char* filename, bool demo);
    // Save a singleplayer or multiplayer replay
    static void save_rec_file(const char* filename, int level_id);
    // Load two replay files and merge them into a multiplayer replay
    static merge_result load_merge(const std::string& filename1, const std::string& filename2);
    // Read only the header (level_id + level_filename) from a .rec file
    static std::optional<rec_header> read_header(const std::string& filename);

    bool is_empty() const { return frame_count_ == 0; }
    int frame_count() const { return frame_count_; }
    void erase(const char* lev_filename);
    void rewind();
    bool recall_frame(motorst* mot, double time, bike_sound* sound);
    void store_frames(motorst* mot, double time, bike_sound* sound);

    void store_event(double time, WavEvent event_id, double volume, int object_id);
    // Return true if a new event has occurred
    bool recall_event(double time, WavEvent* event_id, double* volume, int* object_id);
    // Walk events backward: returns true for each event whose time > `time`
    bool recall_event_reverse(double time, WavEvent* event_id, double* volume, int* object_id);

    // Find frame time of the last direction change at or before `time`,
    // detected from the flipped_bike flag in frame data.
    // Returns -1000.0 if no direction change found.
    double find_last_turn_frame_time(double time) const;

    // Find time of the last volt (RightVolt or LeftVolt) at or before `time`.
    // Sets `is_right_volt` to indicate direction. Returns -1000.0 if none found.
    double find_last_volt_time(double time, bool* is_right_volt) const;

    bool flagtag() const { return (bool)(flagtag_); };
    void set_flagtag(bool flagtag) { flagtag_ = (int)(flagtag); }

    // Encode framecount into MSB of the flags
    void encode_frame_count();
    // Check that the framecount matches with the MSB of the flags
    bool frame_count_integrity();
};
static_assert(sizeof(recorder::level_filename) == 16);

extern recorder* Rec1;
extern recorder* Rec2;
extern int MultiplayerRec;

void add_event_buffer(WavEvent event_id, double volume, int object_id);
void reset_event_buffer();
// Return true if a new event has been obtained
bool get_event_buffer(WavEvent* event_id, double* volume, int* object_id);

#endif
