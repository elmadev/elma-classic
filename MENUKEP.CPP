#include "ALL.H"

pic8* BufferMain = NULL;
pic8* BufferBall = NULL;

static pic8 *BackgroundTileMain = NULL, *BackgroundTileBall = NULL;
abc8* MenuFont = NULL;
anim* Helmet = NULL;

ddpal* MenuPal = NULL;
unsigned char MenuPalData[768];

void init_menu_pictures1(void) {
    if (BufferMain) {
        hiba("init_menu_pictures1 already called!");
    }
    BufferMain = new pic8(SCREEN_WIDTH, SCREEN_HEIGHT);
    BufferBall = new pic8(SCREEN_WIDTH, SCREEN_HEIGHT);
    BackgroundTileMain = new pic8("szoveg1.pcx");
    BackgroundTileBall = new pic8("szoveg2.pcx");
    MenuFont = new abc8("menu.abc");
    MenuFont->set_spacing(2);

    Margrafikus = true;

    pic8* helmet_tmp = new pic8("sisak.pcx");
    forditkepet(helmet_tmp);
    Helmet = new anim(helmet_tmp, "sisak.pcx");
    delete helmet_tmp;
    helmet_tmp = NULL;
    Helmet->make_helmet_top();

    balls_init();
}

void init_menu_pictures2(void) {
    if (MenuPal) {
        hiba("init_menu_pictures2 already called!");
    }
    get_pcx_pal("intro.pcx", &MenuPal); // O new-ik egy ddpal-t
    get_pcx_pal("intro.pcx", MenuPalData);
}

menu_pic::menu_pic(void) {
    helmet_x = helmet_y = -100;

    line_count = 0;
    image_valid = 0;
    lines = new text_line[MENU_MAX_LINES];
    if (!lines) {
        hiba("menu_pic memory!");
    }
}

menu_pic::~menu_pic(void) {
    if (!lines) {
        hiba("menu_pic::~menu_pic !lines!");
    }
    delete lines;
    lines = NULL;
}

void menu_pic::add_line(const char* text, int x, int y) {
    if (strlen(text) > 100) {
        hiba("menu::add_line strlen(text) > MENU_LINE_LENGTH");
    }
    if (line_count >= MENU_MAX_LINES - 1) {
        hiba("menu::add_line linecount >= MENU_MAX_LINES - 1");
    }
    strcpy(lines[line_count].text, text);
    lines[line_count].x = x;
    lines[line_count].y = y;
    line_count++;
    image_valid = 0;
}

void menu_pic::add_line_centered(const char* text, int x, int y) {
    add_line(text, x - MenuFont->len(text) / 2, y);
}

void menu_pic::set_helmet(int x, int y) {
    helmet_x = x;
    helmet_y = y;
}

void menu_pic::clear(void) {
    line_count = 0;
    image_valid = 0;
}

static pic8* ScreenBuffer = NULL; // Ideiglenes

static int ScrollingAnimationY = 0; // Ennel kiseb x-ek helyere sotet zoldet rak kirazgolyo
static unsigned char BLACK_PALETTE_ID = 254;
static unsigned char* GreenRow = NULL;
static unsigned char* BlackRow = NULL;

static void render_ball(vekt2 r, double radius, pic8* source_pic) {
    if (!GreenRow) {
        GreenRow = new unsigned char[SCREEN_WIDTH];
        BlackRow = new unsigned char[SCREEN_WIDTH];
        for (int i = 0; i < SCREEN_WIDTH; i++) {
            GreenRow[i] = 248;
            BlackRow[i] = BLACK_PALETTE_ID;
        }
    }

    if (WallsDisabled) {
        if (r.x < -500.0 || r.x > 1500.0 || r.y < -500.0 || r.y > 1500.0) {
            return;
        }
    }

    double x_center = r.x;
    double y_center = r.y;
    int y = y_center - radius;
    while (y < y_center + radius) {
        if (y < 0 || y >= SCREEN_HEIGHT) {
            y++;
            continue;
        }

        double dx = radius * radius - (y - y_center) * (y - y_center);
        if (dx < 0.5) {
            y++;
            continue;
        }
        dx = sqrt(dx);
        int x1 = x_center - dx;
        int x2 = x_center + dx;
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
        unsigned char* dest = ScreenBuffer->get_row(y) + x1;
        unsigned char* source;
        if (y < ScrollingAnimationY) {
            if (source_pic == BufferBall) {
                source = GreenRow;
            } else {
                source = BlackRow;
            }
        } else {
            source = source_pic->get_row(y) + x1;
        }
        memcpy(dest, source, x2 - x1 + 1);

        y++;
    }
}

static double BallsPrevTime = 0.0;
static double BallsStartTime = 0.0; // Scroll befejezesekor
static int IntroAnimation = 1;

