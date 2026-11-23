#ifndef MAIN_H
#define MAIN_H

#include <source_location>
#include <string>

extern bool ErrorGraphicsLoaded;

[[noreturn]] void quit();

[[noreturn]] void internal_error(const std::string& message,
                                 std::source_location loc = std::source_location::current());

#define ELMA_ASSERT(condition)                                                                     \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            internal_error("Assertion failed: " #condition);                                       \
        }                                                                                          \
    } while (0)

#endif
