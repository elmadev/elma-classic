#include "game/replay_bike.h"

bool replay_bike::advance(double time, bool rewinding) {
    if (rewinding) {
        bike.rec.rewind();
    }

    // Object events are skipped, so the level is left alone
    if (!recall_recorded_frame(bike.rec, bike.mot, bike.meta, bike.sound, time,
                               [](const event&) {})) {
        return false;
    }

    if (rewinding) {
        rewind_override_animations(bike.meta, &bike.mot, &bike.rec, time);
    }
    update_graphical_metadata(bike.meta, &bike.mot, &bike.rec, false, time);
    return true;
}
