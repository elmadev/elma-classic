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

  private:
    int screen_width_;
    int screen_height_;
    bool pictures_in_background_;
    bool center_camera_;
    bool center_map_;
    MapAlignment map_alignment_;
    RendererType renderer_;
    double zoom_;
    bool zoom_textures_;
};

extern eol_settings* EolSettings;

#endif
