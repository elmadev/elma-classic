#include "editor_canvas.h"
#include "eol/eol.h"
#include "eol_settings.h"
#include "M_PIC.H"
#include "main.h"
#include "menu/intro.h"
#include "menu/pic.h"
#include "platform/implementation.h"
#include "util/util.h"
#include <cstdio>
#include <cstdlib>
#include <directinput/scancodes.h>
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

    EolSettings = new eol_settings();
    eol_settings::read_settings();

    SCREEN_WIDTH = EolSettings->screen_width();
    SCREEN_HEIGHT = EolSettings->screen_height();
    editor_canvas_update_resolution();

    platform_init();

    EolClient = new eol();
    EolClient->connect();

    menu_intro();
}

void quit() { exit(0); }

bool ErrorGraphicsLoaded = false;

[[noreturn]] static void handle_error(const std::string& prefix, const std::string& message) {
    static bool InError = false;
    static FILE* ErrorHandle;
    if (!InError) {
        ErrorHandle = fopen("error.txt", "w");
    }
    if (ErrorHandle) {
        if (InError) {
            fprintf(ErrorHandle, "\nTwo errors while processing!\n");
        }
        fprintf(ErrorHandle, "%s\n%s\n", prefix.c_str(), message.c_str());
    }

    if (InError) {
        if (ErrorHandle) {
            fclose(ErrorHandle);
        }
        message_box("A fatal error occurred. Details written to error.txt.");
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

    fclose(ErrorHandle);
    quit();
}

void internal_error(const std::string& message) { handle_error("Sorry, internal error.", message); }

void external_error(const std::string& message) {
    if (message.find("memory") != std::string::npos) {
        handle_error("Sorry, out of memory!", message);
    } else {
        handle_error("External error encountered:", message);
    }
}
