#include "eol_settings.h"
#include "LGRFILE.H"
#include "main.h"
#include "physics_init.h"
#include <fstream>
#define JSON_DIAGNOSTICS 1
#include <nlohmann/json.hpp>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

using json = nlohmann::ordered_json;

#define SETTINGS_JSON "settings.json"

eol_settings::eol_settings() {
    screen_width = 640;
    screen_height = 480;
    pictures_in_background = false;
    center_camera = false;
    center_map = false;
    map_alignment = MapAlignment::None;
    zoom_ = 1.0;
    zoom_textures_ = false;
    renderer = RendererType::Software;
}

void eol_settings::set_zoom(double z) {
    if (z != zoom_) {
        zoom_ = (z < MIN_ZOOM) ? MIN_ZOOM : (z > MAX_ZOOM ? MAX_ZOOM : z);

        set_zoom_factor();
        invalidate_lgr_cache();
    }
}

void eol_settings::set_zoom_textures(bool zoom_textures) {
    zoom_textures_ = zoom_textures;

    invalidate_lgr_cache();
}

void to_json(json& j, const MapAlignment& m) {
    switch (m) {
    case MapAlignment::None:
        j = "none";
        break;
    case MapAlignment::Left:
        j = "left";
        break;
    case MapAlignment::Middle:
        j = "middle";
        break;
    case MapAlignment::Right:
        j = "right";
        break;
    }
}

void from_json(const json& j, MapAlignment& m) {
    if (j == "none") {
        m = MapAlignment::None;
    } else if (j == "left") {
        m = MapAlignment::Left;
    } else if (j == "middle") {
        m = MapAlignment::Middle;
    } else if (j == "right") {
        m = MapAlignment::Right;
    } else {
        throw("[json.exception.type_error.302] (/map_alignment) invalid value");
    }
}

void to_json(json& j, const RendererType& r) {
    switch (r) {
    case RendererType::Software:
        j = "software";
        break;
    case RendererType::OpenGL:
        j = "opengl";
        break;
    }
}

void from_json(const json& j, RendererType& r) {
    if (j == "software") {
        r = RendererType::Software;
    } else if (j == "opengl") {
        r = RendererType::OpenGL;
    } else {
        throw("[json.exception.type_error.302] (/renderer) invalid value");
    }
}

#define FIELD_LIST                                                                                 \
    JSON_FIELD(screen_width)                                                                       \
    JSON_FIELD(screen_height)                                                                      \
    JSON_FIELD(pictures_in_background)                                                             \
    JSON_FIELD(center_camera)                                                                      \
    JSON_FIELD(center_map)                                                                         \
    JSON_FIELD(map_alignment)                                                                      \
    JSON_FIELD_PRIV(zoom)                                                                          \
    JSON_FIELD_PRIV(zoom_textures)                                                                 \
    JSON_FIELD(renderer)

#define JSON_FIELD(name) {#name, s.name},
#define JSON_FIELD_PRIV(name) {#name, s.name()},
void to_json(json& j, const eol_settings& s) { j = json{FIELD_LIST}; }
#undef JSON_FIELD
#undef JSON_FIELD_PRIV

#define JSON_FIELD(name)                                                                           \
    try {                                                                                          \
        s.name = j.value(#name, s.name);                                                           \
    } catch (json::exception & e) {                                                                \
        external_error("Invalid parameter in " SETTINGS_JSON "!", e.what());                       \
    } catch (const char* e) {                                                                      \
        external_error("Invalid parameter in " SETTINGS_JSON "!", e);                              \
    }

#define JSON_FIELD_PRIV(name)                                                                      \
    {                                                                                              \
        try {                                                                                      \
            decltype(s.name()) name;                                                               \
            name = j.value(#name, s.name());                                                       \
            s.set_##name(name);                                                                    \
        } catch (json::exception & e) {                                                            \
            external_error("Invalid parameter in " SETTINGS_JSON "!", e.what());                   \
        }                                                                                          \
    }
void from_json(const json& j, eol_settings& s) { FIELD_LIST }
#undef JSON_FIELD
#undef JSON_FIELD_PRIV

void eol_settings::read_settings() {
    if (access(SETTINGS_JSON, 0) != 0) {
        return;
    }
    std::ifstream i(SETTINGS_JSON);
    json j = json::parse(i, nullptr, false);
    if (!j.is_discarded()) {
        *EolSettings = j;
    } else {
        external_error(SETTINGS_JSON " is corrupt!", "Please fix this or delete the file!");
    }
}

void eol_settings::write_settings() {
    std::ofstream o("settings.json");
    json j = *EolSettings;
    o << std::setw(4) << j << std::endl;
}
