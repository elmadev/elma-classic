#ifndef FS_UTILS_H
#define FS_UTILS_H

bool find_first(const char* pattern, char* filename_dest, int max_name_len);
bool find_next(char* filename_dest);
void find_close();

#endif
