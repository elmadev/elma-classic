#ifndef MAIN_H
#define MAIN_H

#include <source_location>
#include <string>

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
