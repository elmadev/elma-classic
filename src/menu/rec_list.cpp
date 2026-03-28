#include "menu/rec_list.h"
#include "fs_utils.h"

std::vector<std::string> rec_list::get_replays() {
    std::vector<std::string> filenames;
    recname filename;
    bool done = find_first("rec/*.rec", filename, MAX_REPLAY_NAME_LEN);
    while (!done) {
        filenames.emplace_back(filename);
        done = find_next(filename);
    }
    find_close();
    return filenames;
}
