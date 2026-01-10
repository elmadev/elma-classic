#include "menu_pic.h"
#include "ALL.H"
#include "abc8.h"
#include "anim.h"
#include "ball.h"
#include "KIRAJZOL.H"
#include "M_PIC.H"
#include "main.h"
#include "pic8.h"
#include "platform_impl.h"
#include "TOPOL.H"
#include <math.h>
#include <string.h>

// Drawing of the screen when there is no ball. Also used as generic buffer by editor.
pic8* BufferMain = nullptr;
// Darker drawing of the screen when a ball is passing over that section. Also used as backup buffer
// by editor.
pic8* BufferBall = nullptr;

// Background image when there is no ball
static pic8* BackgroundTileMain = nullptr;
// Darker background image when a ball is passing over that section
static pic8* BackgroundTileBall = nullptr;

abc8* MenuFont = nullptr;

// Menu red helmet animation
static anim* Helmet = nullptr;

palette* MenuPal = nullptr;

void init_menu_pictures() {
    if (BufferMain) {
        internal_error("init_menu_pictures already called!");
    }
    BufferMain = new pic8(SCREEN_WIDTH, SCREEN_HEIGHT);
    BufferBall = new pic8(SCREEN_WIDTH, SCREEN_HEIGHT);
    BackgroundTileMain = new pic8("szoveg1.pcx"); // "text1"
    BackgroundTileBall = new pic8("szoveg2.pcx"); // "text2"
    MenuFont = new abc8("menu.abc");
    MenuFont->set_spacing(2);

    ErrorGraphicsLoaded = true;

    pic8* helmet_tmp = new pic8("sisak.pcx"); // "helmet"
    forditkepet(helmet_tmp);
    Helmet = new anim(helmet_tmp, "sisak.pcx");
    delete helmet_tmp;
    helmet_tmp = nullptr;
    Helmet->make_helmet_top();

    balls_init();

    get_pcx_pal("intro.pcx", &MenuPal);
}

menu_pic::menu_pic(bool center_vert) {
    center_vertically = center_vert;
    helmet_x = -100;
    helmet_y = -100;
    line_count = 0;
    image_valid = false;
    lines = new text_line[MENU_MAX_LINES];
    if (!lines) {
        internal_error("menu_pic memory!");
    }
}

menu_pic::~menu_pic() {
    if (!lines) {
        internal_error("menu_pic::~menu_pic !lines!");
    }
    delete lines;
    lines = nullptr;
}

void menu_pic::add_line(const char* text, int x, int y) {
    if (strlen(text) > MENU_LINE_LENGTH) {
        internal_error("menu::add_line strlen(text) > MENU_LINE_LENGTH");
    }
    if (line_count >= MENU_MAX_LINES) {
        internal_error("menu::add_line linecount >= MENU_MAX_LINES");
    }
    strcpy(lines[line_count].text, text);
    lines[line_count].x = x;
    lines[line_count].y = y;
    line_count++;
    image_valid = false;
}

void menu_pic::add_line_centered(const char* text, int x, int y) {
    add_line(text, x - MenuFont->len(text) / 2, y);
}

// Set red helmet pixel position
void menu_pic::set_helmet(int x, int y) {
    helmet_x = x;
    helmet_y = y;
}

void menu_pic::clear() {
    line_count = 0;
    image_valid = false;
}

static pic8* ScreenBuffer = nullptr;

static int ScrollingAnimationY = 0;
static unsigned char* GreenRow = nullptr;
static unsigned char* BlackRow = nullptr;

