#ifndef DRIVER_H
#define DRIVER_H

#include "game/recorder.h"

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
    bike_sound sound;
    double volt_time;
    bool volt_is_right;

    bool turn_key_previous;
    bool one_turn_used;

    double arm_position;

    turning_data bike_turning;
    turning_data camera_turning;
};

struct driver {
    motorst* mot;
    bike_metadata meta;
    recorder* rec;
    player_keys* keys;
    hud_visibility* hud;

    bool dead = false;
    int finish_time = 0;
    bool draw_view = true;

    driver(motorst* mot, recorder* rec, player_keys* keys, hud_visibility* hud);
    void reset_metadata();
};

#endif
