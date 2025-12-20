#include "ALL.H"

pic8* BufferMain = NULL;
pic8* BufferBall = NULL;

static pic8 *Kisnevsima = NULL, *Kisnevgolyo = NULL;
abc8* MenuFont = NULL;
anim* Psisak = NULL;

ddpal* MenuPal = NULL;
unsigned char MenuPalData[768];

void init_menu_pictures1(void) {
    if (BufferMain) {
        hiba("init_menu_pictures1-ben BufferMain!");
    }
    BufferMain = new pic8(SCREEN_WIDTH, SCREEN_HEIGHT);
    BufferBall = new pic8(SCREEN_WIDTH, SCREEN_HEIGHT);
    Kisnevsima = new pic8("szoveg1.pcx");
    Kisnevgolyo = new pic8("szoveg2.pcx");
    MenuFont = new abc8("menu.abc");
    MenuFont->set_spacing(2);

    pic8* sisaktmp = new pic8("sisak.pcx");
    forditkepet(sisaktmp);
    Psisak = new anim(sisaktmp, "sisak.pcx");
    delete sisaktmp;
    sisaktmp = NULL;
    Psisak->make_helmet_top();

    balls_init();
}

void init_menu_pictures2(void) {
    if (MenuPal) {
        hiba("init_menu_pictures2-ben MenuPal!");
    }
    get_pcx_pal("intro.pcx", &MenuPal); // O new-ik egy ddpal-t
    get_pcx_pal("intro.pcx", MenuPalData);
}

menu_pic::menu_pic(void) {
    sisakx = sisaky = -100;

    elemszam = 0;
    ervenyes = 0;
    sorok = new text_line[MAXSZOVEGSOR];
    if (!sorok) {
        hiba("Memoria!");
    }
}

menu_pic::~menu_pic(void) {
    if (!sorok) {
        hiba("menu_pic::~menu_pic !sorok!");
    }
    delete sorok;
    sorok = NULL;
}

void menu_pic::add_line(const char* text, int x, int y) {
    if (strlen(text) > 100) {
        hiba("89ygui");
    }
    if (elemszam >= MAXSZOVEGSOR - 1) {
        hiba("849yptgp8");
    }
    strcpy(sorok[elemszam].text, text);
    sorok[elemszam].x = x;
    sorok[elemszam].y = y;
    elemszam++;
    ervenyes = 0;
}

void menu_pic::add_line_centered(const char* text, int x, int y) {
    add_line(text, x - MenuFont->len(text) / 2, y);
}

void menu_pic::set_helmet(int x, int y) {
    sisakx = x;
    sisaky = y;
}

void menu_pic::clear(void) {
    elemszam = 0;
    ervenyes = 0;
}

static pic8* Tmpbuffer = NULL; // Ideiglenes

static int Scrolly1 = 0; // Ennel kiseb x-ek helyere sotet zoldet rak kirazgolyo
static unsigned char Introhatter = 254;
static unsigned char* Zoldsor = NULL;
static unsigned char* Hattersor = NULL;

static void kirajzgolyo(vekt2 g, double r, pic8* forras) {
    if (!Zoldsor) {
        Zoldsor = new unsigned char[SCREEN_WIDTH];
        Hattersor = new unsigned char[SCREEN_WIDTH];
        for (int i = 0; i < SCREEN_WIDTH; i++) {
            Zoldsor[i] = 248;
            Hattersor[i] = Introhatter;
        }
    }

    if (WallsDisabled) {
        if (g.x < -500.0 || g.x > 1500.0 || g.y < -500.0 || g.y > 1500.0) {
            return;
        }
    }

    double xo = g.x;
    double yo = g.y;
    int y = yo - r;
    while (y < yo + r) {
        if (y < 0 || y >= SCREEN_HEIGHT) {
            y++;
            continue;
        }

        double dx = r * r - (y - yo) * (y - yo);
        if (dx < 0.5) {
            y++;
            continue;
        }
        dx = sqrt(dx);
        int x1 = xo - dx;
        int x2 = xo + dx;
        if (x1 < 0) {
            x1 = 0;
        }
        if (x1 > 639) {
            x1 = 639;
        }
        if (x2 < 0) {
            x2 = 0;
        }
        if (x2 > 639) {
            x2 = 639;
        }
        if (x1 >= x2) {
            y++;
            continue;
        }
        unsigned char* dest = Tmpbuffer->get_row(y) + x1;
        unsigned char* source;
        if (y < Scrolly1) {
            if (forras == BufferBall) {
                source = Zoldsor;
            } else {
                source = Hattersor;
            }
        } else {
            source = forras->get_row(y) + x1;
        }
        memcpy(dest, source, x2 - x1 + 1);

        y++;
    }
}

static double Regiido = 0.0;
static double Elsostabilido = 0.0; // Scroll befejezesekor
static int Kezdoanimjon = 1;

