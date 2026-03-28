#ifndef REC_LIST_H
#define REC_LIST_H

#include <string>
#include <vector>

namespace rec_list {

// Scan rec/ folder and return all .rec filenames
std::vector<std::string> get_replays();
// Return filenames for replays matching a given level_id
std::vector<std::string> replays_for_level(int level_id);

} // namespace rec_list

#endif
