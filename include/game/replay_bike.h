#ifndef GAME_REPLAY_BIKE_H
#define GAME_REPLAY_BIKE_H

#include "game/recorded_bike.h"
#include "pic/pic8.h"
#include <memory>
#include <string>

// A replay drawn alongside the followed one: no view, sound or level interaction
struct replay_bike {
    recorded_bike bike;
    std::string name;
    std::string nick;
    std::unique_ptr<pic8> shirt;

    // Returns false once the replay has run out of frames
    bool advance(double time, bool rewinding);
};

#endif
