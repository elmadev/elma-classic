#include "ALL.H"
#ifndef _WIN32
#include <assert.h>
#include <filesystem>
#include <cstring>
#include <string>
#endif

double K_pi = 3.141592;
double K_pip2 = K_pi * 0.5;
double K_2pi = K_pi * 2.0;

void k_memcpy(void* dest, const void* src, size_t n) {
    // inkremental();
    memcpy(dest, src, n);
}

#ifndef _WIN32
constexpr int MAX_FILENAME_LEN = 8;
static std::filesystem::directory_iterator CurrentIterator;
static std::filesystem::directory_iterator EndIterator;
static std::string CurrentExt;
static bool FindInProgress = false;

int fifirst(const char* minta, char* filenev) {
    if (FindInProgress) {
        hiba("Called fifirst while a find is already in progress!");
    }

    FindInProgress = true;
    std::string directory = std::filesystem::path(minta).parent_path().generic_string();
    CurrentExt = std::filesystem::path(minta).extension().generic_string();
    try {
        CurrentIterator = std::filesystem::directory_iterator(directory);
    } catch (const std::filesystem::filesystem_error&) {
        return 1;
    }

    return finext(filenev);
}

int finext(char* filenev) {
    if (!FindInProgress) {
        hiba("Called finext while no find is in progress!");
    }

    while (CurrentIterator != EndIterator) {
        const std::filesystem::directory_entry& entry = *CurrentIterator;

        if (entry.is_regular_file()) {
            const std::filesystem::path& path = entry.path();
            if (strcmpi(path.extension().generic_string().c_str(), CurrentExt.c_str()) == 0) {
                const std::string filename = path.filename().generic_string();
                if (path.stem().generic_string().size() <= MAX_FILENAME_LEN) {
                    strcpy(filenev, filename.c_str());
                    ++CurrentIterator;
                    return 0;
                }
            }
        }

        ++CurrentIterator;
    }

    return 1;
}

void ficlose(void) {
    if (!FindInProgress) {
        hiba("Called ficlose while no find is in progress!");
    }
    FindInProgress = false;
    CurrentIterator = EndIterator;
}

#else

static int Fiben = 0;
static intptr_t Fihandler = -1;

int fifirst(const char* minta, char* filenev) {
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
