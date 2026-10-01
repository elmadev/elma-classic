#ifndef API_TYPES
#define API_TYPES

#include <string>
#include <vector>

namespace api_type {

struct lgr_info_entry {
    std::string name;
    std::string crc;
    bool low_quality;
};

using lgr_info = std::vector<lgr_info_entry>;

} // namespace api_type

#endif
