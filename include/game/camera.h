#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

class level;
struct motorst;

enum class CameraMode { Normal, MapViewer };

struct view {
    enum class HudSlot {
        Game1,
        Game2,
        Replay1,
        Replay2,
    };

    HudSlot hud_slot;

    bool show_minimap() const;
    bool show_timer() const;
    void toggle_minimap() const;
    void toggle_timer() const;
};

struct camera {
    CameraMode mode;
    double x;
    double y;
    double min_x;
    double min_y;
    double max_x;
    double max_y;

    view player1;
    view player2;

    camera(CameraMode mode, bool is_replay);
    void update_freecam(double dt);
    void init_freecam(const level* lev, const motorst* mot);
};

#endif
