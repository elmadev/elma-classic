#ifndef EOL_SETTINGS
#define EOL_SETTINGS

#include "platform_impl.h"
#include <directinput/scancodes.h>

class state;

enum class MapAlignment { None, Left, Middle, Right };
enum class RendererType { Software, OpenGL };

template <typename T> struct Default {
    T value;
    T def;

    constexpr Default(T default_value)
        : value(default_value),
          def(default_value) {}

    operator T() const;
    Default& operator=(T v);
    void reset();
};

template <typename T> struct Clamp {
    T value;
    T min;
    T def;
    T max;

    constexpr Clamp(T min_val, T def_val, T max_val)
        : value(def_val),
          min(min_val),
          def(def_val),
          max(max_val) {}

    operator T() const;
    Clamp& operator=(T v);
    void reset();
};

// Declares/defines getter/setter/default for a field of `eol_settings`.
// For a field:
//   int foo;
// Expands into:
//   int foo() const { return foo_; }
//   void set_foo(int);
//   int foo_default() const { return foo_.def; }
#define DECLARE_FIELD_FUNCS(name)                                                                  \
    decltype(eol_settings::name##_.value) name() const { return name##_; }                         \
    void set_##name(decltype(eol_settings::name##_.value));                                        \
    decltype(eol_settings::name##_.value) name##_default() const { return name##_.def; }

class eol_settings {
    Clamp<int> screen_width_{640, 640, 10000};
    Clamp<int> screen_height_{480, 480, 10000};
    Default<bool> pictures_in_background_{false};
    Default<bool> center_camera_{false};
    Default<bool> center_map_{false};
    Default<MapAlignment> map_alignment_{MapAlignment::None};
    Default<RendererType> renderer_{RendererType::Software};
    Clamp<double> zoom_{0.25, 1.0, 3.0};
    Default<bool> zoom_textures_{false};
    Clamp<double> turn_time_{0.0, 0.35, 0.35};
    Default<bool> lctrl_search_{false};
    Default<DikScancode> alovolt_key_player_a_{DIK_UNKNOWN};
    Default<DikScancode> alovolt_key_player_b_{DIK_UNKNOWN};
    Default<DikScancode> brake_alias_key_player_a_{DIK_UNKNOWN};
    Default<DikScancode> brake_alias_key_player_b_{DIK_UNKNOWN};
    Default<DikScancode> escape_alias_key_{DIK_UNKNOWN};
    Default<DikScancode> replay_fast_2x_key_{DIK_UP};
    Default<DikScancode> replay_fast_4x_key_{DIK_RIGHT};
    Default<DikScancode> replay_fast_8x_key_{DIK_PRIOR};
    Default<DikScancode> replay_slow_2x_key_{DIK_DOWN};
    Default<DikScancode> replay_slow_4x_key_{DIK_NEXT};
    Default<DikScancode> replay_pause_key_{DIK_SPACE};

  public:
    static void read_settings();
    static void write_settings();
    static void sync_controls_to_state(state* s);
    static void sync_controls_from_state(state* s);

    DECLARE_FIELD_FUNCS(screen_width);
    DECLARE_FIELD_FUNCS(screen_height);
    DECLARE_FIELD_FUNCS(pictures_in_background);
    DECLARE_FIELD_FUNCS(center_camera);
    DECLARE_FIELD_FUNCS(center_map);
    DECLARE_FIELD_FUNCS(map_alignment);
    DECLARE_FIELD_FUNCS(renderer);
    DECLARE_FIELD_FUNCS(zoom);
    DECLARE_FIELD_FUNCS(zoom_textures);
    DECLARE_FIELD_FUNCS(turn_time);
    DECLARE_FIELD_FUNCS(lctrl_search);
    DECLARE_FIELD_FUNCS(alovolt_key_player_a);
    DECLARE_FIELD_FUNCS(alovolt_key_player_b);
    DECLARE_FIELD_FUNCS(brake_alias_key_player_a);
    DECLARE_FIELD_FUNCS(brake_alias_key_player_b);
    DECLARE_FIELD_FUNCS(escape_alias_key);
    DECLARE_FIELD_FUNCS(replay_fast_2x_key);
    DECLARE_FIELD_FUNCS(replay_fast_4x_key);
    DECLARE_FIELD_FUNCS(replay_fast_8x_key);
    DECLARE_FIELD_FUNCS(replay_slow_2x_key);
    DECLARE_FIELD_FUNCS(replay_slow_4x_key);
    DECLARE_FIELD_FUNCS(replay_pause_key);
};

#undef DECLARE_FIELD_FUNCS

extern eol_settings* EolSettings;

#endif
