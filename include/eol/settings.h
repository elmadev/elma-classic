#ifndef EOL_SETTINGS
#define EOL_SETTINGS

#include "eol_table.h"
#include "platform/scancode.h"
#include <string>

class state;

enum class MapAlignment { None, Left, Middle, Right };
enum class RendererType { Software, OpenGL };
enum class FullscreenMode { Windowed, Fullscreen, FullscreenDesktop };
enum class ChatVisibility { Shown, PublicHidden, Hidden };

template <typename T> struct Default {
    T value;
    T persisted;
    T def;

    constexpr Default(const T& default_value)
        : value(default_value),
          persisted(default_value),
          def(default_value) {}

    operator T() const;
    Default& operator=(T v);
    void reset_to_persisted();
    void reset_to_default();

  private:
    friend class eol_settings;
    void mark_persisted();
};

template <typename T> struct Clamp {
    T value;
    T min;
    T persisted;
    T def;
    T max;

    constexpr Clamp(T min_val, T def_val, T max_val)
        : value(def_val),
          min(min_val),
          persisted(def_val),
          def(def_val),
          max(max_val) {}

    operator T() const;
    Clamp& operator=(T v);
    void reset_to_persisted();
    void reset_to_default();

  private:
    friend class eol_settings;
    void mark_persisted();
};

#define SETTING_VALUE_TYPE(name) decltype(eol_settings::name##_.value)

