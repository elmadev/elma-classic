#ifndef MAIN_H
#define MAIN_H

#include <source_location>
#include <string>

constexpr double STOPWATCH_MULTIPLIER = 0.182;
extern bool ErrorGraphicsLoaded;

[[noreturn]] void quit();

double stopwatch();
void stopwatch_reset();
void delay(int milliseconds);

[[noreturn]] void internal_error(const std::string& message,
                                 std::source_location loc = std::source_location::current());
[[noreturn]] void external_error(const std::string& message,
                                 std::source_location loc = std::source_location::current());

#endif
