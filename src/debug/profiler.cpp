#ifdef PROFILE_PERFORMANCE

#include "debug/profiler.h"
#include <iostream>
#include <print>

debug_timer::debug_timer() { start = std::chrono::high_resolution_clock::now(); }

void debug_timer::print_duration(const std::string& title) {
    time_point end = std::chrono::high_resolution_clock::now();
    duration dt = end - start;
    std::cout << std::format("{}: {} ms\n", title,
                             std::chrono::duration_cast<std::chrono::milliseconds>(dt).count());
}

#endif
