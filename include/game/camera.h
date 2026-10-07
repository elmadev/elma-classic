#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

class level;
struct motorst;
struct player_keys;
class state;

enum class CameraMode { Normal, MapViewer };

struct view {
    enum class HudSlot {
        Game1,
        Game2,
        Replay1,
        Replay2,
    };

    HudSlot hud_slot;
    bool draw_view = true;
    player_keys* keys;

    bool show_minimap() const;
    bool show_timer() const;
    void toggle_minimap() const;
    void toggle_timer() const;

    void update_view_settings(bool* other_draw_view);
};

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

    view player1;
    view player2;

    camera(CameraMode mode, bool is_replay, state* stat);

    void update_view_settings(bool single);

    void update_freecam(double dt);
    void init_freecam(const level* lev, const motorst* mot);
};

#endif
