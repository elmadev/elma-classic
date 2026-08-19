#include "eol/kuski.h"

void kuski::clear_apple_data() {
    for (bool& taken : apples_taken) {
        taken = false;
    }
}

const struct spy_data* kuski::spy_data() const { return data ? &*data : nullptr; }

void kuski::add_spy_data(const struct spy_data& sd) { data = sd; }

void kuski::clear_spy_data() { data.reset(); }
