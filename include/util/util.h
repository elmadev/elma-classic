#ifndef UTIL_UTIL_H
#define UTIL_UTIL_H

#include <optional>

constexpr double PI = 3.141592;

namespace util::random {

void seed();
int range(int maximum);
unsigned int uint32();

} // namespace util::random

namespace util::text {

bool is_ascii_char(unsigned char c);

// Get whether `c` is an ASCII digit from '0' to '9'.
bool is_ascii_digit(unsigned char c);

// Get `c` converted to an integer.
// Returns a value when `c` is an ASCII base-10 digit.
std::optional<int> parse_ascii_digit(char c);

bool is_filename_char(unsigned char c);
// Format time as 00:00:00 (or 00:00:00:00 if hours are allowed)
// If `compact` is true, formats as: x:xx or x:xx:xx.
void centiseconds_to_string(int time, char* text, bool show_hours = false, bool compact = false);

// Length of the common case-insensitive prefix of two C strings.
size_t common_prefix_len(const char* a, const char* b);

} // namespace util::text

#endif