void menu_pic::render(int skip_balls_helmet) {
    // Kirakjuk hatteret:
    if (!ScreenBuffer) { // Ideiglenes
        ScreenBuffer = new pic8(SCREEN_WIDTH, SCREEN_HEIGHT);
    }

    if (IntroAnimation && !Intro) {
        IntroAnimation = 0;
    }

    if (!image_valid && !IntroAnimation) {
        // Ujrarajzoljuk hatterkepeket:
        image_valid = 1;
        int y = -47;
        int x1 = 0;
        while (y < SCREEN_HEIGHT) {
            int x = x1;
            x1 += 110;
            while (x > 0) {
                x -= BackgroundTileMain->get_width();
            }
            while (x < SCREEN_WIDTH) {
                blit8(BufferMain, BackgroundTileMain, x, y);
                blit8(BufferBall, BackgroundTileBall, x, y);
                x += BackgroundTileMain->get_width();
            }
            y += BackgroundTileMain->get_height();
        }

        for (int i = 0; i < line_count; i++) {
            MenuFont->write(BufferMain, lines[i].x, lines[i].y, lines[i].text);
            MenuFont->write(BufferBall, lines[i].x, lines[i].y, lines[i].text);
        }
    }

    double time = mv_stopperido();
    if (IntroAnimation) {
        MenuPal->set();
        while (1) {
            if (!render_intro_anim(time)) {
                IntroAnimation = 0;
                BallsPrevTime = time;
                return;
            }

            time = mv_stopperido();
            BallsStartTime = time;
            IntroAnimation = 0;
        }
    }

    double dt;
    if (time < BallsStartTime + 10.0) {
        dt = 0.0000000000001;
    } else {
        dt = time - BallsPrevTime;
        BallsStartTime = -100.0;
    }

    BallsPrevTime = time;
    if (dt < 0.0) {
        dt = 0.001;
    }

    dt *= 3.6;
    if (dt > 100.0) {
        dt = 100.0;
    }

    balls_simulate(dt); // 12.0 volt regebben

    blit8(ScreenBuffer, BufferMain);

    for (int i = 0; i < BallCount; i++) {
        if (skip_balls_helmet || !State->animated_menus) {
            break;
        }

        render_ball(Balls[i].current_r, Balls[i].radius, BufferBall);

        vekt2 small_circle_offset(Balls[i].radius * 0.5 * cos(Balls[i].current_rotation),
                                  Balls[i].radius * 0.5 * sin(Balls[i].current_rotation));
        render_ball(Balls[i].current_r + small_circle_offset, Balls[i].radius * 0.25, BufferMain);
        render_ball(Balls[i].current_r - small_circle_offset, Balls[i].radius * 0.25, BufferMain);
    }
    if (!skip_balls_helmet) {
        pic8* helmet_frame = NULL;
        if (State->animated_menus) {
            helmet_frame = Helmet->get_frame_by_time(time * 0.0024);
        } else {
            helmet_frame = Helmet->get_frame_by_index(25);
        }

        blit8(ScreenBuffer, helmet_frame, helmet_x - 20, helmet_y - 7);
    }

    bltfront(ScreenBuffer);
}

static int IntroAnimFirstFrame = 1;
// Teljes olvassa be es elso kiirloading szabaditja fel:
pic8* Intro = NULL;

static double IntroAnimStartTime = 0.0;

