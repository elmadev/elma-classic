#ifndef DRIVER_H
#define DRIVER_H

#include "game/recorder.h"
#include <string>

struct motorst;
struct player_keys;

struct hud_visibility {
    bool minimap;
    bool timer;
};

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
};

struct motor_stats {
    double speed = 0.0;
    std::string format_speed() const;

    double max_speed = 0.0;
    std::string format_max_speed() const;
};

struct driver {
    motorst* mot;
    bike_metadata meta;
    recorder* rec;
    player_keys* keys;
    hud_visibility* hud;
    bike_sound sound;
    motor_stats stats;

    bool dead = false;
    int finish_time = 0;
    bool draw_view = true;
    bool one_frame_brake_pending = false;

    driver(motorst* mot, recorder* rec, player_keys* keys, hud_visibility* hud);
    void reset_metadata();
    void update_speed();
};

#endif