// Declares/defines getter/setter/default for a field of `eol_settings`.
// For a field:
//   int foo;
// Expands into:
//   int foo() const { return foo_; }
//   void set_foo(int);
//   void persist_foo(int);
//   int foo_persisted() const { return foo_.persisted; }
//   int foo_default() const { return foo_.def; }
#define DECLARE_SETTING_COMMON(name)                                                               \
    SETTING_VALUE_TYPE(name) name() const { return name##_; }                                      \
    void persist_##name(SETTING_VALUE_TYPE(name));                                                 \
    SETTING_VALUE_TYPE(name) name##_persisted() const { return name##_.persisted; }                \
    SETTING_VALUE_TYPE(name) name##_default() const { return name##_.def; }

#define DECLARE_SETTING(name)                                                                      \
    DECLARE_SETTING_COMMON(name)                                                                   \
    void set_##name(SETTING_VALUE_TYPE(name) v) { name##_ = std::move(v); }
#define DECLARE_SETTING_CUSTOM(name)                                                               \
    DECLARE_SETTING_COMMON(name)                                                                   \
    void set_##name(SETTING_VALUE_TYPE(name));

#ifndef DEBUG
constexpr int DEFAULT_TCP_PORT = 4460;
constexpr int DEFAULT_UDP_PORT = 4461;
#else
constexpr int DEFAULT_TCP_PORT = 4470;
constexpr int DEFAULT_UDP_PORT = 4471;
#endif

class eol_settings {
    Clamp<int> screen_width_{640, 800, 10000};
    Clamp<int> screen_height_{480, 600, 10000};

    Default<combo_scancode> high_quality_key_{combo_scancode{DIK_LCONTROL, DIK_H}};
    Default<bool> pictures_in_background_{false};
    Default<bool> default_ground_{false};
    Default<bool> default_sky_{false};
    Default<combo_scancode> default_ground_sky_key_{combo_scancode{DIK_LCONTROL, DIK_G}};
    Default<combo_scancode> zoom_out_key_{combo_scancode{DIK_NONE, DIK_SUBTRACT}};
    Default<combo_scancode> zoom_in_key_{combo_scancode{DIK_NONE, DIK_ADD}};
    Default<combo_scancode> screenshot_level_key_{combo_scancode{DIK_LCONTROL, DIK_S}};

    Default<bool> center_camera_{false};
    Default<bool> center_map_{false};
    Default<MapAlignment> map_alignment_{MapAlignment::None};
    Default<RendererType> renderer_{RendererType::Software};
    Default<FullscreenMode> fullscreen_{FullscreenMode::Windowed};
    Clamp<double> zoom_{0.25, 1.0, 10.0};
    Clamp<double> minimap_zoom_{0.25, 1.0, 10.0};
    Default<bool> zoom_textures_{true};
    Default<bool> zoom_grass_{true};
    Clamp<double> turn_time_{0.0, 0.35, 0.35};
    Default<bool> lctrl_search_{false};

    Default<DikScancode> alovolt_key_player_a_{DIK_NONE};
    Default<DikScancode> alovolt_key_player_b_{DIK_NONE};
    Default<DikScancode> brake_alias_key_player_a_{DIK_NONE};
    Default<DikScancode> brake_alias_key_player_b_{DIK_NONE};
    Default<DikScancode> one_frame_brake_key_player_a_{DIK_NONE};
    Default<DikScancode> one_frame_brake_key_player_b_{DIK_NONE};
    Default<DikScancode> escape_alias_key_{DIK_NONE};

    Default<DikScancode> replay_fast_2x_key_{DIK_UP};
    Default<DikScancode> replay_fast_4x_key_{DIK_RIGHT};
    Default<DikScancode> replay_fast_8x_key_{DIK_PRIOR};
    Default<DikScancode> replay_slow_2x_key_{DIK_DOWN};
    Default<DikScancode> replay_slow_4x_key_{DIK_NEXT};
    Default<DikScancode> replay_pause_key_{DIK_SPACE};
    Default<DikScancode> replay_rewind_key_{DIK_LEFT};

    Default<combo_scancode> show_others_key_{combo_scancode{DIK_NONE, DIK_F1}};
    Default<combo_scancode> spy_next_kuski_key_{combo_scancode{DIK_NONE, DIK_F2}};
    Default<combo_scancode> spy_prev_kuski_key_{combo_scancode{DIK_LSHIFT, DIK_F2}};
    Default<combo_scancode> battle_queue_key_{combo_scancode{DIK_NONE, DIK_F3}};
    Default<combo_scancode> download_battle_level_key_{combo_scancode{DIK_NONE, DIK_F4}};
    Default<combo_scancode> download_level_key_{combo_scancode{DIK_LCONTROL, DIK_F4}};
    Default<combo_scancode> players_online_key_{combo_scancode{DIK_NONE, DIK_F5}};
    Default<combo_scancode> battle_results_key_{combo_scancode{DIK_NONE, DIK_F6}};
    Default<combo_scancode> finished_times_key_{combo_scancode{DIK_NONE, DIK_F7}};
    Default<combo_scancode> cycle_finished_times_filter_key_{combo_scancode{DIK_LSHIFT, DIK_F7}};
    Default<combo_scancode> clear_finished_times_key_{combo_scancode{DIK_LCONTROL, DIK_F7}};
    Default<combo_scancode> chat_key_{combo_scancode{DIK_NONE, DIK_F9}};
    Default<combo_scancode> show_chat_key_{combo_scancode{DIK_LSHIFT, DIK_F9}};
    Default<combo_scancode> team_chat_key_{combo_scancode{DIK_LCONTROL, DIK_F9}};
    Default<combo_scancode> pm_next_kuski_key_{combo_scancode{DIK_NONE, DIK_F2}};
    Default<combo_scancode> pm_prev_kuski_key_{combo_scancode{DIK_LSHIFT, DIK_F2}};
    Default<combo_scancode> pm_clear_kuski_key_{combo_scancode{DIK_LCONTROL, DIK_F2}};
    Default<combo_scancode> battle_status_key_{combo_scancode{DIK_NONE, DIK_F10}};
    Default<combo_scancode> battle_leader_key_{combo_scancode{DIK_LSHIFT, DIK_F10}};
    Default<combo_scancode> speedometer_key_{combo_scancode{DIK_LCONTROL, DIK_F10}};
    Default<combo_scancode> reconnect_key_{combo_scancode{DIK_NONE, DIK_F12}};
    Default<combo_scancode> disconnect_key_{combo_scancode{DIK_LSHIFT, DIK_F12}};
    Default<combo_scancode> toggle_one_wheel_status_key_{combo_scancode{DIK_LCONTROL, DIK_F11}};
    Default<combo_scancode> toggle_last_apple_time_key_{combo_scancode{DIK_LCONTROL, DIK_F8}};

    Default<std::string> default_lgr_name_{"default"};
    Default<bool> fancyboost_{true};

    Default<bool> show_last_apple_time_{true};
    Default<bool> show_gravity_arrows_{true};

    Default<bool> show_fps_{false};
    Default<bool> show_ups_{false};
    Default<bool> fps_limit_enabled_{false};
    Clamp<int> fps_limit_{30, 100, 1000};
    Clamp<int> recording_fps_{30, 30, 120};

    Default<bool> show_demo_menu_{true};
    Default<bool> show_help_menu_{true};
    Default<bool> show_about_menu_{true};
    Default<bool> show_best_times_menu_{true};
    Default<bool> skip_intro_{false};
    Default<bool> still_objects_{false};
    Default<bool> all_internals_accessible_{false};
    Default<bool> show_total_time_{true};
    Clamp<int> minimap_width_{140, 140, 420};
    Clamp<int> minimap_height_{70, 70, 210};
    Clamp<int> minimap_opacity_{25, 100, 100};
    Clamp<int> chat_lines_{1, 10, 50};
    Default<bool> cripple_no_brake_{false};
    Default<bool> cripple_no_throttle_{false};
    Default<bool> cripple_always_throttle_{false};
    Default<bool> cripple_no_turn_{false};
    Default<bool> cripple_no_volt_{false};
    Default<bool> cripple_one_turn_{false};
    Default<bool> cripple_drunk_{false};
    Default<bool> cripple_one_wheel_{false};
    Default<bool> show_one_wheel_status_{false};
    Default<std::string> hostname_{"eol.elma.online"};
    Default<int> tcp_port_{DEFAULT_TCP_PORT};
    Default<int> udp_port_{DEFAULT_UDP_PORT};
    Default<std::string> nick_{""};
    Default<std::string> password_{""};
    Default<bool> play_offline_{false};
    Default<bool> tcp_only_{false};

    Default<bool> show_others_{true};
    Default<bool> show_battle_status_{true};
    Default<bool> show_battle_leader_{true};
    Default<bool> show_speedometer_{false};

    Default<eol_table::Align> table_alignment_{eol_table::Align::Center};
    Default<ChatVisibility> chat_visibility_{ChatVisibility::Shown};

  public:
    static void read_settings();
    static void write_settings();
    static void sync_controls_to_state(state* s);
    static void sync_controls_from_state(state* s);

    DECLARE_SETTING(screen_width);
    DECLARE_SETTING(screen_height);

    DECLARE_SETTING(high_quality_key);
    DECLARE_SETTING_CUSTOM(pictures_in_background);
    DECLARE_SETTING_CUSTOM(default_ground);
    DECLARE_SETTING_CUSTOM(default_sky);
    DECLARE_SETTING(default_ground_sky_key);
    DECLARE_SETTING(zoom_out_key);
    DECLARE_SETTING(zoom_in_key);
    DECLARE_SETTING(screenshot_level_key);

    DECLARE_SETTING(center_camera);
    DECLARE_SETTING(center_map);
    DECLARE_SETTING(map_alignment);
    DECLARE_SETTING_CUSTOM(renderer);
    DECLARE_SETTING_CUSTOM(fullscreen);
    DECLARE_SETTING_CUSTOM(zoom);
    DECLARE_SETTING_CUSTOM(minimap_zoom);
    DECLARE_SETTING_CUSTOM(zoom_textures);
    DECLARE_SETTING_CUSTOM(zoom_grass);
    DECLARE_SETTING(turn_time);
    DECLARE_SETTING(lctrl_search);

    DECLARE_SETTING(alovolt_key_player_a);
    DECLARE_SETTING(alovolt_key_player_b);
    DECLARE_SETTING(brake_alias_key_player_a);
    DECLARE_SETTING(brake_alias_key_player_b);
    DECLARE_SETTING(one_frame_brake_key_player_a);
    DECLARE_SETTING(one_frame_brake_key_player_b);
    DECLARE_SETTING(escape_alias_key);

    DECLARE_SETTING(replay_fast_2x_key);
    DECLARE_SETTING(replay_fast_4x_key);
    DECLARE_SETTING(replay_fast_8x_key);
    DECLARE_SETTING(replay_slow_2x_key);
    DECLARE_SETTING(replay_slow_4x_key);
    DECLARE_SETTING(replay_pause_key);
    DECLARE_SETTING(replay_rewind_key);

    DECLARE_SETTING(show_others_key);
    DECLARE_SETTING(spy_next_kuski_key);
    DECLARE_SETTING(spy_prev_kuski_key);
    DECLARE_SETTING(battle_queue_key);
    DECLARE_SETTING(download_battle_level_key);
    DECLARE_SETTING(download_level_key);
    DECLARE_SETTING(players_online_key);
    DECLARE_SETTING(battle_results_key);
    DECLARE_SETTING(finished_times_key);
    DECLARE_SETTING(cycle_finished_times_filter_key);
    DECLARE_SETTING(clear_finished_times_key);
    DECLARE_SETTING(chat_key);
    DECLARE_SETTING(show_chat_key);
    DECLARE_SETTING(team_chat_key);
    DECLARE_SETTING(pm_next_kuski_key);
    DECLARE_SETTING(pm_prev_kuski_key);
    DECLARE_SETTING(pm_clear_kuski_key);
    DECLARE_SETTING(battle_status_key);
    DECLARE_SETTING(battle_leader_key);
    DECLARE_SETTING(speedometer_key);
    DECLARE_SETTING(reconnect_key);
    DECLARE_SETTING(disconnect_key);
    DECLARE_SETTING(toggle_one_wheel_status_key);
    DECLARE_SETTING(toggle_last_apple_time_key);

    DECLARE_SETTING_CUSTOM(default_lgr_name);
    DECLARE_SETTING_CUSTOM(fancyboost);

    DECLARE_SETTING(show_last_apple_time);
    DECLARE_SETTING(show_gravity_arrows);

    DECLARE_SETTING(show_fps);
    DECLARE_SETTING(show_ups);
    DECLARE_SETTING(fps_limit_enabled);
    DECLARE_SETTING(fps_limit);
    DECLARE_SETTING(recording_fps);

    DECLARE_SETTING(show_demo_menu);
    DECLARE_SETTING(show_help_menu);
    DECLARE_SETTING(show_about_menu);
    DECLARE_SETTING(show_best_times_menu);
    DECLARE_SETTING(skip_intro);
    DECLARE_SETTING(still_objects);
    DECLARE_SETTING(all_internals_accessible);
    DECLARE_SETTING(show_total_time);
    DECLARE_SETTING(minimap_width);
    DECLARE_SETTING(minimap_height);
    DECLARE_SETTING(minimap_opacity);
    DECLARE_SETTING(chat_lines);
    DECLARE_SETTING(cripple_no_brake);
    DECLARE_SETTING_CUSTOM(cripple_no_throttle);
    DECLARE_SETTING_CUSTOM(cripple_always_throttle);
    DECLARE_SETTING_CUSTOM(cripple_no_turn);
    DECLARE_SETTING(cripple_no_volt);
    DECLARE_SETTING_CUSTOM(cripple_one_turn);
    DECLARE_SETTING(cripple_drunk);
    DECLARE_SETTING(cripple_one_wheel);
    DECLARE_SETTING(show_one_wheel_status);
    DECLARE_SETTING(hostname);
    DECLARE_SETTING(tcp_port);
    DECLARE_SETTING(udp_port);
    DECLARE_SETTING(nick);
    DECLARE_SETTING(password);
    DECLARE_SETTING(play_offline);
    DECLARE_SETTING(tcp_only);

    DECLARE_SETTING(show_others);
    DECLARE_SETTING(show_battle_status);
    DECLARE_SETTING(show_battle_leader);
    DECLARE_SETTING(show_speedometer);

    DECLARE_SETTING(table_alignment);
    DECLARE_SETTING(chat_visibility);
};

#undef SETTING_VALUE_TYPE
#undef DECLARE_SETTING
#undef DECLARE_SETTING_CUSTOM

extern eol_settings* EolSettings;

#endif
