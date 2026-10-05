#include "game/driver.h"
#include "editor/editor.h"
#include "eol/settings.h"
#include "level/level.h"
#include "level/object.h"
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

BikeState driver::handle_object_interaction(int object_id) {
    if (object_id < 0 || object_id >= MAX_OBJECTS) {
        internal_error("handle_object_interaction object_id < 0 || object_id >= MAX_OBJECTS!");
    }
    if (!Level->objects[object_id]) {
        internal_error("handle_object_interaction !Level->objects[object_id]!");
    }

    object::Type type = Level->objects[object_id]->type;

    if (type == object::Type::Killer) {
        return BikeState::Dead;
    }
    if (type == object::Type::Food) {
        Level->objects[object_id]->active = false;
        mot->apple_count++;
        add_event_buffer(WavEvent::Food, 0.99, -1);
        std::optional<MotorGravity> gravity = Level->objects[object_id]->gravity();
        if (gravity.has_value()) {
            mot->gravity_direction = gravity.value();
        }
        return BikeState::Normal;
    }
    if (type == object::Type::Exit) {
        if (Motor1->apple_count + Motor2->apple_count >= Level->total_apples) {
            return BikeState::Finish;
        }
    }
    return BikeState::Normal;
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

void replay_driver::reverse_events(double time) {
    while (std::optional<event> ev = rec->recall_event_reverse(time)) {
        if (ev->object_id < 0) {
            continue;
        }
        object* obj = Level->objects[ev->object_id];
        if (obj && obj->type == object::Type::Food) {
            obj->active = true;
            mot->apple_count--;

            mot->last_apple_time = (int)(rec->last_apple_time().value_or(0) * TIME_TO_CENTISECONDS);

            if (obj->gravity()) {
                mot->gravity_direction = rec->last_gravity(*Level);
            }
        }
    }
}

// During rewind, compute animation state from the recorder's event list
// instead of relying on the forward-only state machine.
void replay_driver::rewind_override_animations(double time) {
    double turn_time = rec->find_last_turn_frame_time(time).value_or(-1000.0);
    meta.bike_turning.flipped = mot->flipped_bike;
    meta.bike_turning.turn_time = turn_time;

    meta.camera_turning.turn_time = -1000.0;
    int flipped_camera = mot->flipped_bike;
    if (mot->gravity_direction == MotorGravity::Up) {
        flipped_camera = !flipped_camera;
    }
    meta.camera_turning.flipped = flipped_camera;

    meta.volt_time = rec->last_volt_time(&meta.volt_is_right).value_or(-1000.0);
}

// Load replay data (instead of simulating bike physics)
void replay_driver::replay_frame(double time) {
    // Load replay data
    dead = !rec->recall_frame(mot, time, &sound);
    set_head_position(mot);

    // Play events
    while (std::optional<event> ev = rec->recall_event(time)) {
        if (ev->object_id >= 0) {
            int prev_apple_count = mot->apple_count;
            handle_object_interaction(ev->object_id);
            if (prev_apple_count < mot->apple_count) {
                mot->last_apple_time = (int)(ev->time * TIME_TO_CENTISECONDS);
            }
        } else {
            start_wav(ev->event_id, ev->volume);
            if (ev->event_id == WavEvent::RightVolt) {
                meta.volt_is_right = true;
                meta.volt_time = time;
            }
            if (ev->event_id == WavEvent::LeftVolt) {
                meta.volt_is_right = false;
                meta.volt_time = time;
            }
        }
    }
    return;
}

replay_driver::replay_driver(motorst* mot, recorder* rec)
    : driver(mot, rec) {}
