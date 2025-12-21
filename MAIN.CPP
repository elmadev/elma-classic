#include "ALL.H"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <timeapi.h>
#endif

// szamitogep
// monitor

int Sajatugras = 0; // (forditva mint regen) Ha igaz, akkor jo ugras (right-clockwise)
// Ha nem igaz, csak memoriaba ir es nem flip-pel:

static void ugraseldontes(void) {
    if (access("fordit.inf", 0) == 0) {
        Sajatugras = 0;
    } else {
        Sajatugras = 1;
    }
}

// magassagokeloallitasa-bol kimasolva!!!!!:
/*static void elemszam( char* text, szakaszok* pszakok ) {
    pszakok->felsorolasresetszak();
    szakasz* psz = pszakok->getnextszak();
    if( !psz )
        hiba( "Nincs egyetlen szakasz sem definialva!" );
    double minx = psz->vv.r.x;
    double maxx = psz->vv.r.x;
    double miny = psz->vv.r.y;
    double maxy = psz->vv.r.y;
    while( psz ) {
        if( psz->vv.r.x < minx )
            minx = psz->vv.r.x;
        if( psz->vv.r.x > maxx )
            maxx = psz->vv.r.x;
        if( psz->vv.r.y < miny )
            miny = psz->vv.r.y;
        if( psz->vv.r.y > maxy )
            maxy = psz->vv.r.y;

        if( psz->vv.r.x+psz->vv.v.x < minx )
            minx = psz->vv.r.x+psz->vv.v.x;
        if( psz->vv.r.x+psz->vv.v.x > maxx )
            maxx = psz->vv.r.x+psz->vv.v.x;
        if( psz->vv.r.y+psz->vv.v.y < miny )
            miny = psz->vv.r.y+psz->vv.v.y;
        if( psz->vv.r.y+psz->vv.v.y > maxy )
            maxy = psz->vv.r.y+psz->vv.v.y;

        psz = pszakok->getnextszak();
    }

    vekt2 origo = vekt2( minx-400/Aranyx, miny-300/Aranyy );
    // Egy kis veletlenites!:
    origo.x += 0.0000145839857463;
    origo.y += 0.0000295874632574;

    double xsize = (maxx+400/Aranyx)-origo.x;
    double ysize = (maxy+300/Aranyy)-origo.y;

    // Lefoglalja magassagokat:
    int xszazalek = 100.0*(xsize*Aranyx)/Magmaxelemszam;
    int yszazalek = 100.0*(ysize*Aranyy)/Magmaxelemszam;
    sprintf( text, "Kihasznaltsag szazalekban: x = %d, y = %d", xszazalek,
                                                                yszazalek );
} */

/*
q7/8
    232, 473
o0/1
    336, 343
n8/9
    241, 245, 246, 580, 611, 982
o8/9
    376, 379, 634
*/

static double Startido = 0;

// 182*sec-et adja vissza idot tortresszel egyutt!
// d_stoppe.cpp-ben vannak megvalositva:
double mv_stopperido(void) { return timeGetTime() * 0.182 - Startido; }

void mv_startstopper(void) { Startido = timeGetTime() * 0.182; }

int sdl_init(void);
int main() {
    sdl_init();

    initqopen();

    void mk_init(void);
    mk_init();

    init_menu_pictures();
    mv_main();
}

void mv_main(void) {
    // new wav( "utodes.wav", 0.25 );

    // initqopen();

    if (access("nemmem.inf", 0) != 0) {
        // Ha nem talalja nemmem.inf-et:
        // mem Totall Free 11600K-nal meg eppen elmegy, 101 mar uzenetet ir:
        int memkell = 100;
        char* tomb[110];
        for (int i = 0; i < memkell; i++) {
            {
                tomb[i] = new char[100000];
            }
            if (!tomb[i]) {
                char tmp[200];
                sprintf(tmp, "You need about %d00K more free memory!\n", memkell - i);
                uzenet(tmp);
            }
        }
        for (int i = 0; i < memkell; i++) {
            delete tomb[i];
        }
    }

    // init_physics_data(); teljesben van mar ujabban

    ugraseldontes();

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
    if (strstr(text1, "memory") && !strstr(text1, "about")) {
        // Ha memory szerepel text1-ben:
        if (access("message.inf", 0) == 0) { // Most mem-et is csak inf-re adja:
            directhiba("message.inf accessed\n", text1, text2, text3);
        }
        directhiba("Sorry, out of memory!", NULL, NULL, NULL);
    }
    directhiba(text1, text2, text3, NULL);
}
