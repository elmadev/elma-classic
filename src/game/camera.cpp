#include "game/camera.h"
#include "eol/console.h"
#include "eol/settings.h"
#include "game/state.h"
#include "level/level.h"
#include "physics/init.h"
#include "platform/scancode.h"
#include "renderer/render.h"
#include <algorithm>

bool view::show_minimap() const {
    switch (hud_slot) {
    case HudSlot::Game1:
        return EolSettings->show_minimap_player_a();
    case HudSlot::Game2:
        return EolSettings->show_minimap_player_b();
    case HudSlot::Replay1:
        return EolSettings->show_replay_minimap_player_a();
    case HudSlot::Replay2:
        return EolSettings->show_replay_minimap_player_b();
    }
    return false;
}

bool view::show_timer() const {
    switch (hud_slot) {
    case HudSlot::Game1:
        return EolSettings->show_timer_player_a();
    case HudSlot::Game2:
        return EolSettings->show_timer_player_b();
    case HudSlot::Replay1:
        return EolSettings->show_replay_timer_player_a();
    case HudSlot::Replay2:
        return EolSettings->show_replay_timer_player_b();
    }
    return false;
}

void view::toggle_minimap() const {
    bool show = !show_minimap();
    switch (hud_slot) {
    case HudSlot::Game1:
        EolSettings->set_show_minimap_player_a(show);
        return;
    case HudSlot::Game2:
        EolSettings->set_show_minimap_player_b(show);
        return;
    case HudSlot::Replay1:
        EolSettings->set_show_replay_minimap_player_a(show);
        return;
    case HudSlot::Replay2:
        EolSettings->set_show_replay_minimap_player_b(show);
        return;
    }
}

void view::toggle_timer() const {
    bool show = !show_timer();
    switch (hud_slot) {
    case HudSlot::Game1:
        EolSettings->set_show_timer_player_a(show);
        return;
    case HudSlot::Game2:
        EolSettings->set_show_timer_player_b(show);
        return;
    case HudSlot::Replay1:
        EolSettings->set_show_replay_timer_player_a(show);
        return;
    case HudSlot::Replay2:
        EolSettings->set_show_replay_timer_player_b(show);
        return;
    }
}

camera::camera(CameraMode mode, bool is_replay, state* stat)
    : mode(mode) {
    if (is_replay) {
        player1.hud_slot = view::HudSlot::Replay1;
        player2.hud_slot = view::HudSlot::Replay2;
    } else {
        player1.hud_slot = view::HudSlot::Game1;
        player2.hud_slot = view::HudSlot::Game2;
    }
    player1.keys = &stat->keys1;
    player2.keys = &stat->keys2;
}

void view::update_view_settings(bool* other_draw_view) {
    // Visibility of player viewpoint
    if (was_game_key_just_pressed(keys->toggle_visibility)) {
        reset_game_background();
        if (!*other_draw_view) {
            // You cannot have 0 players visible, so make both players visible instead
            *other_draw_view = true;
            draw_view = true;
        } else {
            draw_view = !draw_view;
        }
    }

    if (was_game_key_just_pressed(keys->toggle_minimap)) {
        toggle_minimap();
    }

    if (was_game_key_just_pressed(keys->toggle_timer)) {
        toggle_timer();
    }
}

void camera::update_view_settings(bool single) {
    player1.update_view_settings(&player2.draw_view);
    if (!single) {
        player2.update_view_settings(&player1.draw_view);
    }
}

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
