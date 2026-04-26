#ifdef PROFILE_PERFORMANCE

#include "debug/profiler.h"
#include "log.h"

debug_timer::debug_timer() { start = std::chrono::high_resolution_clock::now(); }

void debug_timer::log_duration(const std::string& title) {
    time_point end = std::chrono::high_resolution_clock::now();
    duration dt = end - start;
    LOG_INFO("{}: {} ms", title, std::chrono::duration_cast<std::chrono::milliseconds>(dt).count());
}

#endif
