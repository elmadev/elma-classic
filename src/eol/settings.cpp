#include "eol/settings.h"
#include "editor/editor.h"
#include "game/state.h"
#include "level/level.h"
#include "main.h"
#include "physics/init.h"
#include "pic/lgr.h"
#include "pic/surface.h"
#include "platform/implementation.h"
#include "platform/utils.h"
#include "renderer/canvas.h"
#include "renderer/object_overlay.h"
#include <fstream>
#define JSON_DIAGNOSTICS 1
#include <nlohmann/json.hpp>
#include <utility>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

using json = nlohmann::ordered_json;

#define SETTINGS_JSON "settings.json"

template <typename T> Default<T>::operator T() const { return value; }

template <typename T> Default<T>& Default<T>::operator=(T v) {
    value = std::move(v);
    return *this;
}

template <typename T> void Default<T>::mark_persisted() { persisted = value; }

template <typename T> void Default<T>::reset_to_persisted() { value = persisted; }

template <typename T> void Default<T>::reset_to_default() {
    value = def;
    persisted = def;
}

template <typename T> Clamp<T>::operator T() const { return value; }

template <typename T> Clamp<T>& Clamp<T>::operator=(T v) {
    value = (v < min) ? min : (v > max ? max : v);
    return *this;
}

template <typename T> void Clamp<T>::mark_persisted() { persisted = value; }

template <typename T> void Clamp<T>::reset_to_persisted() { value = persisted; }

template <typename T> void Clamp<T>::reset_to_default() {
    value = def;
    persisted = def;
}

template struct Default<bool>;
template struct Default<MapAlignment>;
template struct Default<RendererType>;
template struct Default<FullscreenMode>;
template struct Default<ChatVisibility>;
template struct Default<DikScancode>;
template struct Default<combo_scancode>;
template struct Default<std::string>;
template struct Default<eol_table::Align>;
template struct Clamp<int>;
template struct Clamp<double>;

void eol_settings::set_pictures_in_background(bool b) {
    if (pictures_in_background_ == b) {
        return;
    }
    pictures_in_background_ = b;
    canvas::invalidate_canvases();
}

void eol_settings::set_default_ground(bool b) {
    default_ground_ = b;
    if (Level && Lgr) {
        Lgr->reload_default_textures(*Level, SCREEN_WIDTH);
    }
}
void eol_settings::set_default_sky(bool b) {
    default_sky_ = b;
    if (Level && Lgr) {
        Lgr->reload_default_textures(*Level, SCREEN_WIDTH);
    }
}

void eol_settings::set_renderer(RendererType r) {
    if (renderer_ == r) {
        return;
    }

    renderer_ = r;
    if (has_window()) {
        platform_recreate_window();
    }
}

void eol_settings::set_fullscreen(FullscreenMode f) {
    if (fullscreen_ == f) {
        return;
    }

    fullscreen_ = f;
    if (has_window()) {
        platform_apply_fullscreen_mode();
    }
}

void eol_settings::set_zoom(double z) {
    if (z != zoom_) {
        zoom_ = z;
        set_zoom_factor();
        lgrfile::invalidate_lgr_cache();
        init_gravity_arrows();
    }
}

void eol_settings::set_minimap_zoom(double z) {
    if (z != minimap_zoom_) {
        minimap_zoom_ = z;
        set_minimap_zoom_factor();
        canvas::invalidate_canvases();
    }
}

void eol_settings::set_zoom_textures(bool zoom_textures) {
    zoom_textures_ = zoom_textures;

    lgrfile::invalidate_lgr_cache();
}

void eol_settings::set_zoom_grass(bool zoom_grass) {
    zoom_grass_ = zoom_grass;

    lgrfile::invalidate_lgr_cache();
}

void eol_settings::set_default_lgr_name(std::string name) {
    if (default_lgr_name_.value == name) {
        return;
    }

    if (!Level || !Lgr) {
        default_lgr_name_ = std::move(name);
        return;
    }

    if (strcmpi(Level->lgr_name, "default") == 0) {
        // Level uses default.lgr
        lgrfile::invalidate_lgr_cache();
    } else if (strcmpi(lgrfile::current_lgr_name(), "default") == 0) {
        // Level does not use default.lgr, but we are using default.lgr as a fallback
        lgrfile::invalidate_lgr_cache();
    } else if (strcmpi(Level->lgr_name, default_lgr_name_.value.c_str()) != 0 &&
               strcmpi(lgrfile::current_lgr_name(), default_lgr_name_.value.c_str()) == 0) {
        // Level does not use the previous default_lgr_name,
        // but we are using the previous default_lgr_name as an override
        lgrfile::invalidate_lgr_cache();
    } else {
        // The level was using a non-default LGR, don't invalidate the cache since the
        // default is not currently loaded.
    }

    default_lgr_name_ = std::move(name);
}

