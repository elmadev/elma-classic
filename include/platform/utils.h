#ifndef PLATFORM_UTILS_H
#define PLATFORM_UTILS_H

#ifndef _WIN32
#include <unistd.h> // IWYU pragma: export
#else
#include <io.h> // IWYU pragma: export
#endif

#ifdef _WIN32
#define itoa _itoa
#define strcmpi _strcmpi
#define strncmpi _strncmpi
#define strlwr _strlwr
#define access _access
#endif

#ifndef _WIN32

void itoa(int value, char* str, int base);

int strcmpi(const char* a, const char* b);
int strnicmp(const char* a, const char* b, size_t len);

void strlwr(char* str);

#endif

#endif
