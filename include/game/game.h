#ifndef GAME_GAME_H
#define GAME_GAME_H

#include "game/camera.h"
#include <string>

extern int Single;
extern int FlagTag;
extern bool OutOfBounds;

extern bool ScreenshotRequested;
extern bool VideoRecordingMode;
extern int VideoFrameIndex;
extern std::string VideoOutputDirectory;

void reload_graphic_assets();

int game_loop(const char* filename, CameraMode camera_mode);
int replay_loop(const char* filename, bool restore_player_visibility);

void setup_render_directory(const std::string& replay_filename);
void render_replay(const char* level_filename);

extern int WhoDiedFirst;
extern bool Player1Finished;

#endif
