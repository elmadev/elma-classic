#ifndef PLATFORM_UTILS_H
#define PLATFORM_UTILS_H

#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#endif

constexpr double PI = 3.141592;
constexpr double HALF_PI = PI * 0.5;
constexpr double TWO_PI = PI * 2.0;

#ifdef _WIN32
#define itoa _itoa
#define strcmpi _strcmpi
#define strncmpi _strncmpi
#define strlwr _strlwr
#define access _access
#define strupr _strupr
#endif

bool is_ascii_character(unsigned char c);
bool is_char_valid_for_filename(unsigned char c);

#ifndef _WIN32

void itoa(int value, char* str, int base);

int strcmpi(const char* a, const char* b);
int strnicmp(const char* a, const char* b, size_t len);

void strupr(char* str);
void strlwr(char* str);

#endif

#endif
