#ifndef EOL_PACER_H
#define EOL_PACER_H

#include "main.h"

namespace pacer {

constexpr double MILLISECONDS_TO_PHYS_TIME = STOPWATCH_MULTIPLIER * STOPWATCH_TO_PHYS_TIME;
constexpr double PHYS_MAX_TIMESTEP = 0.0055;

// Returns string showing current and next FPS limit
std::string format_fps_limit();

// Will be updated on the next run
void toggle_fps_boost();

// Will be updated on the next run
void request_fps_limit(bool enabled, int limit);

void reset();

void new_frame();

bool subframe(double* out_dt);

} // namespace pacer

#endif
