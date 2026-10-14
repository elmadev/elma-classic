#ifndef API_TYPES
#define API_TYPES

#include <chrono>
#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace api_type {

// References:
// https://github.com/elmadev/elmaonline-site/tree/dev/api/src/data/models
// https://sequelize.org/docs/v7/models/data-types/
// https://dev.mysql.com/doc/refman/8.4/en/data-types.html

using integer = std::int32_t;

using unix_time = std::chrono::system_clock::time_point;

struct team_data {
    integer TeamIndex;
    std::string Team;
    std::optional<std::string> Logo;
};

struct kuski {
    std::string Kuski;
    std::optional<team_data> TeamData;
};

struct tag {
    enum class TagType { Unknown, replay, level, levelpack, lgr };

    integer TagIndex;
    std::string Name;
    integer Hidden;
    TagType Type;

    bool operator<(const tag& other) const { return TagIndex < other.TagIndex; }
};

using tags = std::set<tag>;

struct lgr {
    unix_time Added;
    integer LGRIndex;
    std::string LGRName;
    std::string LGRDesc;
    integer KuskiIndex;
    integer Downloads;
    std::string FileLink;
    std::string PreviewLink;
    std::string ReplayUUID;
    std::string CRC;
    kuski KuskiData;
    tags Tags;
};

using lgr_info = std::vector<lgr>;

} // namespace api_type

#endif