// Hamis ha animnak mar vege:
int menu_pic::render_intro_anim(double time) {
    if (IntroAnimFirstFrame) {
        balls_simulate(0.000000001);
        IntroAnimFirstFrame = 0;
        IntroAnimStartTime = time;

        // Berajzoljuk hatterkepeket:
        int y = -47;
        int x1 = 0;
        while (y < SCREEN_HEIGHT) {
            int x = x1;
            x1 += 110;
            while (x > 0) {
                x -= BackgroundTileMain->get_width();
            }
            while (x < SCREEN_WIDTH) {
                blit8(BufferMain, BackgroundTileMain, x, y);
                blit8(BufferBall, BackgroundTileBall, x, y);
                x += BackgroundTileMain->get_width();
            }
            y += BackgroundTileMain->get_height();
        }
        // Most egyszer kirakunk minden elemet kepre hogy
        // ne akadjon meg elso lejatszas kozben:
        ScreenBuffer->fill_box(BLACK_PALETTE_ID);
        blit8(ScreenBuffer, BufferMain, 0, 0);

        ScrollingAnimationY = 0;
        double menu_y1 = 0.0;
        for (int i = 0; i < BallCount; i++) {
            if (!State->animated_menus) {
                break;
            }

            Balls[i].current_r.y += menu_y1;

            render_ball(Balls[i].current_r, Balls[i].radius, BufferBall);

            vekt2 small_circle_offset(Balls[i].radius * 0.5 * cos(Balls[i].current_rotation),
                                      Balls[i].radius * 0.5 * sin(Balls[i].current_rotation));
            render_ball(Balls[i].current_r + small_circle_offset, Balls[i].radius * 0.25,
                        BufferMain);
            render_ball(Balls[i].current_r - small_circle_offset, Balls[i].radius * 0.25,
                        BufferMain);

            Balls[i].current_r.y -= menu_y1;
        }

        // Rairjuk szovegeket:
        for (int i = 0; i < line_count; i++) {
            MenuFont->write(ScreenBuffer, lines[i].x, lines[i].y, lines[i].text);
        }

        pic8* helmet_frame = NULL;
        helmet_frame = Helmet->get_frame_by_index(25);
        blit8(ScreenBuffer, helmet_frame, helmet_x - 20, helmet_y - 7);
        blit8(ScreenBuffer, Intro, 0, 0);
        // Idaig tartott minden elem kirakasa egyszer
    }

    time -= IntroAnimStartTime;

    int frame = time * 2.3;

    if (frame >= SCREEN_HEIGHT) {
        ScrollingAnimationY = 0;
        return 0;
    }

    ScreenBuffer->fill_box(BLACK_PALETTE_ID);
    blit8(ScreenBuffer, BufferMain, 0, SCREEN_HEIGHT - frame);

    ScrollingAnimationY = SCREEN_HEIGHT - frame;
    double menu_y1 = frame - double(SCREEN_HEIGHT);
    for (int i = 0; i < BallCount; i++) {
        if (!State->animated_menus) {
            break;
        }

        Balls[i].current_r.y += menu_y1;

        render_ball(Balls[i].current_r, Balls[i].radius, BufferBall);

        vekt2 small_circle_offset(Balls[i].radius * 0.5 * cos(Balls[i].current_rotation),
                                  Balls[i].radius * 0.5 * sin(Balls[i].current_rotation));
        render_ball(Balls[i].current_r + small_circle_offset, Balls[i].radius * 0.25, BufferMain);
        render_ball(Balls[i].current_r - small_circle_offset, Balls[i].radius * 0.25, BufferMain);

        Balls[i].current_r.y -= menu_y1;
    }

    // Rairjuk szovegeket:
    for (int i = 0; i < line_count; i++) {
        MenuFont->write(ScreenBuffer, lines[i].x, lines[i].y + frame - SCREEN_HEIGHT,
                        lines[i].text);
    }

    // Rarakjuk sisakot:
    pic8* helmet_frame = NULL;
    if (State->animated_menus) {
        helmet_frame = Helmet->get_frame_by_time(time * 0.0024);
    } else {
        helmet_frame = Helmet->get_frame_by_index(25);
    }

    blit8(ScreenBuffer, helmet_frame, helmet_x - 20, helmet_y - 7 - (SCREEN_HEIGHT - 1) + frame);

    blit8(ScreenBuffer, Intro, 0, frame);
    bltfront(ScreenBuffer);
    return 1;
}

#define ERROR_MAX_ROW_LENGTH (34)
typedef char hibasor[ERROR_MAX_ROW_LENGTH + 4];
#define ERROR_MAX_ROWS (10)
static hibasor ErrorLines[ERROR_MAX_ROWS];

void render_error(const char* text1, const char* text2, const char* text3) {
    // Kirajzoljuk hatteret BufferMain-ba:
    int y = 0;
    int odd_row = 0;
    while (y < SCREEN_HEIGHT) {
        int x = 0;
        if (odd_row) {
            x -= BackgroundTileMain->get_width() / 2;
        }
        odd_row = !odd_row;
        while (x < SCREEN_WIDTH) {
            blit8(BufferMain, BackgroundTileMain, x, y);
            x += BackgroundTileMain->get_width();
        }
        y += BackgroundTileMain->get_height();
    }

    int line_count = 0;
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
            if (line_count >= ERROR_MAX_ROWS) {
                break;
            }
            if (strlen(text) <= ERROR_MAX_ROW_LENGTH) {
                // Siman belefer:
                strcpy(ErrorLines[line_count], text);
                line_count++;
                break;
            }
            // Tul hosszu sor, le kell vagni belole:
            int j = ERROR_MAX_ROW_LENGTH;
            while (j > 0) {
                if (text[j] == ' ') {
                    break;
                }

                j--;
            }
            if (j <= 0) {
                // Nem talalt szovegben space-t:
                strncpy(ErrorLines[line_count], text, ERROR_MAX_ROW_LENGTH);
                line_count++;
                text += ERROR_MAX_ROW_LENGTH;
            } else {
                strncpy(ErrorLines[line_count], text, j);
                line_count++;
                text += j;
                while (text[j] == ' ') {
                    text++;
                }
            }
        }
    }

    // Rairjuk szovegeket:
    for (int i = 0; i < line_count; i++) {
        int y = 240 - 20 - line_count * (40 / 2) + i * 40;
        MenuFont->write_centered(BufferMain, 320, y, ErrorLines[i]);
    }
    // Kirakjuk:
    bltfront(BufferMain);
    if (MenuPal) {
        MenuPal->set();
    }
}
