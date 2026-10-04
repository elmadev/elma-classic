#include "game/ghost_list.h"
#include "eol/eol.h"
#include "game/driver.h"
#include <algorithm>
#include <filesystem>
#include <utility>

ghost_list Ghosts;

void ghost_list::reset_to(const std::string& followed_name) {
    clear();
    followed_name_ = followed_name;
}

bool ghost_list::add(const std::string& path, int level_id, const std::string& nick) {
    replay_bike& ghost = ghosts.emplace_back();
    if (recorder::load_single(path, ghost.bike.rec) != level_id) {
        ghosts.pop_back();
        return false;
    }
    ghost.name = std::filesystem::path(path).stem().string();
    ghost.nick = nick;
    if (!nick.empty()) {
        ghost.shirt.reset(eol::load_shirt(nick));
    }
    return true;
}

void ghost_list::rewind() {
    for (replay_bike& g : ghosts) {
        g.bike.rec.rewind();
        g.bike.meta.reset();
    }
}

bool ghost_list::advance(double time, bool rewinding) {
    bool all_finished = true;
    for (replay_bike& g : ghosts) {
        if (g.advance(time, rewinding)) {
            all_finished = false;
        }
    }
    return all_finished;
}

// The ghosts keep their order, so cycling forward through all of them returns
// to the start.
void ghost_list::swap_followed(driver& driv, bool next) {
    replay_bike& ghost = next ? ghosts.front() : ghosts.back();
    std::swap(*driv.rec, ghost.bike.rec);
    std::swap(*driv.mot, ghost.bike.mot);
    std::swap(driv.meta, ghost.bike.meta);
    std::swap(followed_name_, ghost.name);
    if (next) {
        std::rotate(ghosts.begin(), ghosts.begin() + 1, ghosts.end());
    } else {
        std::rotate(ghosts.rbegin(), ghosts.rbegin() + 1, ghosts.rend());
    }
}
