#include "util/util.h"
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <format>

namespace util::random {

void seed() { srand((unsigned int)clock()); }

int range(int maximum) { return rand() % maximum; }

unsigned int uint32() {
    unsigned int result = 0;
    for (int i = 0; i < 4; i++) {
        result = (result << 8) | (rand() & 0xFF);
    }
    return result;
}

} // namespace util::random

namespace util::text {

bool is_ascii_char(unsigned char c) { return (c >= 32 && c < 127); }

bool is_filename_char(unsigned char c) {
    if (c == '\\' || c == '/' || c == ':' || c == '*' || c == '?' || c == '\"' || c == '<' ||
        c == '>' || c == '|') {
        return false;
    }

    return is_ascii_char(c);
}

void centiseconds_to_string(int time, char* text, bool show_hours, bool compact) {
    assert(time >= 0 && "centiseconds_to_string time < 0!");

    int centiseconds = int(time % 100);
    time /= 100;

    int seconds = int(time % 60);
    time /= 60;

    int minutes = int(time % 60);
    time /= 60;

    int hours = 0;
    if (time > 0) {
        if (show_hours) {
            hours = time;
        } else {
            // Cap to 59:59:99 if hours disallowed
            minutes = 59;
            seconds = 59;
            centiseconds = 99;
        }
    }

    text[0] = '\0';
    char* it = text;

    if (compact) {
        if (hours > 0) {
            it = std::format_to(it, "{}:{:02}:{:02}:{:02}", hours, minutes, seconds, centiseconds);
        } else if (minutes > 0) {
            it = std::format_to(it, "{}:{:02}:{:02}", minutes, seconds, centiseconds);
        } else {
            it = std::format_to(it, "{}:{:02}", seconds, centiseconds);
        }

        *it = '\0';
        return;
    }

    if (show_hours) {
        it = std::format_to(it, "{:02}:{:02}:{:02}:{:02}", hours, minutes, seconds, centiseconds);
    } else {
        it = std::format_to(it, "{:02}:{:02}:{:02}", minutes, seconds, centiseconds);
    }

    *it = '\0';
}

} // namespace util::text
