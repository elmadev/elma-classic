#ifndef EOL_PACER_H
#define EOL_PACER_H

namespace pacer {

constexpr double PHYS_MAX_TIMESTEP = 0.0055;

void reset();

void new_frame();

bool subframe(double* out_dt);

} // namespace pacer

#endif
