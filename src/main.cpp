#include "main.h"
#include "api/api.h"
#include "log.h"
#include "menu/intro.h"
#include "menu/pic.h"
#include "platform/implementation.h"
#include "platform/scancode.h"
#include "systems_init.h"
#include <cstdlib>
#include <string>

int main() {
    systems_init::init_pre_intro();
    menu_intro();
}

void quit() {
    eol_api::cleanup();
    exit(0);
}

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

    if (render_error(text) && platform_render_error(BufferMain)) {
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
