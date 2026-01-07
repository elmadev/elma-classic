#include "fs_utils.h"
#include "main.h"
#include "platform_utils.h"
#include <cstring>
#include <filesystem>

static std::filesystem::directory_iterator CurrentIterator;
static std::filesystem::directory_iterator EndIterator;
static std::string CurrentExt;
static bool FindInProgress = false;

bool find_first(const char* pattern, char* filename_dest) {
    if (FindInProgress) {
        internal_error("Called find_first while a find is already in progress!");
    }

    FindInProgress = true;
    std::string directory = std::filesystem::path(pattern).parent_path().generic_string();
    CurrentExt = std::filesystem::path(pattern).extension().generic_string();
    try {
        CurrentIterator = std::filesystem::directory_iterator(directory);
    } catch (const std::filesystem::filesystem_error&) {
        return true;
    }

    return find_next(filename_dest);
}

bool find_next(char* filename_dest) {
    if (!FindInProgress) {
        internal_error("Called find_next while no find is in progress!");
    }

    while (CurrentIterator != EndIterator) {
        const std::filesystem::directory_entry& entry = *CurrentIterator;

        if (entry.is_regular_file()) {
            const std::filesystem::path& path = entry.path();
            if (strcmpi(path.extension().generic_string().c_str(), CurrentExt.c_str()) == 0) {
                const std::string filename = path.filename().generic_string();
                if (path.stem().generic_string().size() <= MAX_FILENAME_LEN) {
                    strcpy(filename_dest, filename.c_str());
                    ++CurrentIterator;
                    return false;
                }
            }
        }

        ++CurrentIterator;
    }

    return true;
}

void find_close() {
    if (!FindInProgress) {
        internal_error("Called ficlose while no find is in progress!");
    }
    FindInProgress = false;
    CurrentIterator = EndIterator;
}
