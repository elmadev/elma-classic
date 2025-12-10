#include "all.h"
#include <assert.h>
#include <filesystem>
#include <regex>
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

static std::filesystem::directory_iterator current_iterator;
static std::filesystem::directory_iterator end_iterator;
static std::regex current_pattern;
static bool find_in_progress = false;

int fifirst(char* m, char* filenev) {
    if (find_in_progress) {
        hiba("fifirst");
    }

    // Extract directory and filename pattern
    std::string pattern(m);
    size_t slash_pos = pattern.find_last_of('/');
    std::string directory = (slash_pos != std::string::npos) ? pattern.substr(0, slash_pos) : ".";
    std::string file_pattern =
        (slash_pos != std::string::npos) ? pattern.substr(slash_pos + 1) : pattern;

    // Convert glob pattern to regex
    file_pattern = std::regex_replace(file_pattern, std::regex("\\."), "\\.");
    file_pattern = std::regex_replace(file_pattern, std::regex("\\*"), ".*");
    current_pattern = std::regex(file_pattern, std::regex_constants::icase);

    try {
        current_iterator = std::filesystem::directory_iterator(directory);
        find_in_progress = true;
        return finext(filenev);
    } catch (const std::filesystem::filesystem_error&) {
        return 1;
    }
}

int finext(char* filenev) {
    if (!find_in_progress) {
        hiba("8794t");
    }

    filenev[0] = 0;

    while (current_iterator != end_iterator) {
        const auto& entry = *current_iterator;

        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();

            if (std::regex_match(filename, current_pattern)) {
                strncpy(filenev, filename.c_str(), filename.length());
                filenev[filename.length()] = 0;
                ++current_iterator;
                return 0;
            }
        }

        ++current_iterator;
    }

    return 1;
}

void ficlose() {
    if (!find_in_progress) {
        hiba("908uy6");
    }
    find_in_progress = false;
    current_iterator = end_iterator;
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
