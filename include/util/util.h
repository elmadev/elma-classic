#ifndef UTIL_UTIL_H
#define UTIL_UTIL_H

#include <cstdio>
#include <optional>

constexpr int MAX_FILENAME_LEN = 8;
constexpr int MAX_FILENAME_EXT_LEN = 15; // binary formats allow up to 15 chars
constexpr int MAX_FILE_PATH_LEN = 4 + MAX_FILENAME_EXT_LEN + 4; // "lgr/" + name + ".lgr" = 23

constexpr int MAX_REPLAY_NAME_LEN = 15;
constexpr int MAX_REPLAY_EXT_LEN = MAX_REPLAY_NAME_LEN + 4; // "replayname.rec" = 19
constexpr int MAX_REPLAY_PATH_LEN = 4 + MAX_REPLAY_EXT_LEN; // "rec/replayname.rec" = 23

using finame = char[MAX_FILENAME_EXT_LEN + 1];
using filepath = char[MAX_FILE_PATH_LEN + 1];
using recname = char[MAX_REPLAY_EXT_LEN + 1];
using recpath = char[MAX_REPLAY_PATH_LEN + 1];

constexpr double PI = 3.141592;

namespace util::math {

// fmod with the result wrapped into [0, mod).
double positive_fmod(double val, double mod);

} // namespace util::math

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

// fwrite a char array without leaking data after the first \0
void fwrite_array(const char* buffer, size_t size, FILE* h);

} // namespace util::text

#endif
