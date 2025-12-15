#ifndef EOL_SETTINGS
#define EOL_SETTINGS

enum class MapAlignment { None, Left, Middle, Right };
enum class RendererType { Software, OpenGL };

constexpr double MIN_ZOOM = 0.25;
constexpr double MAX_ZOOM = 3.00;

struct eol_settings {
    eol_settings();
    static void read_settings();
    static void write_settings();

    int screen_width;
    int screen_height;
    bool pictures_in_background;
    bool center_camera;
    bool center_map;
    MapAlignment map_alignment;
    RendererType renderer;

    double zoom() const { return zoom_; }
    void set_zoom(double z);

    bool zoom_textures() const { return zoom_textures_; }
    void set_zoom_textures(bool zoom_textures);

  private:
    double zoom_;
    bool zoom_textures_;
};

extern eol_settings* EolSettings;

#endif
