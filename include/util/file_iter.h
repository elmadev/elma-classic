#ifndef FILE_ITER_H
#define FILE_ITER_H

bool find_first(const char* pattern, char* filename_dest, int max_name_len);
bool find_next(char* filename_dest);
void find_close();

#endif
