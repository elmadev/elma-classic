#ifndef REC_LIST_H
#define REC_LIST_H

#include <string>
#include <vector>

namespace rec_list {

// Build cache of replay headers in the background
void build_cache();
// Check if the background cache has finished building
bool is_cache_ready();
// Scan rec/ folder and return all .rec filenames
std::vector<std::string> get_replays();
// Return filenames for replays matching a given level_id
std::vector<std::string> replays_for_level(int level_id);
// Notify that a new replay was saved so the cache can be updated
void add_new_replay(const std::string& filename, int level_id);

} // namespace rec_list

#endif
