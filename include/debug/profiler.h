#ifndef PROFILER_H
#define PROFILER_H

#ifdef PROFILE_PERFORMANCE

#include <chrono>
#include <string>

using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;
using duration = std::chrono::high_resolution_clock::duration;

class debug_timer {
  private:
    time_point start;

  public:
    debug_timer();
    void print_duration(const std::string& title);
};

#define START_TIME(VAR) debug_timer VAR;
#define END_TIME(VAR, TITLE) VAR.print_duration(TITLE);

#else

#define START_TIME(VAR)
#define END_TIME(VAR, TITLE)

#endif

#endif
