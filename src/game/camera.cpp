#include "game/camera.h"
#include "eol/console.h"
#include "level/level.h"
#include "physics/init.h"
#include "platform/scancode.h"
#include <algorithm>

void camera::update_freecam(double dt) {
    double speed = 30.0;
    if (is_game_key_down(DIK_LSHIFT) || is_game_key_down(DIK_RSHIFT)) {
        speed *= 4.0;
    }
    double move = speed * dt;
    if (is_game_key_down(DIK_UP)) {
        y += move;
    }
    if (is_game_key_down(DIK_DOWN)) {
        y -= move;
    }
    if (is_game_key_down(DIK_LEFT)) {
        x -= move;
    }
    if (is_game_key_down(DIK_RIGHT)) {
        x += move;
    }

    x = std::clamp(x, min_x, max_x);
    y = std::clamp(y, min_y, max_y);
}

void camera::init_freecam(const level* lev, const motorst* mot) {
    x = mot->bike.r.x;
    y = mot->bike.r.y;
    start_x = mot->bike.r.x;
    start_y = mot->bike.r.y;

    double level_min_y;
    double level_max_y;
    lev->get_boundaries(&min_x, &level_min_y, &max_x, &level_max_y, false);
    // Convert level y-coordinates to camera y-coordinates
    min_y = -level_max_y;
    max_y = -level_min_y;
}
