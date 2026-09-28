#include "game/driver.h"
#include "eol/settings.h"
#include "physics/forces.h"
#include <format>

constexpr double PHYSICS_SPEED_TO_EOL_SPEED = 5.0;

static std::string format_value(double value) { return std::format("{:.2f}", value); }

std::string motor_stats::format_speed() const { return format_value(speed); }
std::string motor_stats::format_max_speed() const { return format_value(max_speed); }

void driver::update_speed() {
    stats.speed = mot->bike.v.length() * PHYSICS_SPEED_TO_EOL_SPEED;
    stats.max_speed = std::max(stats.max_speed, stats.speed);
}

void driver::reset_metadata() {
    sound.motor_frequency = 0.0;
    sound.gas = 0;
    sound.friction_volume = 0.0;

    meta.volt_time = -100.0;
    meta.volt_is_right = false;

    meta.turn_key_previous = false;
    meta.one_turn_used = false;

    meta.arm_position = 0.0;

    meta.bike_turning.flipped = 0;
    meta.bike_turning.turn_time = -1000.0;
    meta.bike_turning.turn_phase = 0.0;

    meta.camera_turning.flipped = 0;
    meta.camera_turning.turn_time = -1000.0;
    meta.camera_turning.turn_phase = 0.0;
}

driver::driver(motorst* mot, recorder* rec, player_keys* keys, HudSlot hud_slot)
    : mot(mot),
      rec(rec),
      keys(keys),
      hud_slot(hud_slot) {
    reset_metadata();
    reset_motor_forces(mot);
}

bool driver::show_minimap() const {
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

bool driver::show_timer() const {
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

void driver::toggle_minimap() const {
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

void driver::toggle_timer() const {
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
