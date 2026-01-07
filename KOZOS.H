#ifndef KOZOS_H
#define KOZOS_H

#ifndef _WIN32
#include <sys/time.h>
#endif

constexpr double PI = 3.141592;
constexpr double HALF_PI = PI * 0.5;
constexpr double TWO_PI = PI * 2.0;

#define _fstrnicmp _strnicmp

#ifdef _WIN32
#define itoa _itoa
#define strcmpi _strcmpi
#define strlwr _strlwr
#define access _access
#define strupr _strupr
#endif

typedef char finame[20];
bool find_first(const char* pattern, char* filename_dest);
bool find_next(char* filename_dest);
void find_close();

#ifndef _WIN32

void itoa(int value, char* str, int base);

int strcmpi(const char* a, const char* b);
int strnicmp(const char* a, const char* b, size_t len);
#define _strnicmp strnicmp

void strupr(char* str);
void strlwr(char* str);

static inline long long timeGetTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
#endif

#endif
