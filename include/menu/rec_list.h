#ifndef REC_LIST_H
#define REC_LIST_H

#include <string>
#include <vector>

namespace rec_list {

// Scan rec/ folder and return all .rec filenames
std::vector<std::string> get_replays();

} // namespace rec_list

#endif
