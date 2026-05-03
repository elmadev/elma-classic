#ifndef LEJATSZO_H
#define LEJATSZO_H

#include "recorder.h"
#include <string>

// Jatek modja ket fele lehet:
extern int Single;
extern int FlagTag; // Van-e fogocska
extern bool OutOfBounds;
// extern int Coop;

extern int ScreenshotRequested; // kiraj320 ebbol tudja, hogy snapshot-ot keszitsen
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

// Azt a nevet kerik, amit topol::topol kap:
// BestTime beallitasahoz kell nekik:
long game_loop(const char* filename, CameraMode camera_mode);
long replay_loop(const char* filename, int restore_player_visibility);

void setup_render_directory(const std::string& replay_filename);
void render_replay(const char* level_filename);

struct turning_data {
    int flipped;
    double bike_turn_time;
    double camera_turn_time;
    double camera_turn_phase; // kulsoresz szamolja tobbibol
    double bike_turn_phase;
};

struct bike_metadata {
    bike_sound sound;
    double surlero;
    double frekvencia;
    double volt_time;
    int volt_is_right;

    int draw_view;

    int turn_key_previous;
    bool one_turn_used;

    double arm_position;

    turning_data bike_turning, camera_turning;
};

// lejatszo ezt beallitja: 0 -> nem kell kiirni, 1 -> A eloszor, 2 -> B
// csak akkor A vagy B eloszor, ha mindketten meghaltak, tehat
// menu_level menu-nek ezt mar feldolgozta elore:
extern int WhoDiedFirst;

// Ha sikeres szint, ide teszi be hogy ki ment ki:
extern int Player1Finished;

#endif