void menu_pic::render(int skip_balls_helmet) {
    // Kirakjuk hatteret:
    if (!Tmpbuffer) { // Ideiglenes
        Tmpbuffer = new pic8(SCREEN_WIDTH, SCREEN_HEIGHT);
    }

    if (Kezdoanimjon && !Intro) {
        Kezdoanimjon = 0;
    }

    if (!ervenyes && !Kezdoanimjon) {
        // Ujrarajzoljuk hatterkepeket:
        ervenyes = 1;
        int y = -47;
        int kezdox = 0;
        while (y < SCREEN_HEIGHT) {
            int x = kezdox;
            kezdox += 110;
            while (x > 0) {
                x -= Kisnevsima->get_width();
            }
            while (x < SCREEN_WIDTH) {
                blit8(BufferMain, Kisnevsima, x, y);
                blit8(BufferBall, Kisnevgolyo, x, y);
                x += Kisnevsima->get_width();
            }
            y += Kisnevsima->get_height();
        }

        for (int i = 0; i < elemszam; i++) {
            MenuFont->write(BufferMain, sorok[i].x, sorok[i].y, sorok[i].text);
            MenuFont->write(BufferBall, sorok[i].x, sorok[i].y, sorok[i].text);
        }
    }

    double ujido = mv_stopperido();
    if (Kezdoanimjon) {
        MenuPal->set();
        while (1) {
            if (!render_intro_anim(ujido)) {
                Kezdoanimjon = 0;
                Regiido = ujido;
                return;
            }

            ujido = mv_stopperido();
            Elsostabilido = ujido;
            Kezdoanimjon = 0;
        }
    }

    double dt;
    if (ujido < Elsostabilido + 10.0) {
        dt = 0.0000000000001;
    } else {
        dt = ujido - Regiido;
        Elsostabilido = -100.0;
    }

    Regiido = ujido;
    if (dt < 0.0) {
        dt = 0.001;
    }

    dt *= 3.6;
    if (dt > 100.0) {
        dt = 100.0;
    }

    balls_simulate(dt); // 12.0 volt regebben

    blit8(Tmpbuffer, BufferMain);

    for (int i = 0; i < BallCount; i++) {
        if (skip_balls_helmet || !State->animated_menus) {
            break;
        }

        kirajzgolyo(Balls[i].current_r, Balls[i].radius, BufferBall);

        vekt2 diff(Balls[i].radius * 0.5 * cos(Balls[i].current_rotation),
                   Balls[i].radius * 0.5 * sin(Balls[i].current_rotation));
        kirajzgolyo(Balls[i].current_r + diff, Balls[i].radius * 0.25, BufferMain);
        kirajzgolyo(Balls[i].current_r - diff, Balls[i].radius * 0.25, BufferMain);
    }
    if (!skip_balls_helmet) {
        pic8* sisakframe = NULL;
        if (State->animated_menus) {
            sisakframe = Psisak->get_frame_by_time(ujido * 0.0024);
        } else {
            sisakframe = Psisak->get_frame_by_index(25);
        }

        blit8(Tmpbuffer, sisakframe, sisakx - 20, sisaky - 7);
    }

    bltfront(Tmpbuffer);
}

static int Elsokiranim = 1;
// Teljes olvassa be es elso kiirloading szabaditja fel:
pic8* Intro = NULL;

static double Animkezdete = 0.0;

