#ifndef TIMER_H
#define TIMER_H

constexpr double TimeToCentiseconds = 100.0 / (182.0 * 0.0024);

void draw_timers(const char* best_time_text, double flag_tag_time, double current_time, pic8* dest,
                 int dest_width, int dest_height);

#endif
