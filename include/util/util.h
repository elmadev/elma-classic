#ifndef UTIL_UTIL_H
#define UTIL_UTIL_H

namespace util::random {

void seed();
int range(int maximum);
unsigned int uint32();

} // namespace util::random

namespace util::text {

bool is_ascii_char(unsigned char c);
bool is_filename_char(unsigned char c);

} // namespace util::text

#endif
