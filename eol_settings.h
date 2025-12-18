#ifndef EOL_SETTINGS
#define EOL_SETTINGS

enum class MapAlignment : unsigned char { None = 0, Left = 1, Middle = 2, Right = 3 };

struct eol_settings {
    eol_settings();

    bool pictures_in_background;
    bool center_camera;
    bool center_map;
    MapAlignment map_alignment;
};

extern eol_settings* EolSettings;

#endif
