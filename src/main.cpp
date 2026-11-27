#include "main.h"
#include "api/api.h"
#include "api/lgr_list.h"
#include "editor/canvas.h"
#include "eol/eol.h"
#include "eol/settings.h"
#include "log.h"
#include "menu/intro.h"
#include "menu/pic.h"
#include "pic/surface.h"
#include "platform/implementation.h"
#include "platform/scancode.h"
#include "util/util.h"
#include <cstdlib>
#include <string>

static double StopwatchStartTime = 0.0;

double stopwatch() { return get_milliseconds() * STOPWATCH_MULTIPLIER - StopwatchStartTime; }

void stopwatch_reset() { StopwatchStartTime = get_milliseconds() * STOPWATCH_MULTIPLIER; }

void delay(int milliseconds) {
    double current_time = stopwatch();
    while (stopwatch() / STOPWATCH_MULTIPLIER <
           current_time / STOPWATCH_MULTIPLIER + milliseconds) {
        handle_events();
    }
}

eol_settings* EolSettings = nullptr;
eol* EolClient = nullptr;

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

    platform_init();
    editor_canvas_update_resolution();

    EolClient = new eol();
    EolClient->connect();

    eol_api::init();

    // Start fetch in background
    LgrInfo.fetch();
    auto result = LgrInfo.get_data(); // result not yet available
    ELMA_ASSERT(result == nullptr);

    // Sync fetch (waits for previous fetch to finish)
    LgrInfo.await_fetch();
    auto info = LgrInfo.get_data(); // result is available
    for (const auto& entry : *info) {
        LOG_DEBUG("LGRName: {} (CRC: {})", entry.LGRName, entry.CRC);
    }

    // Reset data
    LgrInfo.clear_cache();
    auto result2 = LgrInfo.get_data(); // result no longer available
    ELMA_ASSERT(result2 == nullptr);

    // Sync fetch (starts a new fetch)
    LgrInfo.await_fetch();
    auto result3 = LgrInfo.get_data(); // result is available
    ELMA_ASSERT(result3 != nullptr);

    menu_intro();
}

void quit() {
    eol_api::cleanup();
    exit(0);
}

bool ErrorGraphicsLoaded = false;

void internal_error(const std::string& message, std::source_location loc) {
    static bool InError = false;
    logger::instance().write(LogLevel::Fatal, loc,
                             std::format("Sorry, internal error. {}", message));

    if (InError) {
        message_box("A fatal error occurred. Details written to eol.log.");
        exit(1);
    }
    InError = true;

    std::string text = "Sorry, internal error.\n" + message;

    bool rendered = false;
    if (ErrorGraphicsLoaded) {
        render_error(text);
        rendered = platform_render_error(BufferMain);
    }
    if (rendered) {
        while (true) {
            handle_events();
            if (was_key_just_pressed(DIK_ESCAPE) || was_key_just_pressed(DIK_RETURN) ||
                was_key_just_pressed(DIK_SPACE)) {
                break;
            }
        }
    } else {
        message_box(text.c_str());
    }

    quit();
}
