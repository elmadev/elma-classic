#ifdef __EMSCRIPTEN__

#include "platform/api/api.h"
#include "eol/settings.h"
#include <emscripten/fetch.h>
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
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
    std::string url = std::format("https://{}/api/lgr/get/{}" LGR_GET_DL, EolSettings->api_name(),
                                  url_encode(lgr_name));
    emscripten_fetch_t* fetch = emscripten_fetch(&attr, url.c_str());
    if (fetch->status == 200) {
        std::string dest = std::format("lgr/{}.lgr", lgr_name);
        if (FILE* file = fopen(dest.c_str(), "wb")) {
            fwrite(fetch->data, 1, fetch->numBytes, file);
            fclose(file);
        }
        emscripten_fetch_close(fetch);
        return std::nullopt;
    }
    emscripten_fetch_close(fetch);
    return std::format("Downloading %s failed, HTTP failure status code: %d.", fetch->url,
                       fetch->status);
}

} // namespace eol_api

#endif
