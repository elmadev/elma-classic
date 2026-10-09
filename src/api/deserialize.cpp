#include "api/deserialize.h"
#include "api/types.h"

#define JSON_DIAGNOSTICS 1
#include <nlohmann/json.hpp>
using json = nlohmann::ordered_json;

// Deserialize 3rd-party types
// https://json.nlohmann.me/features/arbitrary_types/#how-do-i-convert-third-party-types
NLOHMANN_JSON_NAMESPACE_BEGIN
template <typename T> struct adl_serializer<std::optional<T>> {
    static void from_json(const json& j, std::optional<T>& v) {
        if (j.is_null()) {
            v = std::nullopt;
        } else {
            v = j.get<T>();
        }
    }
};

template <typename T> struct adl_serializer<std::set<T>> {
    static void from_json(const json& j, std::set<T>& v) {
        for (const json& element : j) {
            v.insert(element.get<T>());
        }
    }
};

template <> struct adl_serializer<api_type::unix_time> {
    static void from_json(const json& j, api_type::unix_time& v) {
        v = api_type::unix_time{std::chrono::seconds{std::stoll(j.get<std::string>())}};
    }
};
NLOHMANN_JSON_NAMESPACE_END

// Deserialize our own types
// https://json.nlohmann.me/features/arbitrary_types/#simplify-your-life-with-macros
namespace api_type {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(team_data, TeamIndex, Team, Logo);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(kuski, Kuski, TeamData);

// Unknown enum values in SERIALIZE_ENUM use first enum value in the list
NLOHMANN_JSON_SERIALIZE_ENUM(tag::TagType, {
                                               {tag::TagType::Unknown, 0},
                                               {tag::TagType::replay, "replay"},
                                               {tag::TagType::level, "level"},
                                               {tag::TagType::levelpack, "levelpack"},
                                               {tag::TagType::lgr, "lgr"},
                                           })

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(tag, TagIndex, Name, Hidden, Type);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(lgr, Added, LGRIndex, LGRName, LGRDesc, KuskiIndex, Downloads,
                                   FileLink, PreviewLink, ReplayUUID, CRC, KuskiData, Tags);

} // namespace api_type

template <typename T>
std::pair<T, std::string> deserialize(const std::vector<unsigned char>& data) {
    json j = json::parse(data, nullptr, false);
    if (j.is_discarded()) {
        return {T{}, "invalid json file"};
    }

    return {j.get<T>(), ""};
}

// Entry points
template std::pair<api_type::lgr_info, std::string>
deserialize(const std::vector<unsigned char>& data);
