#include "eol_settings.h"
#include "lgr.h"
#include "main.h"
#include "menu_pic.h"
#include "state.h"
#include "physics_init.h"
#include "platform_impl.h"
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

template <typename T> Default<T>::operator T() const { return value; }

template <typename T> Default<T>& Default<T>::operator=(T v) {
    value = v;
    return *this;
}

template <typename T> void Default<T>::reset() { value = def; }

template <typename T> Clamp<T>::operator T() const { return value; }

template <typename T> Clamp<T>& Clamp<T>::operator=(T v) {
    value = (v < min) ? min : (v > max ? max : v);
    return *this;
}

template <typename T> void Clamp<T>::reset() { value = def; }

template struct Default<bool>;
template struct Default<MapAlignment>;
template struct Default<RendererType>;
template struct Default<DikScancode>;
template struct Clamp<int>;
template struct Clamp<double>;

void eol_settings::set_screen_width(int w) { screen_width_ = w; }

void eol_settings::set_screen_height(int h) { screen_height_ = h; }

void eol_settings::set_pictures_in_background(bool b) { pictures_in_background_ = b; }

void eol_settings::set_center_camera(bool b) { center_camera_ = b; }

void eol_settings::set_center_map(bool b) { center_map_ = b; }

void eol_settings::set_map_alignment(MapAlignment m) { map_alignment_ = m; }

void eol_settings::set_renderer(RendererType r) {
    if (renderer_ == r) {
        return;
    }

    renderer_ = r;
    if (has_window()) {
        platform_recreate_window();
        MenuPalette->set();
    }
}

void eol_settings::set_zoom(double z) {
    if (z != zoom_) {
        zoom_ = z;
        set_zoom_factor();
        invalidate_lgr_cache();
    }
}

void eol_settings::set_zoom_textures(bool zoom_textures) {
    zoom_textures_ = zoom_textures;

    invalidate_lgr_cache();
}

void eol_settings::set_turn_time(double t) { turn_time_ = t; }

void eol_settings::set_lctrl_search(bool lctrl_search) { lctrl_search_ = lctrl_search; }

void eol_settings::set_alovolt_key_player_a(DikScancode key) { alovolt_key_player_a_ = key; }

void eol_settings::set_alovolt_key_player_b(DikScancode key) { alovolt_key_player_b_ = key; }

void eol_settings::set_brake_alias_key_player_a(DikScancode key) {
    brake_alias_key_player_a_ = key;
}

void eol_settings::set_brake_alias_key_player_b(DikScancode key) {
    brake_alias_key_player_b_ = key;
}

void eol_settings::set_escape_alias_key(DikScancode key) { escape_alias_key_ = key; }

void eol_settings::set_replay_fast_2x_key(DikScancode key) { replay_fast_2x_key_ = key; }

void eol_settings::set_replay_fast_4x_key(DikScancode key) { replay_fast_4x_key_ = key; }

void eol_settings::set_replay_fast_8x_key(DikScancode key) { replay_fast_8x_key_ = key; }

void eol_settings::set_replay_slow_2x_key(DikScancode key) { replay_slow_2x_key_ = key; }

void eol_settings::set_replay_slow_4x_key(DikScancode key) { replay_slow_4x_key_ = key; }

void eol_settings::set_replay_pause_key(DikScancode key) { replay_pause_key_ = key; }

/*
 * This uses the nlohmann json library to (de)serialise `eol_settings` to json.
 *
 * from_json() / to_json() can be overloaded to provide custom (de)serialisation for types.
 *
 * `FIELD_LIST` is a list of all the fields from `eol_settings` to be put into the json.
 * `JSON_FIELD` handles serialization through getter/setter methods, allowing validation
 * and constraints to be applied. These macros are used to avoid repeating code.
 *
 * The value for a missing field when reading the json is the default value set by the
 * `eol_settings` constructor.
 */

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
    JSON_FIELD(zoom)                                                                               \
    JSON_FIELD(zoom_textures)                                                                      \
    JSON_FIELD(renderer)                                                                           \
    JSON_FIELD(turn_time)                                                                          \
    JSON_FIELD(lctrl_search)                                                                       \
    JSON_FIELD(alovolt_key_player_a)                                                               \
    JSON_FIELD(alovolt_key_player_b)                                                               \
    JSON_FIELD(brake_alias_key_player_a)                                                           \
    JSON_FIELD(brake_alias_key_player_b)                                                           \
    JSON_FIELD(escape_alias_key)                                                                   \
    JSON_FIELD(replay_fast_2x_key)                                                                 \
    JSON_FIELD(replay_fast_4x_key)                                                                 \
    JSON_FIELD(replay_fast_8x_key)                                                                 \
    JSON_FIELD(replay_slow_2x_key)                                                                 \
    JSON_FIELD(replay_slow_4x_key)                                                                 \
    JSON_FIELD(replay_pause_key)

#define JSON_FIELD(name) {#name, s.name()},
void to_json(json& j, const eol_settings& s) { j = json{FIELD_LIST}; }
#undef JSON_FIELD

#define JSON_FIELD(name)                                                                           \
    {                                                                                              \
        try {                                                                                      \
            decltype(s.name()) name;                                                               \
            name = j.value(#name, s.name());                                                       \
            s.set_##name(name);                                                                    \
        } catch (json::exception & e) {                                                            \
            external_error("Invalid parameter in " SETTINGS_JSON "!", e.what());                   \
        } catch (const char* e) {                                                                  \
            external_error("Invalid parameter in " SETTINGS_JSON "!", e);                          \
        }                                                                                          \
    }
void from_json(const json& j, eol_settings& s) { FIELD_LIST }
#undef JSON_FIELD

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

void eol_settings::sync_controls_to_state(state* s) {
    if (!s) {
        return;
    }

    s->keys1.alovolt = EolSettings->alovolt_key_player_a();
    s->keys2.alovolt = EolSettings->alovolt_key_player_b();
    s->keys1.brake_alias = EolSettings->brake_alias_key_player_a();
    s->keys2.brake_alias = EolSettings->brake_alias_key_player_b();
    s->key_escape_alias = EolSettings->escape_alias_key();
    s->key_replay_fast_2x = EolSettings->replay_fast_2x_key();
    s->key_replay_fast_4x = EolSettings->replay_fast_4x_key();
    s->key_replay_fast_8x = EolSettings->replay_fast_8x_key();
    s->key_replay_slow_2x = EolSettings->replay_slow_2x_key();
    s->key_replay_slow_4x = EolSettings->replay_slow_4x_key();
    s->key_replay_pause = EolSettings->replay_pause_key();
}

void eol_settings::sync_controls_from_state(state* s) {
    if (!s) {
        return;
    }

    EolSettings->set_alovolt_key_player_a(s->keys1.alovolt);
    EolSettings->set_alovolt_key_player_b(s->keys2.alovolt);
    EolSettings->set_brake_alias_key_player_a(s->keys1.brake_alias);
    EolSettings->set_brake_alias_key_player_b(s->keys2.brake_alias);
    EolSettings->set_escape_alias_key(s->key_escape_alias);
    EolSettings->set_replay_fast_2x_key(s->key_replay_fast_2x);
    EolSettings->set_replay_fast_4x_key(s->key_replay_fast_4x);
    EolSettings->set_replay_fast_8x_key(s->key_replay_fast_8x);
    EolSettings->set_replay_slow_2x_key(s->key_replay_slow_2x);
    EolSettings->set_replay_slow_4x_key(s->key_replay_slow_4x);
    EolSettings->set_replay_pause_key(s->key_replay_pause);
}
