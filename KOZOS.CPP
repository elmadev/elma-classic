#include "ALL.H"
#include <filesystem>
#ifndef _WIN32
#include <assert.h>
#include <cstring>
#include <string>
#endif

double K_pi = 3.141592;
double K_pip2 = K_pi * 0.5;
double K_2pi = K_pi * 2.0;

constexpr int MAX_FILENAME_LEN = 8;
static std::filesystem::directory_iterator CurrentIterator;
static std::filesystem::directory_iterator EndIterator;
static std::string CurrentExt;
static bool FindInProgress = false;

int fifirst(const char* minta, char* filenev) {
    if (FindInProgress) {
        internal_error("Called fifirst while a find is already in progress!");
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
        internal_error("Called finext while no find is in progress!");
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
        internal_error("Called ficlose while no find is in progress!");
    }
    FindInProgress = false;
    CurrentIterator = EndIterator;
}

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