static void render_ball(vect2 r, double radius, pic8* source_pic) {
    // Solid colors are used to render the balls in top half of the screen during scrolling intro
    if (!GreenRow) {
        GreenRow = new unsigned char[SCREEN_WIDTH];
        BlackRow = new unsigned char[SCREEN_WIDTH];
        for (int i = 0; i < SCREEN_WIDTH; i++) {
            GreenRow[i] = GREEN_PALETTE_ID;
            BlackRow[i] = BLACK_PALETTE_ID;
        }
    }

    // Draw circle
    double x_center = r.x;
    double y_center = r.y;
    int y = (int)(y_center - radius);
    while (y < y_center + radius) {
        if (y < 0 || y >= SCREEN_HEIGHT) {
            y++;
            continue;
        }

        // For each row, determine the width we need to draw
        // Circle equation is (x - center.x)^2 + (y - center.y)^2 = radius^2
        // Rearranging: (x - center.x)^2 = radius^2 - (y - center.y)^2
        // Rearranging: dx^2 = r^2 - dy^2
        // Rearranging: dx = sqrt(r^2 - dy^2)
        double dx = radius * radius - (y - y_center) * (y - y_center);
        if (dx < 0.5) {
            y++;
            continue;
        }
        dx = sqrt(dx);

        // Clip horizontally to the screen
        int x1 = (int)(x_center - dx);
        int x2 = (int)(x_center + dx);
        if (x1 < 0) {
            x1 = 0;
        }
        if (x1 > SCREEN_WIDTH - 1) {
            x1 = SCREEN_WIDTH - 1;
        }
        if (x2 < 0) {
            x2 = 0;
        }
        if (x2 > SCREEN_WIDTH - 1) {
            x2 = SCREEN_WIDTH - 1;
        }
        if (x1 >= x2) {
            y++;
            continue;
        }

        unsigned char* dest = ScreenBuffer->get_row(y) + x1;
        unsigned char* source;
        if (y < ScrollingAnimationY) {
            // Top of scrolling intro animation. Use solid colors
            if (source_pic == BufferBall) {
                source = GreenRow;
            } else {
                source = BlackRow;
            }
        } else {
            // Normal menu (or bottom of scrolling intro animation)
            // Use source image
            source = source_pic->get_row(y) + x1;
        }
        memcpy(dest, source, x2 - x1 + 1);

        y++;
    }
}

static double BallsPrevTime = 0.0;
static double BallsStartTime = 0.0;
static bool IntroAnimation = true;

// Draw the menu. Optionally don't draw the background balls or helmet.
void menu_pic::render(bool skip_balls_helmet) {
    if (!ScreenBuffer) {
        ScreenBuffer = new pic8(SCREEN_WIDTH, SCREEN_HEIGHT);
    }

    // Skip the intro if we are missing intro.pcx (should not occur)
    if (IntroAnimation && !Intro) {
        IntroAnimation = false;
    }

    // Regenerate the image cache if the menu has changed
    if (!image_valid && !IntroAnimation) {
        image_valid = true;

        // Tile the background vertically
        int y = -47;
        int x1 = 0;
        while (y < SCREEN_HEIGHT) {
            int x = x1;
            x1 += 110; // Each row is shifted by 110 pixels
            while (x > 0) {
                x -= BackgroundTileMain->get_width();
            }

            // Tile the background horizontally
            while (x < SCREEN_WIDTH) {
                blit8(BufferMain, BackgroundTileMain, x, y);
                blit8(BufferBall, BackgroundTileBall, x, y);
                x += BackgroundTileMain->get_width();
            }
            y += BackgroundTileMain->get_height();
        }

        for (int i = 0; i < line_count; i++) {
            int x = lines[i].x + SCREEN_WIDTH / 2 - 320;
            int y = lines[i].y;
            if (center_vertically) {
                y += SCREEN_HEIGHT / 2 - 240;
            }
            MenuFont->write(BufferMain, x, y, lines[i].text);
            MenuFont->write(BufferBall, x, y, lines[i].text);
        }
    }

    double time = stopwatch();
    if (IntroAnimation) {
        MenuPal->set();
        IntroAnimation = false;
        empty_keypress_buffer();
        while (true) {
            if (has_keypress()) {
                int c = get_keypress();
                if (c == KEY_ESC || c == KEY_ENTER) {
                    break;
                }
            }
            if (!render_intro_anim(time)) {
                break;
            }
            time = stopwatch();
            BallsStartTime = time;
        }
        ScrollingAnimationY = 0;
        BallsPrevTime = time;
        delete Intro;
        Intro = nullptr;
        return;
    }

    // Display a normal menu (not scrolling)
    double dt;
    if (time < BallsStartTime + 10.0) {
        // The ball will only start moving about 55 milliseconds after the scrolling animation is
        // done
        dt = 0.0000000000001;
    } else {
        // Otherwise, just calculate dt normally
        dt = time - BallsPrevTime;
        BallsStartTime = -100.0;
    }

    // Cap min/max dt and simulate ball movement
    BallsPrevTime = time;
    if (dt < 0.0) {
        dt = 0.001;
    }
    dt *= 3.6;
    if (dt > 100.0) {
        dt = 100.0;
    }
    balls_simulate(dt);

    // By default, the screen uses the "bright" version of the menu
    blit8(ScreenBuffer, BufferMain);

    // Now we need to draw the "dark" version of the menu wherever the ball is
    for (int i = 0; i < BallCount; i++) {
        if (skip_balls_helmet || !State->animated_menus) {
            break;
        }

        // Draw big circle
        render_ball(Balls[i].current_r, Balls[i].radius, BufferBall);
        // Draw two inner subcircles
        vect2 small_circle_offset(Balls[i].radius * 0.5 * cos(Balls[i].current_rotation),
                                  Balls[i].radius * 0.5 * sin(Balls[i].current_rotation));
        render_ball(Balls[i].current_r + small_circle_offset, Balls[i].radius * 0.25, BufferMain);
        render_ball(Balls[i].current_r - small_circle_offset, Balls[i].radius * 0.25, BufferMain);
    }

    if (!skip_balls_helmet) {
        pic8* helmet_frame = nullptr;
        if (State->animated_menus) {
            helmet_frame = Helmet->get_frame_by_time(time * 0.0024);
        } else {
            helmet_frame = Helmet->get_frame_by_index(25);
        }
        int x = helmet_x - 20 + SCREEN_WIDTH / 2 - 320;
        int y = helmet_y - 7; // Helmet is never centered vertically
        blit8(ScreenBuffer, helmet_frame, x, y);
    }

    // We're done!
    bltfront(ScreenBuffer);
}

