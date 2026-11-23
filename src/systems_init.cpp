#include "systems_init.h"
#include "api/api.h"
#include "editor/canvas.h"
#include "eol/eol.h"
#include "eol/settings.h"
#include "pic/surface.h"
#include "platform/implementation.h"
#include "util/util.h"
#include <filesystem>

namespace systems_init {

void init_pre_intro() {
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

    platform_init();
    editor_canvas_update_resolution();

    EolClient = new eol();
    EolClient->connect();

    eol_api::init();
}

} // namespace systems_init
