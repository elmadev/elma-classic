#include "api/api.h"
#include "editor/canvas.h"
#include "eol/eol.h"
#include "eol/settings.h"
#include "menu/intro.h"
#include "pic/surface.h"
#include "platform/implementation.h"
#include "util/util.h"

int main() {
    util::random::seed();

    std::filesystem::create_directory("lev");
    std::filesystem::create_directory("rec");

    EolSettings = new eol_settings();
    eol_settings::read_settings();
    if (const char* overrides = std::getenv("EOL_SETTINGS_OVERRIDES")) {
        eol_settings::read_overrides(overrides);
    }

    SCREEN_WIDTH = EolSettings->screen_width();
    SCREEN_HEIGHT = EolSettings->screen_height();
    editor_canvas_update_resolution();

    platform_init();

    EolClient = new eol();
    EolClient->connect();

    eol_api::init();

    menu_intro();
}