static bool IntroAnimFirstFrame = true;
pic8* Intro = nullptr;

static double IntroAnimStartTime = 0.0;

// Do the fancy intro scrolling effect. Return 1 if not yet done
// The menu text, helmet and balls start at offset -SCREEN_HEIGHT and move down to 0
// intro.pcx starts at offset 0 and moves down to SCREEN_HEIGHT
// The background tiles start at offset SCREEN_HEIGHT and move up to 0
bool menu_pic::render_intro_anim(double time) {
    if (IntroAnimFirstFrame) {
        balls_simulate(0.000000001);
        IntroAnimFirstFrame = false;
        IntroAnimStartTime = time;

        // Tile background vertically
        int y = -47;
        int x1 = 0;
        while (y < SCREEN_HEIGHT) {
            int x = x1;
            x1 += 110; // Each row is shifted by 110 pixels
            while (x > 0) {
                x -= BackgroundTileMain->get_width();
            }

            // Tile the background horizontally
            while (x < SCREEN_WIDTH) {
                blit8(BufferMain, BackgroundTileMain, x, y);
                blit8(BufferBall, BackgroundTileBall, x, y);
                x += BackgroundTileMain->get_width();
            }
            y += BackgroundTileMain->get_height();
        }
        // Instead of pre-rendering the text into BufferMain/BufferBall,
        // they will be directly drawn onto ScreenBuffer, so we don't do this here
    }

    // Calculate our current progression through time
    int frame = (int)((time - IntroAnimStartTime) * 2.3 * SCREEN_HEIGHT / 480.0);
    if (frame >= SCREEN_HEIGHT) {
        return false;
    }

    // Draw the screen

    // The top is black
    ScreenBuffer->fill_box(BLACK_PALETTE_ID);
    // The background image moves up from SCREEN_HEIGHT to 0
    blit8(ScreenBuffer, BufferMain, 0, SCREEN_HEIGHT - frame);

    // The balls move down from -SCREEN_HEIGHT to 0
    ScrollingAnimationY = SCREEN_HEIGHT - frame;
    double menu_y1 = frame - double(SCREEN_HEIGHT);
    for (int i = 0; i < BallCount; i++) {
        if (!State->animated_menus) {
            break;
        }

        Balls[i].current_r.y += menu_y1;

        // Draw big circle
        render_ball(Balls[i].current_r, Balls[i].radius, BufferBall);
        // Draw two inner subcircles
        vect2 small_circle_offset(Balls[i].radius * 0.5 * cos(Balls[i].current_rotation),
                                  Balls[i].radius * 0.5 * sin(Balls[i].current_rotation));
        render_ball(Balls[i].current_r + small_circle_offset, Balls[i].radius * 0.25, BufferMain);
        render_ball(Balls[i].current_r - small_circle_offset, Balls[i].radius * 0.25, BufferMain);

        Balls[i].current_r.y -= menu_y1;
    }

    // The text moves down from -SCREEN_HEIGHT to 0
    for (int i = 0; i < line_count; i++) {
        int x = lines[i].x + SCREEN_WIDTH / 2 - 320;
        if (center_vertically) {
            internal_error("menu_pic::render_intro_anim should not center vertically!");
        }
        MenuFont->write(ScreenBuffer, x, lines[i].y + frame - SCREEN_HEIGHT, lines[i].text);
    }

    // The helmet moves down from -SCREEN_HEIGHT to 0
    pic8* helmet_frame = nullptr;
    if (State->animated_menus) {
        helmet_frame = Helmet->get_frame_by_time(time * 0.0024);
    } else {
        helmet_frame = Helmet->get_frame_by_index(25);
    }
    int x = helmet_x - 20 + SCREEN_WIDTH / 2 - 320;
    int y = helmet_y - 7; // Helmet is never centered vertically
    blit8(ScreenBuffer, helmet_frame, x, y - SCREEN_HEIGHT + frame);

    // intro.pcx moves down from 0 to SCREEN_HEIGHT
    blit8(ScreenBuffer, Intro, SCREEN_WIDTH / 2 - Intro->get_width() / 2,
          SCREEN_HEIGHT / 2 - Intro->get_height() / 2 + frame);

    // We're done!
    bltfront(ScreenBuffer);
    return true;
}

