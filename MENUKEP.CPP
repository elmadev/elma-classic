#include "ALL.H"

pic8* Buffsima = NULL;
pic8* Buffgolyo = NULL;

static pic8 *Kisnevsima = NULL, *Kisnevgolyo = NULL;
abc8* Pmenuabc = NULL;
anim* Psisak = NULL;

ddpal* Pmenupal = NULL;
unsigned char Menupaltomb[768];

void initmenukep1(void) {
    if (Buffsima) {
        hiba("initmenukep1-ben Buffsima!");
    }
    Buffsima = new pic8(SCREEN_WIDTH, SCREEN_HEIGHT);
    Buffgolyo = new pic8(SCREEN_WIDTH, SCREEN_HEIGHT);
    Kisnevsima = new pic8("szoveg1.pcx");
    Kisnevgolyo = new pic8("szoveg2.pcx");
    Pmenuabc = new abc8("menu.abc");
    Pmenuabc->set_spacing(2);

    pic8* sisaktmp = new pic8("sisak.pcx");
    forditkepet(sisaktmp);
    Psisak = new anim(sisaktmp, "sisak.pcx");
    delete sisaktmp;
    sisaktmp = NULL;
    Psisak->make_helmet_top();

    balls_init();
}

void initmenukep2(void) {
    if (Pmenupal) {
        hiba("initmenukep2-ben Pmenupal!");
    }
    get_pcx_pal("intro.pcx", &Pmenupal); // O new-ik egy ddpal-t
    get_pcx_pal("intro.pcx", Menupaltomb);
}

szoveglista::szoveglista(void) {
    sisakx = sisaky = -100;

    elemszam = 0;
    ervenyes = 0;
    sorok = new egyszovsor[MAXSZOVEGSOR];
    if (!sorok) {
        hiba("Memoria!");
    }
}

szoveglista::~szoveglista(void) {
    if (!sorok) {
        hiba("szoveglista::~szoveglista !sorok!");
    }
    delete sorok;
    sorok = NULL;
}

