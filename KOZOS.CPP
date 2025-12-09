#include "all.h"
#include <assert.h>
#ifndef _WIN32
#include <string>
#endif

double K_pi = 3.141592;
double K_pip2 = K_pi * 0.5;
double K_2pi = K_pi * 2.0;

void k_memcpy(void* dest, const void* src, size_t n) {
    // inkremental();
    memcpy(dest, src, n);
}

static int Fiben = 0;

#ifndef _WIN32
int fifirst(char* minta, char* filenev) { return 1; }
int finext(char* filenev) { return 1; }
void ficlose(void) {}

#else

/*struct _finddata_t c_file;
    long hFile;

    // Find first .c file in current directory
    if( (hFile = _findfirst( "*.c", &c_file )) == -1L )
       printf( "No *.c files in current directory!\n" );
   else
   {
            printf( "Listing of .c files\n\n" );
            printf( "\nRDO HID SYS ARC  FILE         DATE %25c SIZE\n", ' ' );
            printf( "--- --- --- ---  ----         ---- %25c ----\n", ' ' );
            printf( ( c_file.attrib & _A_RDONLY ) ? " Y  " : " N  " );
            printf( ( c_file.attrib & _A_SYSTEM ) ? " Y  " : " N  " );
            printf( ( c_file.attrib & _A_HIDDEN ) ? " Y  " : " N  " );
            printf( ( c_file.attrib & _A_ARCH )   ? " Y  " : " N  " );
            printf( " %-12s %.24s  %9ld\n",
               c_file.name, ctime( &( c_file.time_write ) ), c_file.size );

            // Find the rest of the .c files
            while( _findnext( hFile, &c_file ) == 0 )
            {
                printf( ( c_file.attrib & _A_RDONLY ) ? " Y  " : " N  " );
                printf( ( c_file.attrib & _A_SYSTEM ) ? " Y  " : " N  " );
                printf( ( c_file.attrib & _A_HIDDEN ) ? " Y  " : " N  " );
                printf( ( c_file.attrib & _A_ARCH )   ? " Y  " : " N  " );
                printf( " %-12s %.24s  %9ld\n",
                   c_file.name, ctime( &( c_file.time_write ) ), c_file.size );
            }

       _findclose( hFile )
*/

static long Fihandler = -1;

int fifirst(char* minta, char* filenev) {
    if (Fiben || Fihandler != -1L) {
        hiba("fifirst");
    }
    Fiben = 1;

    filenev[0];
    struct _finddata_t c_file;
    Fihandler = _findfirst(minta, &c_file);
    if (Fihandler == -1) {
        return 1;
    }

    // Megvizsgaljuk nem tul hosszu-e nev:
    if (strlen(c_file.name) <= 12) {
        char tmpnev[20];
        strcpy(tmpnev, c_file.name);
        // Levagjuk vegerol kiterjesztest:
        if (strstr(tmpnev, ".")) {
            *strstr(tmpnev, ".") = 0;
        }
        if (strlen(tmpnev) <= 8) {
            strcpy(filenev, c_file.name);
            return 0;
        }
    }
    // Tul hosszu volt nev, tehat nezzuk tovabb:
    return finext(filenev);
}

int finext(char* filenev) {
    if (!Fiben || Fihandler == -1L) {
        hiba("8794t");
    }

    filenev[0] = 0;

    while (1) {
        struct _finddata_t c_file;
        if (_findnext(Fihandler, &c_file) != 0) {
            return 1;
        }

        // Megvizsgaljuk nem tul hosszu-e nev:
        if (strlen(c_file.name) > 12) {
            continue; // Tul hosszu nev
        }

        char tmpnev[20];
        strcpy(tmpnev, c_file.name);
        // Levagjuk vegerol kiterjesztest:
        if (strstr(tmpnev, ".")) {
            *strstr(tmpnev, ".") = 0;
        }
        if (strlen(tmpnev) > 8) {
            continue; // Tul hosszu nev
        }

        // Nev rendben van:
        strcpy(filenev, c_file.name);
        return 0;
    }
}

void ficlose(void) {
    if (!Fiben) {
        hiba("908uy6");
    }
    Fiben = 0;
    if (Fihandler != -1L) {
        _findclose(Fihandler);
        Fihandler = -1L;
    }
}

#endif

#ifndef _WIN32
void itoa(int value, char* str, int base) {
    assert(base == 10);
    std::string tmp2 = std::to_string(value);
    std::strcpy(str, tmp2.c_str());
}

int strcmpi(const char* a, const char* b) {
    char ca, cb;
    int v;
    do {
        ca = *a++;
        cb = *b++;
        v = (unsigned int)std::tolower(ca) - (unsigned int)std::tolower(cb);
    } while (!v && ca && cb);
    return v;
}

int strnicmp(const char* a, const char* b, size_t len) {
    char ca, cb;
    int v;
    do {
        ca = *a++;
        cb = *b++;
        v = (unsigned int)std::tolower(ca) - (unsigned int)std::tolower(cb);
        len--;
    } while (!v && ca && cb && len);
    return v;
}

void strupr(char* str) {
    while (*str) {
        *str = std::toupper((unsigned char)*str);
        str++;
    }
}

void strlwr(char* str) {
    while (*str) {
        *str = std::tolower((unsigned char)*str);
        str++;
    }
}
#endif
