#include "game/driver.h"
#include "physics/forces.h"
#include "renderer/timer.h"
#include <algorithm>
#include <cstdint>
#include <format>

static std::string format2_value(double value) { return std::format("{:.2f}", value); }
static std::string format5_value(double value) { return std::format("{:.5f}", value); }

std::string run_stats::format_speed() const { return format2_value(speed); }
std::string run_stats::format_max_speed() const { return format2_value(max_speed); }

std::string run_stats::format_wheel_distance() const { return format5_value(wheel_distance); }
std::string run_stats::format_min_wheel_distance() const {
    return format5_value(min_wheel_distance);
}

namespace {

// The shortest tap still counts as a centisecond
uint32_t held_centiseconds(double held_time) {
    if (held_time <= 0) {
        return 0;
    }

    return std::max<uint32_t>(1, static_cast<uint32_t>(held_time * TIME_TO_CENTISECONDS));
}

// An input that was never let go is reported as the whole run.
// Anything else stays strictly below the run time.
uint32_t input_centiseconds(uint32_t held, bool released, uint32_t run_time) {
    if (!released) {
        return held > 0 ? run_time : 0;
    }

    if (run_time == 0) {
        return 0;
    }

    return std::min(held, run_time - 1);
}

} // namespace

uint32_t run_stats::throttle_centiseconds(uint32_t run_time) const {
    return input_centiseconds(held_centiseconds(throttle_time), throttle_released, run_time);
}

uint32_t run_stats::brake_centiseconds(uint32_t run_time) const {
    return input_centiseconds(held_centiseconds(brake_time), brake_released, run_time);
}

void driver::update_speed() {
    stats.speed = mot->speed();
    stats.max_speed = std::max(stats.max_speed, stats.speed);
}

void driver::update_wheel_distance() {
    stats.wheel_distance = mot->wheel_distance();
    stats.min_wheel_distance = std::min(stats.min_wheel_distance, stats.wheel_distance);
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

driver::driver(motorst* mot, recorder* rec)
    : mot(mot),
      rec(rec) {
    reset_metadata();
    reset_motor_forces(mot);
}

game_driver::game_driver(motorst* mot, recorder* rec, player_keys* keys)
    : driver(mot, rec),
      keys(keys) {}

replay_driver::replay_driver(motorst* mot, recorder* rec)
    : driver(mot, rec) {}
