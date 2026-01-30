#include "abc8.h"
#include "eol_settings.h"
#include "keys.h"
#include "M_PIC.H"
#include "main.h"
#include "menu_pic.h"
#include "platform_impl.h"
#include "TELJES.H"
#include <cstdio>
#include <cstdlib>
#include <cstring>
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

int main() {
    EolSettings = new eol_settings();
    eol_settings::read_settings();

    SCREEN_WIDTH = EolSettings->screen_width();
    SCREEN_HEIGHT = EolSettings->screen_height();

    platform_init();

    menu_intro();
}

void quit() { exit(0); }

int random_range(int maximum) { return rand() % maximum; }

bool ErrorGraphicsLoaded = false;

static void handle_error(const char* text1, const char* text2, const char* text3,
                         const char* text4) {
    static bool InError = false;
    static FILE* ErrorHandle;
    ErrorOnMissingCodepoint = false;
    if (!InError) {
        ErrorHandle = fopen("error.txt", "w");
    }
    if (ErrorHandle) {
        if (InError) {
            fprintf(ErrorHandle, "\nTwo errors while processing!\n");
        }
        fprintf(ErrorHandle, "%s\n", text1);
        if (text2) {
            fprintf(ErrorHandle, "%s\n", text2);
        }
        if (text3) {
            fprintf(ErrorHandle, "%s\n", text3);
        }
        if (text4) {
            fprintf(ErrorHandle, "%s\n", text4);
        }
    }

    if (InError) {
        return;
    }
    InError = true;

    if (ErrorGraphicsLoaded) {
        render_error(text1, text2, text3, text4);
        while (true) {
            Keycode c = get_keypress();
            if (c == KEY_ESC || c == KEY_ENTER) {
                break;
            }
        }
    } else {
        std::string text = text1;
        if (text2) {
            text = text + "\n" + text2;
        }
        if (text3) {
            text = text + "\n" + text3;
        }
        if (text4) {
            text = text + "\n" + text4;
        }
        message_box(text.c_str());
    }

    fclose(ErrorHandle);
    quit();
}

void internal_error(const char* text1, const char* text2, const char* text3) {
    handle_error("Sorry, internal error.", text1, text2, text3);
}

void external_error(const char* text1, const char* text2, const char* text3) {
    if (strstr(text1, "memory")) {
        handle_error("Sorry, out of memory!", text1, text2, text3);
    }
    handle_error("External error encountered:", text1, text2, text3);
}
