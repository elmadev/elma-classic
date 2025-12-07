#ifndef _WIN32
#include <sys/time.h>
#endif

extern double K_pi;
extern double K_pip2;
extern double K_2pi;

#ifdef W_95
#define _fstrnicmp _strnicmp
#endif

typedef char finame[20];
int fifirst( char* minta, char* filenev );
int finext( char* filenev );
void ficlose( void );

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
