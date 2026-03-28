#include "menu/rec_list.h"
#include "fs_utils.h"
#include "recorder.h"

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

std::vector<std::string> rec_list::replays_for_level(int level_id) {
    std::vector<std::string> result;
    for (const auto& filename : get_replays()) {
        auto header = recorder::read_header(filename);
        if (header && header->level_id == level_id) {
            result.push_back(filename);
        }
    }
    return result;
}
