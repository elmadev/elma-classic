#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

class level;
struct motorst;

enum class CameraMode { Normal, MapViewer };

struct camera {
    CameraMode mode;
    double x;
    double y;
    double start_x;
    double start_y;
    double min_x;
    double min_y;
    double max_x;
    double max_y;

    void update_freecam(double dt);
    void init_freecam(const level* lev, const motorst* mot);
};

#endif
