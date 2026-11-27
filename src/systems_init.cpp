#include "systems_init.h"
#include "api/api.h"
#include "editor/canvas.h"
#include "editor/editor.h"
#include "eol/eol.h"
#include "eol/settings.h"
#include "game/qopen.h"
#include "main.h"
#include "menu/pic.h"
#include "menu/rec_list.h"
#include "physics/init.h"
#include "pic/abc8.h"
#include "pic/surface.h"
#include "platform/implementation.h"
#include "renderer/render.h"
#include "util/stopwatch.h"
#include "util/util.h"
#include <filesystem>

namespace systems_init {

void init_pre_intro() {
    EolSettings = new eol_settings();
    eol_settings::read_settings();
    if (const char* overrides = std::getenv("EOL_SETTINGS_OVERRIDES")) {
        eol_settings::read_overrides(overrides);
    }

    SCREEN_WIDTH = EolSettings->screen_width();
    SCREEN_HEIGHT = EolSettings->screen_height();

    platform_init();

    init_qopen();
    init_menu_pictures();
}

void init_post_intro() {
    util::random::seed();

    std::filesystem::create_directory("lev");
    std::filesystem::create_directory("rec");

    EolClient = new eol();
    EolClient->connect();

    eol_api::init();
    State = new state;
    if (!State) {
        internal_error("memory");
    }

    merge_states();
    eol_settings::sync_controls_to_state(State);

    init_physics_data();

    rec_list::build_cache();

    // test_player();

    if (State->sound_on) {
        open_sound_device();
    }

    EditorWhiteFont = new abc8("kisbetu1.abc", 1, 19); // "small letter 1"
    EditorBlackFont = new abc8("kisbetu2.abc", 1, 19); // "small letter 2"

    init_renderer();

    Rec1 = new recorder;
    Rec2 = new recorder;

    editor_canvas_update_resolution();
    create_editor_palette();

    stopwatch_reset();
}

} // namespace systems_init
