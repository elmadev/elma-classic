#ifndef GAME_DRIVER_H
#define GAME_DRIVER_H

#include "game/recorder.h"
#include <cstdint>
#include <string>

struct motorst;
struct player_keys;

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

struct run_stats {
    double speed = 0.0;
    std::string format_speed() const;

    double max_speed = 0.0;
    std::string format_max_speed() const;

    // Physics time, counted from the inputs the bike actually got after cripples
    double throttle_time = 0.0;
    double brake_time = 0.0;
    bool throttle_released = false;
    bool brake_released = false;
    // Centiseconds for the run summary, given the run time in centiseconds
    uint32_t throttle_centiseconds(uint32_t run_time) const;
    uint32_t brake_centiseconds(uint32_t run_time) const;

    int left_volt_count = 0;
    int right_volt_count = 0;
    int super_volt_count = 0;
    int turn_count = 0;

    // Only a run that was drunk from start to finish counts as drunk
    bool drunk = false;
};

struct driver {
    motorst* mot;
    bike_metadata meta;
    recorder* rec;
    player_keys* keys;
    bike_sound sound;
    run_stats stats;

    bool dead = false;
    int finish_time = 0;
    bool one_frame_brake_pending = false;

    driver(motorst* mot, recorder* rec, player_keys* keys);
    void reset_metadata();
    void update_speed();
};

#endif
