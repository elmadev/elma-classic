#ifndef EOL_SETTINGS
#define EOL_SETTINGS

enum class MapAlignment { None, Left, Middle, Right };

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
};

extern eol_settings* EolSettings;

#endif
