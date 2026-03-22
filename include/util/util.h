#ifndef UTIL_UTIL_H
#define UTIL_UTIL_H

constexpr double PI = 3.141592;

namespace util::random {

void seed();
int range(int maximum);
unsigned int uint32();

} // namespace util::random

namespace util::text {

bool is_ascii_char(unsigned char c);
bool is_filename_char(unsigned char c);
// Format time as 00:00:00 (or 00:00:00:00 if hours are allowed)
// If `compact` is true, formats as: x:xx or x:xx:xx.
void centiseconds_to_string(int time, char* text, bool show_hours = false, bool compact = false);

} // namespace util::text

#endif
