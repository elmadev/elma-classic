#include "eol/kuski.h"

void kuski::clear_apple_data() {
    for (bool& taken : apples_taken) {
        taken = false;
    }
}

const struct spy_data* spy_playback::spy_data() const { return data ? &*data : nullptr; }

void spy_playback::add(const struct spy_data& sd) { data = sd; }

void spy_playback::clear() { data.reset(); }
