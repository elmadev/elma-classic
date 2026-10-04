#ifndef GAME_GHOST_LIST_H
#define GAME_GHOST_LIST_H

#include "game/replay_bike.h"
#include <string>
#include <vector>

struct driver;

class ghost_list {
  public:
    void clear() { ghosts.clear(); }
    // Forget the ghosts and remember the replay now being followed
    void reset_to(const std::string& followed_name);
    // Load the first bike of `path` as a ghost if it is a replay of `level_id`.
    // The shirt is bmp/<nick>.bmp, none when the nick is empty.
    bool add(const std::string& path, int level_id, const std::string& nick = "");
    bool empty() const { return ghosts.empty(); }

    void rewind();
    // Returns true once every ghost has run out of frames
    bool advance(double time, bool rewinding);

    // Swap the followed replay with the next (or previous) ghost
    void swap_followed(driver& driv, bool next);
    const std::string& followed_name() const { return followed_name_; }

    const std::vector<replay_bike>& all() const { return ghosts; }

  private:
    std::vector<replay_bike> ghosts;
    std::string followed_name_;
};

extern ghost_list Ghosts;

#endif
