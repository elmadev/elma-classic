#ifndef EOL_SETTINGS
#define EOL_SETTINGS

struct eol_settings {
    eol_settings();

    bool pictures_in_background;
    bool center_camera;
    bool center_map;
};

extern eol_settings* EolSettings;

#endif