void szoveglista::addszoveg(const char* text, int x, int y) {
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

void szoveglista::addszoveg_kozep(const char* text, int x, int y) {
    addszoveg(text, x - Pmenuabc->len(text) / 2, y);
}

void szoveglista::setsisak(int x, int y) {
    sisakx = x;
    sisaky = y;
}

void szoveglista::clear(void) {
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
            if (forras == Buffgolyo) {
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

void szoveglista::kirajzol(int nemkellgolyo) {
    // Kirakjuk hatteret:
    if (!Tmpbuffer) { // Ideiglenes
        Tmpbuffer = new pic8(SCREEN_WIDTH, SCREEN_HEIGHT);
    }

    if (Kezdoanimjon && !Pintro) {
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
                blit8(Buffsima, Kisnevsima, x, y);
                blit8(Buffgolyo, Kisnevgolyo, x, y);
                x += Kisnevsima->get_width();
            }
            y += Kisnevsima->get_height();
        }

        for (int i = 0; i < elemszam; i++) {
            Pmenuabc->write(Buffsima, sorok[i].x, sorok[i].y, sorok[i].text);
            Pmenuabc->write(Buffgolyo, sorok[i].x, sorok[i].y, sorok[i].text);
        }
    }

    double ujido = mv_stopperido();
    if (Kezdoanimjon) {
        Pmenupal->set();
        while (1) {
            if (!kirajzolanim(ujido)) {
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

    blit8(Tmpbuffer, Buffsima);

    for (int i = 0; i < BallCount; i++) {
        if (nemkellgolyo || !State->animated_menus) {
            break;
        }

        kirajzgolyo(Balls[i].current_r, Balls[i].radius, Buffgolyo);

        vekt2 diff(Balls[i].radius * 0.5 * cos(Balls[i].current_rotation),
                   Balls[i].radius * 0.5 * sin(Balls[i].current_rotation));
        kirajzgolyo(Balls[i].current_r + diff, Balls[i].radius * 0.25, Buffsima);
        kirajzgolyo(Balls[i].current_r - diff, Balls[i].radius * 0.25, Buffsima);
    }
    if (!nemkellgolyo) {
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
pic8* Pintro = NULL;

static double Animkezdete = 0.0;

// Hamis ha animnak mar vege:
int szoveglista::kirajzolanim(double ido) {
    if (Elsokiranim) {
        balls_simulate(0.000000001);
        Elsokiranim = 0;
        Animkezdete = ido;

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
                blit8(Buffsima, Kisnevsima, x, y);
                blit8(Buffgolyo, Kisnevgolyo, x, y);
                x += Kisnevsima->get_width();
            }
            y += Kisnevsima->get_height();
        }
        // Most egyszer kirakunk minden elemet kepre hogy
        // ne akadjon meg elso lejatszas kozben:
        Tmpbuffer->fill_box(Introhatter);
        blit8(Tmpbuffer, Buffsima, 0, 0);

        Scrolly1 = 0;
        double yeltolas = 0.0;
        for (int i = 0; i < BallCount; i++) {
            if (!State->animated_menus) {
                break;
            }

            Balls[i].current_r.y += yeltolas;

            kirajzgolyo(Balls[i].current_r, Balls[i].radius, Buffgolyo);

            vekt2 diff(Balls[i].radius * 0.5 * cos(Balls[i].current_rotation),
                       Balls[i].radius * 0.5 * sin(Balls[i].current_rotation));
            kirajzgolyo(Balls[i].current_r + diff, Balls[i].radius * 0.25, Buffsima);
            kirajzgolyo(Balls[i].current_r - diff, Balls[i].radius * 0.25, Buffsima);

            Balls[i].current_r.y -= yeltolas;
        }

        // Rairjuk szovegeket:
        for (int i = 0; i < elemszam; i++) {
            Pmenuabc->write(Tmpbuffer, sorok[i].x, sorok[i].y, sorok[i].text);
        }

        pic8* sisakframe = NULL;
        sisakframe = Psisak->get_frame_by_index(25);
        blit8(Tmpbuffer, sisakframe, sisakx - 20, sisaky - 7);
        blit8(Tmpbuffer, Pintro, 0, 0);
        // Idaig tartott minden elem kirakasa egyszer
    }

    ido -= Animkezdete;

    int frame = ido * 2.3;

    if (frame >= SCREEN_HEIGHT) {
        Scrolly1 = 0;
        return 0;
    }

    Tmpbuffer->fill_box(Introhatter);
    blit8(Tmpbuffer, Buffsima, 0, SCREEN_HEIGHT - frame);

    Scrolly1 = SCREEN_HEIGHT - frame;
    double yeltolas = frame - double(SCREEN_HEIGHT);
    for (int i = 0; i < BallCount; i++) {
        if (!State->animated_menus) {
            break;
        }

        Balls[i].current_r.y += yeltolas;

        kirajzgolyo(Balls[i].current_r, Balls[i].radius, Buffgolyo);

        vekt2 diff(Balls[i].radius * 0.5 * cos(Balls[i].current_rotation),
                   Balls[i].radius * 0.5 * sin(Balls[i].current_rotation));
        kirajzgolyo(Balls[i].current_r + diff, Balls[i].radius * 0.25, Buffsima);
        kirajzgolyo(Balls[i].current_r - diff, Balls[i].radius * 0.25, Buffsima);

        Balls[i].current_r.y -= yeltolas;
    }

    // Rairjuk szovegeket:
    for (int i = 0; i < elemszam; i++) {
        Pmenuabc->write(Tmpbuffer, sorok[i].x, sorok[i].y + frame - SCREEN_HEIGHT, sorok[i].text);
    }

    // Rarakjuk sisakot:
    pic8* sisakframe = NULL;
    if (State->animated_menus) {
        sisakframe = Psisak->get_frame_by_time(ido * 0.0024);
    } else {
        sisakframe = Psisak->get_frame_by_index(25);
    }

    blit8(Tmpbuffer, sisakframe, sisakx - 20, sisaky - 7 - (SCREEN_HEIGHT - 1) + frame);

    blit8(Tmpbuffer, Pintro, 0, frame);
    bltfront(Tmpbuffer);
    return 1;
}

#define MAXHIBASORHOSSZ (34)
typedef char hibasor[MAXHIBASORHOSSZ + 4];
#define MAXHIBASOR (10)
static hibasor Hibasortomb[MAXHIBASOR];

void grafikushiba(const char* text1, const char* text2, const char* text3) {
    // Kirajzoljuk hatteret Buffsima-ba:
    int y = 0;
    int paros = 0;
    while (y < SCREEN_HEIGHT) {
        int x = 0;
        if (paros) {
            x -= Kisnevsima->get_width() / 2;
        }
        paros = !paros;
        while (x < SCREEN_WIDTH) {
            blit8(Buffsima, Kisnevsima, x, y);
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
        Pmenuabc->write_centered(Buffsima, 320, y, Hibasortomb[i]);
    }
    // Kirakjuk:
    bltfront(Buffsima);
    if (Pmenupal) {
        Pmenupal->set();
    }
}
