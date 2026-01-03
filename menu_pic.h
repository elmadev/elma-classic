#ifndef MENU_PIC_H
#define MENU_PIC_H

constexpr unsigned char BLACK_PALETTE_ID = 254;
constexpr unsigned char GREEN_PALETTE_ID = 248;

constexpr int MENU_LINE_LENGTH = 100;
constexpr int MENU_MAX_LINES = 200;

class abc8;
class palette;
class pic8;

extern pic8* BufferMain;
extern pic8* BufferBall;
extern abc8* MenuFont;
extern palette* MenuPal;
extern pic8* Intro;

struct text_line {
    char text[MENU_LINE_LENGTH + 2];
    int x;
    int y;
};

class menu_pic {
    int helmet_x;
    int helmet_y;
    text_line* lines;
    int line_count;
    bool image_valid;
    bool center_vertically;

  public:
    menu_pic(bool center_vert = true);
    ~menu_pic();
    void add_line(const char* text, int x, int y);
    void add_line_centered(const char* text, int x, int y);
    void set_helmet(int x, int y);
    void clear();
    void render(bool skip_balls_helmet = false);
    bool render_intro_anim(double time);
};

void init_menu_pictures();
void render_error(const char* text1, const char* text2, const char* text3);

#endif
