#ifndef EOL_SETTINGS
#define EOL_SETTINGS

struct eol_settings {
    eol_settings();

    bool pictures_in_background;
    bool center_camera;
};

extern eol_settings* EolSettings;

#endif
