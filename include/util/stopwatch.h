#ifndef UTIL_STOPWATCH_H
#define UTIL_STOPWATCH_H

constexpr double STOPWATCH_MULTIPLIER = 0.182;
constexpr double STOPWATCH_TO_PHYS_TIME = 0.0024;

double stopwatch();
void stopwatch_reset();
void delay(int milliseconds);

#endif
