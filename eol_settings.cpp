#include "directinput_scancodes.h"
#include "eol_settings.h"

eol_settings::eol_settings() {
    pictures_in_background = false;
    center_camera = false;
    center_map = false;
    map_alignment = MapAlignment::None;
    zoom = 1.0;
    zoom_textures = false;
    alovolt_P1 = DIK_S;
}
