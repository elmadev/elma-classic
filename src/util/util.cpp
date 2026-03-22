#include "util/util.h"
#include <cstdlib>
#include <ctime>

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

} // namespace util::text
