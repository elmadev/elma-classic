#ifndef GAME_REPLAY_BIKE_H
#define GAME_REPLAY_BIKE_H

#include "game/recorded_bike.h"
#include <string>

// A replay drawn alongside the followed one: no view, sound or level interaction
struct replay_bike {
    recorded_bike bike;
    std::string name;

    // Returns false once the replay has run out of frames
    bool advance(double time, bool rewinding);
};

#endif
