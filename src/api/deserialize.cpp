#include "api/deserialize.h"
#include "api/types.h"

#define JSON_DIAGNOSTICS 1
#include <nlohmann/json.hpp>
using json = nlohmann::ordered_json;

namespace api_type {

void from_json(const json& j, lgr_info_entry& v) {
    j.at("LGRName").get_to(v.name);
    j.at("CRC").get_to(v.crc);

    v.low_quality = false;
    for (const json& tag : j.at("Tags")) {
        if (tag.at("Name") == "Low Quality") {
            v.low_quality = true;
        }
    }
};

} // namespace api_type

template <typename T>
std::pair<T, std::string> deserialize(const std::vector<unsigned char>& data) {
    json j = json::parse(data, nullptr, false);
    if (j.is_discarded()) {
        return {T{}, "invalid json file"};
    }

    return {j.get<T>(), ""};
}
template std::pair<api_type::lgr_info, std::string>
deserialize(const std::vector<unsigned char>& data);
