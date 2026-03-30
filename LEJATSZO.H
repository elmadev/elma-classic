#ifndef LEJATSZO_H
#define LEJATSZO_H

#include "recorder.h"
#include <string>

// Jatek modja ket fele lehet:
extern int Single;
extern int Tag; // Van-e fogocska
// extern int Coop;

extern int Legkozment; // kiraj320 ebbol tudja, hogy snapshot-ot keszitsen
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
long lejatszo(const char* filenev, CameraMode cameramode);
long lejatszo_r(const char* filenev, int showkepmarad);

void setup_render_directory(const std::string& replay_filename);
void render_replay(const char* level_filename);

struct turning_camera_pan_animation {
    // `flipped_bike` is true when the bike is facing right.
    bool flipped_bike;
    double ucsoforgas;
    double ucsoford;
    // 0.0 is the leftmost position, for when the bike is facing right, and 1.0 is the rightmost
    // position.
    // The camera position is animated from left to right after turning at the rate defined by
    // `EolSettings->turn_time`.
    double camera_position;
    double forgas;
};

struct valtozok {
    bike_sound inf;
    double surlero;
    double frekvencia;
    double utolsougras;
    int ugras1volt;

    int showkep;

    int hatranyomva;
    bool one_turn_used;

    double ugrasnagysag;

    turning_camera_pan_animation baljobbv_f, baljobbv_h;
};

// lejatszo ezt beallitja: 0 -> nem kell kiirni, 1 -> A eloszor, 2 -> B
// csak akkor A vagy B eloszor, ha mindketten meghaltak, tehat
// menu_level menu-nek ezt mar feldolgozta elore:
extern int MeghalteloszorAB;

// Ha sikeres szint, ide teszi be hogy ki ment ki:
extern int Aerintetteviragot;

#endif
