#ifndef FS_UTILS_H
#define FS_UTILS_H

constexpr int MAX_FILENAME_LEN = 8;
constexpr int MAX_FILENAME_EXT_LEN = MAX_FILENAME_LEN + 4;  // "FILENAME.EXT" = 12
constexpr int MAX_FILE_PATH_LEN = 4 + MAX_FILENAME_EXT_LEN; // "dir/FILENAME.EXT" = 16

constexpr int MAX_REPLAY_NAME_LEN = 15;
constexpr int MAX_REPLAY_EXT_LEN = MAX_REPLAY_NAME_LEN + 4; // "replayname.rec" = 19
constexpr int MAX_REPLAY_PATH_LEN = 4 + MAX_REPLAY_EXT_LEN; // "rec/replayname.rec" = 23

using finame = char[MAX_FILENAME_EXT_LEN + 1];
using filepath = char[MAX_FILE_PATH_LEN + 1];
using recname = char[MAX_REPLAY_EXT_LEN + 1];
using recpath = char[MAX_REPLAY_PATH_LEN + 1];

bool find_first(const char* pattern, char* filename_dest, int max_name_len = MAX_FILENAME_LEN);
bool find_next(char* filename_dest);
void find_close();

#endif
