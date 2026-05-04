#ifndef LEJATSZO_H
#define LEJATSZO_H

#include "recorder.h"
#include <string>

extern int Single;
extern int FlagTag;
extern bool OutOfBounds;

extern bool ScreenshotRequested;
extern bool VideoRecordingMode;
extern int VideoFrameIndex;
extern std::string VideoOutputDirectory;

enum class CameraMode { Normal, MapViewer };

struct camera {
    CameraMode mode;
    double x;
    double y;
    double min_x;
    double min_y;
    double max_x;
    double max_y;
};

int game_loop(const char* filename, CameraMode camera_mode);
int replay_loop(const char* filename, int restore_player_visibility);

void setup_render_directory(const std::string& replay_filename);
void render_replay(const char* level_filename);

struct turning_data {
    int flipped;
    double bike_turn_time;
    double camera_turn_time;
    double camera_turn_phase;
    double bike_turn_phase;
};

struct bike_metadata {
    bike_sound sound;
    double volt_time;
    bool volt_is_right;

    bool draw_view;

    bool turn_key_previous;
    bool one_turn_used;

    double arm_position;

    turning_data bike_turning;
    turning_data camera_turning;
};

extern int WhoDiedFirst;
extern bool Player1Finished;

#endif
