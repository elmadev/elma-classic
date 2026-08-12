#include "game/driver.h"
#include "physics/forces.h"
#include <cmath>
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

driver::driver(motorst* mot, recorder* rec, player_keys* keys, hud_visibility* hud)
    : mot(mot),
      rec(rec),
      keys(keys),
      hud(hud) {
    reset_metadata();
    reset_motor_forces(mot);
}
