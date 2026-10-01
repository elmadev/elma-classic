#include "api/deserialize.h"
#include "api/types.h"

#define JSON_DIAGNOSTICS 1
#include <nlohmann/json.hpp>
using json = nlohmann::ordered_json;

template <typename T>
std::pair<T, std::string> deserialize(const std::vector<unsigned char>& data) {
    json j = json::parse(data, nullptr, false);
    if (j.is_discarded()) {
        return {T{}, "invalid json file"};
    }

    return {j.get<T>(), ""};
}
