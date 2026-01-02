#include "ALL.H"
#include <string>
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
        render_error(text1, text2, text3, text4);
        while (true) {
            int c = mk_getextchar();
            if (c == MK_ESC || c == MK_ENTER) {
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
