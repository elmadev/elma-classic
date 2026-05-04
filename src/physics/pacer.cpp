#include "physics/pacer.h"
#include "main.h"

namespace pacer {

namespace {

static double time = 0.0;
static double target_time = 0.0;

} // namespace

void reset() {
    time = 0.0;
    target_time = 0.0;
}

void new_frame() {
    target_time = stopwatch() * 0.0024;
    target_time = std::max(0.000001, target_time);
}

bool subframe(double* out_dt) {
    double dt = target_time - time;
    if (0.000001 > dt) {
        return false;
    }
    dt = std::min(dt, PHYS_MAX_TIMESTEP);
    *out_dt = dt;
    time += dt;
    return true;
}

} // namespace pacer
