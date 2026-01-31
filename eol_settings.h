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

class eol_settings {
  public:
    static void read_settings();
    static void write_settings();
    static void sync_controls_to_state(state* s);
    static void sync_controls_from_state(state* s);

    int screen_width() const { return screen_width_; }
    void set_screen_width(int w);

    int screen_height() const { return screen_height_; }
    void set_screen_height(int h);

    bool pictures_in_background() const { return pictures_in_background_; }
    void set_pictures_in_background(bool b);

    bool center_camera() const { return center_camera_; }
    void set_center_camera(bool b);

    bool center_map() const { return center_map_; }
    void set_center_map(bool b);

    MapAlignment map_alignment() const { return map_alignment_; }
    void set_map_alignment(MapAlignment m);

    RendererType renderer() const { return renderer_; }
    void set_renderer(RendererType r);

    double zoom() const { return zoom_; }
    void set_zoom(double z);

    bool zoom_textures() const { return zoom_textures_; }
    void set_zoom_textures(bool zoom_textures);

    double turn_time() const { return turn_time_; }
    void set_turn_time(double t);

    bool lctrl_search() const { return lctrl_search_; }
    void set_lctrl_search(bool lctrl_search);

    int alovolt_key_player_a() const { return alovolt_key_player_a_; }
    void set_alovolt_key_player_a(int key);

    int alovolt_key_player_b() const { return alovolt_key_player_b_; }
    void set_alovolt_key_player_b(int key);

    int brake_alias_key_player_a() const { return brake_alias_key_player_a_; }
    void set_brake_alias_key_player_a(int key);

    int brake_alias_key_player_b() const { return brake_alias_key_player_b_; }
    void set_brake_alias_key_player_b(int key);

    int escape_alias_key() const { return escape_alias_key_; }
    void set_escape_alias_key(int key);

  private:
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
};

extern eol_settings* EolSettings;

#endif