// Hamis ha animnak mar vege:
int menu_pic::render_intro_anim(double time) {
    if (Elsokiranim) {
        balls_simulate(0.000000001);
        Elsokiranim = 0;
        Animkezdete = time;

        // Berajzoljuk hatterkepeket:
        int y = -47;
        int kezdox = 0;
        while (y < SCREEN_HEIGHT) {
            int x = kezdox;
            kezdox += 110;
            while (x > 0) {
                x -= Kisnevsima->get_width();
            }
            while (x < SCREEN_WIDTH) {
                blit8(BufferMain, Kisnevsima, x, y);
                blit8(BufferBall, Kisnevgolyo, x, y);
                x += Kisnevsima->get_width();
            }
            y += Kisnevsima->get_height();
        }
        // Most egyszer kirakunk minden elemet kepre hogy
        // ne akadjon meg elso lejatszas kozben:
        Tmpbuffer->fill_box(Introhatter);
        blit8(Tmpbuffer, BufferMain, 0, 0);

        Scrolly1 = 0;
        double yeltolas = 0.0;
        for (int i = 0; i < BallCount; i++) {
            if (!State->animated_menus) {
                break;
            }

            Balls[i].current_r.y += yeltolas;

            kirajzgolyo(Balls[i].current_r, Balls[i].radius, BufferBall);

            vekt2 diff(Balls[i].radius * 0.5 * cos(Balls[i].current_rotation),
                       Balls[i].radius * 0.5 * sin(Balls[i].current_rotation));
            kirajzgolyo(Balls[i].current_r + diff, Balls[i].radius * 0.25, BufferMain);
            kirajzgolyo(Balls[i].current_r - diff, Balls[i].radius * 0.25, BufferMain);

            Balls[i].current_r.y -= yeltolas;
        }

        // Rairjuk szovegeket:
        for (int i = 0; i < elemszam; i++) {
            MenuFont->write(Tmpbuffer, sorok[i].x, sorok[i].y, sorok[i].text);
        }

        pic8* sisakframe = NULL;
        sisakframe = Psisak->get_frame_by_index(25);
        blit8(Tmpbuffer, sisakframe, sisakx - 20, sisaky - 7);
        blit8(Tmpbuffer, Intro, 0, 0);
        // Idaig tartott minden elem kirakasa egyszer
    }

    time -= Animkezdete;

    int frame = time * 2.3;

    if (frame >= SCREEN_HEIGHT) {
        Scrolly1 = 0;
        return 0;
    }

    Tmpbuffer->fill_box(Introhatter);
    blit8(Tmpbuffer, BufferMain, 0, SCREEN_HEIGHT - frame);

    Scrolly1 = SCREEN_HEIGHT - frame;
    double yeltolas = frame - double(SCREEN_HEIGHT);
    for (int i = 0; i < BallCount; i++) {
        if (!State->animated_menus) {
            break;
        }

        Balls[i].current_r.y += yeltolas;

        kirajzgolyo(Balls[i].current_r, Balls[i].radius, BufferBall);

        vekt2 diff(Balls[i].radius * 0.5 * cos(Balls[i].current_rotation),
                   Balls[i].radius * 0.5 * sin(Balls[i].current_rotation));
        kirajzgolyo(Balls[i].current_r + diff, Balls[i].radius * 0.25, BufferMain);
        kirajzgolyo(Balls[i].current_r - diff, Balls[i].radius * 0.25, BufferMain);

        Balls[i].current_r.y -= yeltolas;
    }

    // Rairjuk szovegeket:
    for (int i = 0; i < elemszam; i++) {
        MenuFont->write(Tmpbuffer, sorok[i].x, sorok[i].y + frame - SCREEN_HEIGHT, sorok[i].text);
    }

    // Rarakjuk sisakot:
    pic8* sisakframe = NULL;
    if (State->animated_menus) {
        sisakframe = Psisak->get_frame_by_time(time * 0.0024);
    } else {
        sisakframe = Psisak->get_frame_by_index(25);
    }

    blit8(Tmpbuffer, sisakframe, sisakx - 20, sisaky - 7 - (SCREEN_HEIGHT - 1) + frame);

    blit8(Tmpbuffer, Intro, 0, frame);
    bltfront(Tmpbuffer);
    return 1;
}

#define MAXHIBASORHOSSZ (34)
typedef char hibasor[MAXHIBASORHOSSZ + 4];
#define MAXHIBASOR (10)
static hibasor Hibasortomb[MAXHIBASOR];

void render_error(const char* text1, const char* text2, const char* text3) {
    // Kirajzoljuk hatteret BufferMain-ba:
    int y = 0;
    int paros = 0;
    while (y < SCREEN_HEIGHT) {
        int x = 0;
        if (paros) {
            x -= Kisnevsima->get_width() / 2;
        }
        paros = !paros;
        while (x < SCREEN_WIDTH) {
            blit8(BufferMain, Kisnevsima, x, y);
            x += Kisnevsima->get_width();
        }
        y += Kisnevsima->get_height();
    }

    int sorszam = 0;
    for (int i = 0; i < 3; i++) {
        const char* text = text1;
        if (i == 1) {
            text = text2;
        }
        if (i == 2) {
            text = text3;
        }
        if (!text) {
            break;
        }

        while (1) {
            if (sorszam >= MAXHIBASOR) {
                break;
            }
            if (strlen(text) <= MAXHIBASORHOSSZ) {
                // Siman belefer:
                strcpy(Hibasortomb[sorszam], text);
                sorszam++;
                break;
            }
            // Tul hosszu sor, le kell vagni belole:
            int j = MAXHIBASORHOSSZ;
            while (j > 0) {
                if (text[j] == ' ') {
                    break;
                }

                j--;
            }
            if (j <= 0) {
                // Nem talalt szovegben space-t:
                strncpy(Hibasortomb[sorszam], text, MAXHIBASORHOSSZ);
                sorszam++;
                text += MAXHIBASORHOSSZ;
            } else {
                strncpy(Hibasortomb[sorszam], text, j);
                sorszam++;
                text += j;
                while (text[j] == ' ') {
                    text++;
                }
            }
        }
    }

    // Rairjuk szovegeket:
    for (int i = 0; i < sorszam; i++) {
        int y = 240 - 20 - sorszam * (40 / 2) + i * 40;
        MenuFont->write_centered(BufferMain, 320, y, Hibasortomb[i]);
    }
    // Kirakjuk:
    bltfront(BufferMain);
    if (MenuPal) {
        MenuPal->set();
    }
}
