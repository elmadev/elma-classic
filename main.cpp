#include "ALL.H"
#include "abc8.h"
#include "menu_pic.h"
#include "platform_utils.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <timeapi.h>
#endif

static double StopwatchStartTime = 0;

double stopwatch() { return timeGetTime() * 0.182 - StopwatchStartTime; }

void stopwatch_reset() { StopwatchStartTime = timeGetTime() * 0.182; }

int main() {
    platform_init();

    teljes();
}

void quit() { exit(0); }

int random_range(int maximum) { return rand() % maximum; }

bool ErrorGraphicsLoaded = false;
static bool InError = false;
static char ErrorTextTmp[500];

static void handle_error(const char* text1, const char* text2, const char* text3,
                         const char* text4) {
    ErrorOnMissingCodepoint = false;
    FILE* h;
    if (InError) {
        h = fopen("error_b.txt", "wt");
    } else {
        h = fopen("error.txt", "wt");
    }
    if (h) {
        fprintf(h, "%s\n", text1);
        if (text2) {
            fprintf(h, "%s\n", text2);
        }
        if (text3) {
            fprintf(h, "%s\n", text3);
        }
        if (text4) {
            fprintf(h, "%s\n", text4);
        }
        if (InError) {
            fprintf(h, "Two errors while processing!\n");
        }

        fclose(h);
    }

    if (InError) {
        return;
    }
    InError = true;

    if (ErrorGraphicsLoaded) {
        if (text4) {
            if (strlen(text3) + strlen(text4) < 490) {
                strcpy(ErrorTextTmp, text3);
                strcat(ErrorTextTmp, " ");
                strcat(ErrorTextTmp, text4);

                text3 = ErrorTextTmp;
            }
        }
        render_error(text1, text2, text3);
        while (true) {
            int c = get_keypress();
            if (c == KEY_ESC || c == KEY_ENTER) {
                break;
            }
        }
    } else {
        char text[200];
        strcpy(text, text1);
        if (text2) {
            strcat(text, "\n");
            strcat(text, text2);
        }
        if (text3) {
            strcat(text, "\n");
            strcat(text, text3);
        }
        if (text4) {
            strcat(text, "\n");
            strcat(text, text4);
        }
        message_box(text);
    }
    quit();
}

void internal_error(const char* text1, const char* text2, const char* text3) {
    if (access("message.inf", 0) == 0) {
        handle_error("<<message.inf accessed>>", text1, text2, text3);
    }
    handle_error("Sorry, internal error.", nullptr, nullptr, nullptr);
}

void external_error(const char* text1, const char* text2, const char* text3) {
    if (strstr(text1, "memory")) {
        if (access("message.inf", 0) == 0) {
            handle_error("message.inf accessed\n", text1, text2, text3);
        }
        handle_error("Sorry, out of memory!", nullptr, nullptr, nullptr);
    }
    handle_error(text1, text2, text3, nullptr);
}
