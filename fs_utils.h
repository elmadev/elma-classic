#ifndef FS_UTILS_H
#define FS_UTILS_H

constexpr int MAX_FILENAME_LEN = 8;

typedef char finame[20];
bool find_first(const char* pattern, char* filename_dest);
bool find_next(char* filename_dest);
void find_close();

#endif
