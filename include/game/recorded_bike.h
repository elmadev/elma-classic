#ifndef GAME_RECORDED_BIKE_H
#define GAME_RECORDED_BIKE_H

#include "game/recorder.h"
#include "physics/forces.h"
#include "physics/init.h"
#include <concepts>
#include <optional>

struct turning_data {
    int flipped;
    double turn_time;
    double turn_phase;
};

struct bike_metadata {
    double volt_time;
    bool volt_is_right;

    bool turn_key_previous;
    bool one_turn_used;

    double arm_position;

    turning_data bike_turning;
    turning_data camera_turning;

    void reset();
    void note_volt(const event& ev, double time);
};

// A bike and the recorder that drives it
struct recorded_bike {
    recorder rec;
    motorst mot;
    bike_metadata meta;
    bike_sound sound;

    recorded_bike();
};

// Returns false once the replay has run out of frames
template <std::invocable<const event&> OnEvent>
bool recall_recorded_frame(recorder& rec, motorst& mot, bike_metadata& meta, bike_sound& sound,
                           double time, OnEvent&& on_event) {
    bool alive = rec.recall_frame(&mot, time, &sound);
    set_head_position(&mot);
    while (std::optional<event> ev = rec.recall_event(time)) {
        if (ev->object_id < 0) {
            meta.note_volt(*ev, time);
        }
        on_event(*ev);
    }
    return alive;
}

void update_graphical_metadata(bike_metadata& meta, motorst* mot, recorder* rec, bool update_rec,
                               double time);
// During rewind, compute animation state from the recorder's event list
// instead of relying on the forward-only state machine.
void rewind_override_animations(bike_metadata& meta, motorst* mot, recorder* rec, double time);

#endif
