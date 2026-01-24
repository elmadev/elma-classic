#ifndef EOL_SETTINGS
#define EOL_SETTINGS

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

struct eol_settings {
    static void read_settings();
    static void write_settings();

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
};

extern eol_settings* EolSettings;

#endif
