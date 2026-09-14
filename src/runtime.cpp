#include "runtime.h"
#include "api/api.h"
#include "editor/editor.h"
#include "eol/eol.h"
#include "eol/settings.h"
#include "game/qopen.h"
#include "game/recorder.h"
#include "game/state.h"
#include "log.h"
#include "main.h"
#include "menu/pic.h"
#include "physics/init.h"
#include "pic/abc8.h"
#include "platform/implementation.h"
#include "platform/scancode.h"
#include "renderer/render.h"
#include <cstdlib>
#include <format>
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

void runtime::init_data() {
    EolClient = new eol();

    init_qopen();

    init_menu_pictures();

    State = new state;
    if (!State) {
        external_error("memory");
    }

    merge_states();
    eol_settings::sync_controls_to_state(State);

    init_physics_data();

    // Load globals
    EditorWhiteFont = new abc8("kisbetu1.abc", 1, 19); // "small letter 1"
    EditorBlackFont = new abc8("kisbetu2.abc", 1, 19); // "small letter 2"

    init_renderer();

    Rec1 = new recorder;
    Rec2 = new recorder;

    create_editor_palette();

    // Initialize stopwatch, just in case
    stopwatch_reset();
}

void quit() {
    eol_api::cleanup();
    exit(0);
}

bool ErrorGraphicsLoaded = false;

[[noreturn]] static void handle_error(const std::string& prefix, const std::string& message,
                                      std::source_location loc) {
    static bool InError = false;
    logger::instance().write(LogLevel::Fatal, loc, std::format("{} {}", prefix, message));

    if (InError) {
        message_box("A fatal error occurred. Details written to eol.log.");
        quit();
    }
    InError = true;

    std::string text = prefix + "\n" + message;

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

void internal_error(const std::string& message, std::source_location loc) {
    handle_error("Sorry, internal error.", message, loc);
}

void external_error(const std::string& message, std::source_location loc) {
    if (message.find("memory") != std::string::npos) {
        handle_error("Sorry, out of memory!", message, loc);
    }
    handle_error("External error encountered:", message, loc);
}
