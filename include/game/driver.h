#ifndef DRIVER_H
#define DRIVER_H

#include "game/recorder.h"

struct bike_metadata;
struct motorst;
struct player_keys;

struct driver {
    motorst* mot;
    bike_metadata* meta;
    bool dead = false;
    int finish_time = 0;
    recorder* rec;
    player_keys* keys;

    driver(motorst* mot, bike_metadata* meta, recorder* rec, player_keys* keys);
};

#endif
