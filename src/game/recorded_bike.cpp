#include "game/recorded_bike.h"
#include "eol/settings.h"
#include "sound/engine.h"
#include <algorithm>

void bike_metadata::reset() {
    volt_time = -100.0;
    volt_is_right = false;

    turn_key_previous = false;
    one_turn_used = false;

    arm_position = 0.0;

    bike_turning.flipped = 0;
    bike_turning.turn_time = -1000.0;
    bike_turning.turn_phase = 0.0;

    camera_turning.flipped = 0;
    camera_turning.turn_time = -1000.0;
    camera_turning.turn_phase = 0.0;
}

void bike_metadata::note_volt(const event& ev, double time) {
    if (ev.event_id == WavEvent::RightVolt) {
        volt_is_right = true;
        volt_time = time;
    }
    if (ev.event_id == WavEvent::LeftVolt) {
        volt_is_right = false;
        volt_time = time;
    }
}

// The `rec` argument is only used for game play, not when playing a replay.
static void update_bike_turn_phase(bike_metadata& meta, recorder* rec, bool update_rec, double time,
                                   int flipped) {
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

static void update_camera_turn_phase(turning_data* data, double time, int flipped) {
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

void update_graphical_metadata(bike_metadata& meta, motorst* mot, recorder* rec, bool update_rec,
                               double time) {
    // Update bike turn data
    update_bike_turn_phase(meta, rec, update_rec, time, mot->flipped_bike);

    // Update camera position
    int flipped_camera = mot->flipped_bike;
    if (mot->gravity_direction == MotorGravity::Up) {
        flipped_camera = !flipped_camera;
    }
    update_camera_turn_phase(&meta.camera_turning, time, flipped_camera);

    // Update arm position
    meta.arm_position = std::max(0.0, 1.0 - (time - meta.volt_time) / VoltDelay);
}

// During rewind, compute animation state from the recorder's event list
// instead of relying on the forward-only state machine.
void rewind_override_animations(bike_metadata& meta, motorst* mot, recorder* rec, double time) {
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

recorded_bike::recorded_bike()
    : mot{},
      sound{} {
    init_motor(&mot);
    meta.reset();
}