void eol_settings::set_fancyboost(bool b) {
    fancyboost_ = b;
    lgrfile::invalidate_lgr_cache();
}

void eol_settings::set_cripple_no_throttle(bool b) {
    cripple_no_throttle_ = b;
    if (b) {
        cripple_always_throttle_ = false;
    }
}

void eol_settings::set_cripple_always_throttle(bool b) {
    cripple_always_throttle_ = b;
    if (b) {
        cripple_no_throttle_ = false;
    }
}

void eol_settings::set_cripple_no_turn(bool b) {
    cripple_no_turn_ = b;
    if (b) {
        cripple_one_turn_ = false;
    }
}

void eol_settings::set_cripple_one_turn(bool b) {
    cripple_one_turn_ = b;
    if (b) {
        cripple_no_turn_ = false;
    }
}

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

void to_json(json& j, const eol_table::Align& a) {
    switch (a) {
    case eol_table::Align::Left:
        j = "left";
        break;
    case eol_table::Align::Center:
        j = "center";
        break;
    case eol_table::Align::Right:
        j = "right";
        break;
    }
}

void from_json(const json& j, eol_table::Align& a) {
    if (j == "left") {
        a = eol_table::Align::Left;
    } else if (j == "center") {
        a = eol_table::Align::Center;
    } else if (j == "right") {
        a = eol_table::Align::Right;
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

void to_json(json& j, const FullscreenMode& f) {
    switch (f) {
    case FullscreenMode::Windowed:
        j = "windowed";
        break;
    case FullscreenMode::Fullscreen:
        j = "fullscreen";
        break;
    case FullscreenMode::FullscreenDesktop:
        j = "fullscreen_desktop";
        break;
    }
}

void from_json(const json& j, FullscreenMode& f) {
    if (j == "windowed") {
        f = FullscreenMode::Windowed;
    } else if (j == "fullscreen") {
        f = FullscreenMode::Fullscreen;
    } else if (j == "fullscreen_desktop") {
        f = FullscreenMode::FullscreenDesktop;
    } else {
        throw("[json.exception.type_error.302] (/fullscreen) invalid value");
    }
}

void to_json(json& j, const ChatVisibility& c) {
    switch (c) {
    case ChatVisibility::Shown:
        j = "shown";
        break;
    case ChatVisibility::PublicHidden:
        j = "public_hidden";
        break;
    case ChatVisibility::Hidden:
        j = "hidden";
        break;
    }
}

void from_json(const json& j, ChatVisibility& c) {
    if (j == "shown") {
        c = ChatVisibility::Shown;
    } else if (j == "public_hidden") {
        c = ChatVisibility::PublicHidden;
    } else if (j == "hidden") {
        c = ChatVisibility::Hidden;
    } else {
        throw("[json.exception.type_error.302] (/chat_visibility) invalid value");
    }
}

void to_json(json& j, const combo_scancode& r) { j = (unsigned long long)(r); }

void from_json(const json& j, combo_scancode& r) { r = combo_scancode((unsigned long long)(j)); }

#define FIELD_LIST                                                                                 \
    JSON_FIELD(screen_width)                                                                       \
    JSON_FIELD(screen_height)                                                                      \
                                                                                                   \
    JSON_FIELD(high_quality_key)                                                                   \
    JSON_FIELD(pictures_in_background)                                                             \
    JSON_FIELD(default_ground)                                                                     \
    JSON_FIELD(default_sky)                                                                        \
    JSON_FIELD(default_ground_sky_key)                                                             \
    JSON_FIELD(zoom_out_key)                                                                       \
    JSON_FIELD(zoom_in_key)                                                                        \
    JSON_FIELD(screenshot_level_key)                                                               \
                                                                                                   \
    JSON_FIELD(center_camera)                                                                      \
    JSON_FIELD(center_map)                                                                         \
    JSON_FIELD(map_alignment)                                                                      \
    JSON_FIELD(zoom)                                                                               \
    JSON_FIELD(minimap_zoom)                                                                       \
    JSON_FIELD(zoom_textures)                                                                      \
    JSON_FIELD(zoom_grass)                                                                         \
    JSON_FIELD(renderer)                                                                           \
    JSON_FIELD(fullscreen)                                                                         \
    JSON_FIELD(turn_time)                                                                          \
    JSON_FIELD(lctrl_search)                                                                       \
                                                                                                   \
    JSON_FIELD(alovolt_key_player_a)                                                               \
    JSON_FIELD(alovolt_key_player_b)                                                               \
    JSON_FIELD(brake_alias_key_player_a)                                                           \
    JSON_FIELD(brake_alias_key_player_b)                                                           \
    JSON_FIELD(one_frame_brake_key_player_a)                                                       \
    JSON_FIELD(one_frame_brake_key_player_b)                                                       \
    JSON_FIELD(escape_alias_key)                                                                   \
                                                                                                   \
    JSON_FIELD(replay_fast_2x_key)                                                                 \
    JSON_FIELD(replay_fast_4x_key)                                                                 \
    JSON_FIELD(replay_fast_8x_key)                                                                 \
    JSON_FIELD(replay_slow_2x_key)                                                                 \
    JSON_FIELD(replay_slow_4x_key)                                                                 \
    JSON_FIELD(replay_pause_key)                                                                   \
    JSON_FIELD(replay_rewind_key)                                                                  \
                                                                                                   \
    JSON_FIELD(show_others_key)                                                                    \
    JSON_FIELD(spy_next_kuski_key)                                                                 \
    JSON_FIELD(spy_prev_kuski_key)                                                                 \
    JSON_FIELD(battle_queue_key)                                                                   \
    JSON_FIELD(download_battle_level_key)                                                          \
    JSON_FIELD(download_level_key)                                                                 \
    JSON_FIELD(players_online_key)                                                                 \
    JSON_FIELD(battle_results_key)                                                                 \
    JSON_FIELD(finished_times_key)                                                                 \
    JSON_FIELD(cycle_finished_times_filter_key)                                                    \
    JSON_FIELD(clear_finished_times_key)                                                           \
    JSON_FIELD(chat_key)                                                                           \
    JSON_FIELD(show_chat_key)                                                                      \
    JSON_FIELD(team_chat_key)                                                                      \
    JSON_FIELD(pm_next_kuski_key)                                                                  \
    JSON_FIELD(pm_prev_kuski_key)                                                                  \
    JSON_FIELD(pm_clear_kuski_key)                                                                 \
    JSON_FIELD(battle_status_key)                                                                  \
    JSON_FIELD(battle_leader_key)                                                                  \
    JSON_FIELD(speedometer_key)                                                                    \
    JSON_FIELD(reconnect_key)                                                                      \
    JSON_FIELD(disconnect_key)                                                                     \
    JSON_FIELD(toggle_one_wheel_status_key)                                                        \
    JSON_FIELD(toggle_last_apple_time_key)                                                         \
                                                                                                   \
    JSON_FIELD(default_lgr_name)                                                                   \
    JSON_FIELD(fancyboost)                                                                         \
                                                                                                   \
    JSON_FIELD(show_last_apple_time)                                                               \
    JSON_FIELD(show_lev_name_in_rec)                                                               \
    JSON_FIELD(show_gravity_arrows)                                                                \
                                                                                                   \
    JSON_FIELD(show_fps)                                                                           \
    JSON_FIELD(show_ups)                                                                           \
    JSON_FIELD(fps_limit_enabled)                                                                  \
    JSON_FIELD(fps_limit)                                                                          \
    JSON_FIELD(recording_fps)                                                                      \
                                                                                                   \
    JSON_FIELD(show_demo_menu)                                                                     \
    JSON_FIELD(show_help_menu)                                                                     \
    JSON_FIELD(show_about_menu)                                                                    \
    JSON_FIELD(show_best_times_menu)                                                               \
    JSON_FIELD(skip_intro)                                                                         \
    JSON_FIELD(still_objects)                                                                      \
    JSON_FIELD(all_internals_accessible)                                                           \
    JSON_FIELD(show_total_time)                                                                    \
    JSON_FIELD(minimap_width)                                                                      \
    JSON_FIELD(minimap_height)                                                                     \
    JSON_FIELD(minimap_opacity)                                                                    \
    JSON_FIELD(chat_lines)                                                                         \
    JSON_FIELD(cripple_no_brake)                                                                   \
    JSON_FIELD(cripple_no_throttle)                                                                \
    JSON_FIELD(cripple_always_throttle)                                                            \
    JSON_FIELD(cripple_no_turn)                                                                    \
    JSON_FIELD(cripple_no_volt)                                                                    \
    JSON_FIELD(cripple_one_turn)                                                                   \
    JSON_FIELD(cripple_drunk)                                                                      \
    JSON_FIELD(cripple_one_wheel)                                                                  \
    JSON_FIELD(show_one_wheel_status)                                                              \
    JSON_FIELD(hostname)                                                                           \
    JSON_FIELD(tcp_port)                                                                           \
    JSON_FIELD(udp_port)                                                                           \
    JSON_FIELD(nick)                                                                               \
    JSON_FIELD(password)                                                                           \
    JSON_FIELD(play_offline)                                                                       \
    JSON_FIELD(tcp_only)                                                                           \
                                                                                                   \
    JSON_FIELD(show_others)                                                                        \
    JSON_FIELD(show_battle_status)                                                                 \
    JSON_FIELD(show_battle_leader)                                                                 \
    JSON_FIELD(show_speedometer)                                                                   \
                                                                                                   \
    JSON_FIELD(table_alignment)                                                                    \
    JSON_FIELD(chat_visibility)

#define JSON_FIELD(name)                                                                           \
    void eol_settings::persist_##name(decltype(eol_settings::name##_.value) v) {                   \
        set_##name(std::move(v));                                                                  \
        name##_.mark_persisted();                                                                  \
    }
FIELD_LIST
#undef JSON_FIELD

#define JSON_FIELD(name) {#name, s.name##_persisted()},
void to_json(json& j, const eol_settings& s) { j = json{FIELD_LIST}; }
#undef JSON_FIELD

#define JSON_FIELD(name)                                                                           \
    {                                                                                              \
        try {                                                                                      \
            auto value = j.value(#name, s.name##_persisted());                                     \
            s.persist_##name(std::move(value));                                                    \
        } catch (json::exception & e) {                                                            \
            external_error(std::string("Invalid parameter in " SETTINGS_JSON "!\n") + e.what());   \
        } catch (const char* e) {                                                                  \
            external_error(std::string("Invalid parameter in " SETTINGS_JSON "!\n") + e);          \
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
        external_error(SETTINGS_JSON " is corrupt! Please fix this or delete the file!");
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
    s->keys1.one_frame_brake = EolSettings->one_frame_brake_key_player_a();
    s->keys2.one_frame_brake = EolSettings->one_frame_brake_key_player_b();

    s->key_escape_alias = EolSettings->escape_alias_key();
    s->key_high_quality = EolSettings->high_quality_key();
    s->key_default_ground_sky = EolSettings->default_ground_sky_key();
    s->key_zoom_out = EolSettings->zoom_out_key();
    s->key_zoom_in = EolSettings->zoom_in_key();
    s->key_screenshot_level = EolSettings->screenshot_level_key();

    s->key_replay_fast_2x = EolSettings->replay_fast_2x_key();
    s->key_replay_fast_4x = EolSettings->replay_fast_4x_key();
    s->key_replay_fast_8x = EolSettings->replay_fast_8x_key();
    s->key_replay_slow_2x = EolSettings->replay_slow_2x_key();
    s->key_replay_slow_4x = EolSettings->replay_slow_4x_key();
    s->key_replay_pause = EolSettings->replay_pause_key();
    s->key_replay_rewind = EolSettings->replay_rewind_key();

    s->key_show_others = EolSettings->show_others_key();
    s->key_spy_next_kuski = EolSettings->spy_next_kuski_key();
    s->key_spy_prev_kuski = EolSettings->spy_prev_kuski_key();
    s->key_battle_queue = EolSettings->battle_queue_key();
    s->key_download_battle_level = EolSettings->download_battle_level_key();
    s->key_download_level = EolSettings->download_level_key();
    s->key_players_online = EolSettings->players_online_key();
    s->key_battle_results = EolSettings->battle_results_key();
    s->key_finished_times = EolSettings->finished_times_key();
    s->key_cycle_finished_times_filter = EolSettings->cycle_finished_times_filter_key();
    s->key_clear_finished_times = EolSettings->clear_finished_times_key();
    s->key_chat = EolSettings->chat_key();
    s->key_show_chat = EolSettings->show_chat_key();
    s->key_team_chat = EolSettings->team_chat_key();
    s->key_pm_next_kuski = EolSettings->pm_next_kuski_key();
    s->key_pm_prev_kuski = EolSettings->pm_prev_kuski_key();
    s->key_pm_clear_kuski = EolSettings->pm_clear_kuski_key();
    s->key_battle_status = EolSettings->battle_status_key();
    s->key_battle_leader = EolSettings->battle_leader_key();
    s->key_speedometer = EolSettings->speedometer_key();
    s->key_reconnect = EolSettings->reconnect_key();
    s->key_disconnect = EolSettings->disconnect_key();
    s->key_toggle_one_wheel_status = EolSettings->toggle_one_wheel_status_key();
    s->key_toggle_last_apple_time = EolSettings->toggle_last_apple_time_key();
}

void eol_settings::sync_controls_from_state(state* s) {
    if (!s) {
        return;
    }

    EolSettings->persist_alovolt_key_player_a(s->keys1.alovolt);
    EolSettings->persist_alovolt_key_player_b(s->keys2.alovolt);
    EolSettings->persist_brake_alias_key_player_a(s->keys1.brake_alias);
    EolSettings->persist_brake_alias_key_player_b(s->keys2.brake_alias);
    EolSettings->persist_one_frame_brake_key_player_a(s->keys1.one_frame_brake);
    EolSettings->persist_one_frame_brake_key_player_b(s->keys2.one_frame_brake);

    EolSettings->persist_escape_alias_key(s->key_escape_alias);
    EolSettings->persist_high_quality_key(s->key_high_quality);
    EolSettings->persist_default_ground_sky_key(s->key_default_ground_sky);
    EolSettings->persist_zoom_out_key(s->key_zoom_out);
    EolSettings->persist_zoom_in_key(s->key_zoom_in);
    EolSettings->persist_screenshot_level_key(s->key_screenshot_level);

    EolSettings->persist_replay_fast_2x_key(s->key_replay_fast_2x);
    EolSettings->persist_replay_fast_4x_key(s->key_replay_fast_4x);
    EolSettings->persist_replay_fast_8x_key(s->key_replay_fast_8x);
    EolSettings->persist_replay_slow_2x_key(s->key_replay_slow_2x);
    EolSettings->persist_replay_slow_4x_key(s->key_replay_slow_4x);
    EolSettings->persist_replay_pause_key(s->key_replay_pause);
    EolSettings->persist_replay_rewind_key(s->key_replay_rewind);

    EolSettings->persist_show_others_key(s->key_show_others);
    EolSettings->persist_spy_next_kuski_key(s->key_spy_next_kuski);
    EolSettings->persist_spy_prev_kuski_key(s->key_spy_prev_kuski);
    EolSettings->persist_battle_queue_key(s->key_battle_queue);
    EolSettings->persist_download_battle_level_key(s->key_download_battle_level);
    EolSettings->persist_download_level_key(s->key_download_level);
    EolSettings->persist_players_online_key(s->key_players_online);
    EolSettings->persist_battle_results_key(s->key_battle_results);
    EolSettings->persist_finished_times_key(s->key_finished_times);
    EolSettings->persist_cycle_finished_times_filter_key(s->key_cycle_finished_times_filter);
    EolSettings->persist_clear_finished_times_key(s->key_clear_finished_times);
    EolSettings->persist_chat_key(s->key_chat);
    EolSettings->persist_show_chat_key(s->key_show_chat);
    EolSettings->persist_team_chat_key(s->key_team_chat);
    EolSettings->persist_pm_next_kuski_key(s->key_pm_next_kuski);
    EolSettings->persist_pm_prev_kuski_key(s->key_pm_prev_kuski);
    EolSettings->persist_pm_clear_kuski_key(s->key_pm_clear_kuski);
    EolSettings->persist_battle_status_key(s->key_battle_status);
    EolSettings->persist_battle_leader_key(s->key_battle_leader);
    EolSettings->persist_speedometer_key(s->key_speedometer);
    EolSettings->persist_reconnect_key(s->key_reconnect);
    EolSettings->persist_disconnect_key(s->key_disconnect);
    EolSettings->persist_toggle_one_wheel_status_key(s->key_toggle_one_wheel_status);
    EolSettings->persist_toggle_last_apple_time_key(s->key_toggle_last_apple_time);
}
