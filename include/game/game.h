#ifndef GAME_H
#define GAME_H

#include "game/recorder.h"
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

extern int WhoDiedFirst;
extern bool Player1Finished;

#endif
