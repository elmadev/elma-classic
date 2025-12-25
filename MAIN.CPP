#include "ALL.H"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <timeapi.h>
#endif

static double Startido = 0;

// 182*sec-et adja vissza idot tortresszel egyutt!
// d_stoppe.cpp-ben vannak megvalositva:
double mv_stopperido(void) { return timeGetTime() * 0.182 - Startido; }

void mv_startstopper(void) { Startido = timeGetTime() * 0.182; }

void sdl_init();
int main() {
    sdl_init();

    teljes();
}

void mv_exit() { exit(0); }

int s_random(int maximum) { return rand() % maximum; }

bool Margrafikus = false;
static int Hibaban = 0;
static char Tmphibatext[500];

static void directhiba(const char* text1, const char* text2, const char* text3, const char* text4) {
    FILE* h;
    if (Hibaban) {
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
        if (Hibaban) {
            fprintf(h, "Two errors while processing!\n");
        }

        fclose(h);
    }

    if (Hibaban) {
        return;
    }
    Hibaban = 1;

    if (Margrafikus) {
        if (text4) {
            if (strlen(text3) + strlen(text4) < 490) {
                strcpy(Tmphibatext, text3);
                strcat(Tmphibatext, " ");
                strcat(Tmphibatext, text4);

                text3 = Tmphibatext;
            }
        }
        render_error(text1, text2, text3);
        while (1) {
            int c = mk_getextchar();
            if (c == MK_ESC || c == MK_ENTER) {
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
    mv_exit();
}

void hiba(const char* text1, const char* text2, const char* text3) {
    if (access("message.inf", 0) == 0) {
        directhiba("<<message.inf accessed>>", text1, text2, text3);
    }
    directhiba("Sorry, internal error.", NULL, NULL, NULL);
}

void uzenet(const char* text1, const char* text2, const char* text3) {
    // about azert kell, hogy kezdo memchecknel ne agazzon el:
    if (strstr(text1, "memory")) {
        // Ha memory szerepel text1-ben:
        if (access("message.inf", 0) == 0) { // Most mem-et is csak inf-re adja:
            directhiba("message.inf accessed\n", text1, text2, text3);
        }
        directhiba("Sorry, out of memory!", NULL, NULL, NULL);
    }
    directhiba(text1, text2, text3, NULL);
}
