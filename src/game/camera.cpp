#include "game/camera.h"
#include "eol/console.h"
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
