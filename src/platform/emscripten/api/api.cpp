#include "platform/api.h"
#include "eol/settings.h"
#include <emscripten.h>
#include <format>

namespace {

std::string url_encode(const std::string& value) {
    constexpr char HEX[] = "0123456789ABCDEF";

    std::string result;
    result.reserve(value.size());

    for (unsigned char c : value) {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            result += static_cast<char>(c);
        } else {
            result += '%';
            result += HEX[c >> 4];
            result += HEX[c & 0x0F];
        }
    }
    return result;
}

std::optional<std::string> fetch_file(const std::string& url, const std::string& file) {
    if (emscripten_wget(url.c_str(), file.c_str()) == 1) {
        return std::format("Failed to download {} to {}", url, file);
    }
    return std::nullopt;
}

} // namespace

namespace eol_api {

void init() {}

void cleanup() {}

#ifdef DEBUG
// Avoid incrementing the download counter at https://elma.online/lgrs
#define LGR_GET_DL ""
#else
#define LGR_GET_DL "?dl"
#endif

// https://api.elma.online/api/lgr/get/LGRNAME.lgr?dl
std::optional<std::string> lgr_get(const std::string& lgr_name) {
    std::string url = std::format("https://{}/api/lgr/get/{}" LGR_GET_DL, EolSettings->api_name(),
                                  url_encode(lgr_name));
    std::string dest = std::format("lgr/{}.lgr", lgr_name);
    return fetch_file(url, dest);
}

} // namespace eol_api
