#include "game/driver.h"
#include "eol/settings.h"
#include "physics/forces.h"
#include "renderer/timer.h"
#include <algorithm>
#include <cstdint>
#include <format>

constexpr double PHYSICS_SPEED_TO_EOL_SPEED = 5.0;

static std::string format_value(double value) { return std::format("{:.2f}", value); }

std::string run_stats::format_speed() const { return format_value(speed); }
std::string run_stats::format_max_speed() const { return format_value(max_speed); }

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

// The `rec` argument is only used for game play, not when playing a replay.
void driver::update_bike_turn_phase(bool update_rec, double time, int flipped) {
    turning_data* data = &meta.bike_turning;

    if (data->flipped != flipped) {
        // New flip this frame
        data->flipped = flipped;
        data->turn_time = time;
        if (update_rec) {
            start_wav(WavEvent::Turn, 0.99);
            rec->store_event(time, WavEvent::Turn, 0.99, -1);
        }
    }

    double turn_time = EolSettings->turn_time();
    if (turn_time == 0.0) {
        // Instant turn
        data->turn_phase = 1.0;
    } else {
        data->turn_phase = (time - data->turn_time) / turn_time;
        data->turn_phase = std::clamp(data->turn_phase, 0.0, 1.0);
    }
}

void driver::update_camera_turn_phase(double time, int flipped) {
    turning_data* data = &meta.camera_turning;

    double camera_flip_time = EolSettings->turn_time() + 0.15;
    if (data->flipped != flipped) {
        // New flip this frame
        data->flipped = flipped;
        double time_since_prev_turn = time - data->turn_time;
        if (camera_flip_time > 0.0 && time_since_prev_turn < camera_flip_time) {
            // If camera is mid-turn, calculate camera start time so it seamlessly continues from
            // the mid-turn position
            data->turn_time = time + time_since_prev_turn - camera_flip_time;
        } else {
            // Camera is not mid-turn, so just set the camera turn time normally
            data->turn_time = time;
        }
    }

    double elapsed_time = std::max(0.0, time - data->turn_time);
    data->turn_phase = std::min(1.0, elapsed_time / camera_flip_time);
    if (flipped) {
        data->turn_phase = 1.0 - data->turn_phase;
    }
}

void driver::update_graphical_metadata(bool update_rec, double time) {
    // Update bike turn data
    update_bike_turn_phase(update_rec, time, mot->flipped_bike);

    // Update camera position
    int flipped_camera = mot->flipped_bike;
    if (mot->gravity_direction == MotorGravity::Up) {
        flipped_camera = !flipped_camera;
    }
    update_camera_turn_phase(time, flipped_camera);

    // Update arm position
    meta.arm_position = std::max(0.0, 1.0 - (time - meta.volt_time) / VoltDelay);
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
