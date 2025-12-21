#ifndef KOZOS_H
#define KOZOS_H

#ifndef _WIN32
#include <sys/time.h>
#endif

extern double PI;
extern double HALF_PI;
extern double TWO_PI;

#define _fstrnicmp _strnicmp

#ifdef _WIN32
#define itoa _itoa
#define strcmpi _strcmpi
#define strlwr _strlwr
#define access _access
#define strupr _strupr
#endif

typedef char finame[20];
int find_first(const char* pattern, char* filename_dest);
int find_next(char* filename_dest);
void find_close(void);

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