constexpr int ERROR_MAX_ROW_LENGTH = 34;
constexpr int ERROR_MAX_ROWS = 10;
static char ErrorLines[ERROR_MAX_ROWS][ERROR_MAX_ROW_LENGTH + 4];

void render_error(const char* text1, const char* text2, const char* text3) {
    // Tile the background vertically
    int y = 0;
    int odd_row = 0;
    while (y < SCREEN_HEIGHT) {
        // This is the only place left where the offset is 50% of the tile width
        // Everywhere else it has been replaced with an offset of 110 pixels
        int x = 0;
        if (odd_row) {
            x -= BackgroundTileMain->get_width() / 2;
        }
        odd_row = !odd_row;

        // Tile horizontally
        while (x < SCREEN_WIDTH) {
            blit8(BufferMain, BackgroundTileMain, x, y);
            x += BackgroundTileMain->get_width();
        }
        y += BackgroundTileMain->get_height();
    }

    // Process the text by making sure each line is at most ERROR_MAX_ROW_LENGTH characters long
    // We break up each text1/text2/text3 into multiple components, each of max length
    // ERROR_ROW_MAX_LENGTH We break it up on the last ' ' character, or if none exists, then we
    // just break the word We can have a total of ERROR_MAX_ROWS segments
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

        while (true) {
            if (line_count >= ERROR_MAX_ROWS) {
                break;
            }
            // Text is not too long, just copy it
            if (strlen(text) <= ERROR_MAX_ROW_LENGTH) {
                strcpy(ErrorLines[line_count], text);
                line_count++;
                break;
            }
            // The text is too long, break it up
            // Find the last ' ' character
            int j = ERROR_MAX_ROW_LENGTH;
            while (j > 0) {
                if (text[j] == ' ') {
                    break;
                }
                j--;
            }
            if (j <= 0) {
                // No ' ' found, just cut at the maximum length and copy to the buffer
                strncpy(ErrorLines[line_count], text, ERROR_MAX_ROW_LENGTH);
                line_count++;
                text += ERROR_MAX_ROW_LENGTH;
            } else {
                // Cut at the ' ' and copy to the buffer
                strncpy(ErrorLines[line_count], text, j);
                line_count++;
                text += j;
                while (text[0] == ' ') {
                    text++;
                }
            }
        }
    }
    // Write all the text to the screen
    for (int i = 0; i < line_count; i++) {
        int y = SCREEN_HEIGHT / 2 - 20 - line_count * (40 / 2) + i * 40;
        MenuFont->write_centered(BufferMain, SCREEN_WIDTH / 2, y, ErrorLines[i]);
    }
    // Display the error message
    if (MenuPal) {
        MenuPal->set();
    }
    bltfront(BufferMain);
}
