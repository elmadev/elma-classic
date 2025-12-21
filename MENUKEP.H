#ifndef MENUKEP_H
#define MENUKEP_H

constexpr unsigned char BLACK_PALETTE_ID = 254;
constexpr unsigned char GREEN_PALETTE_ID = 248;

void init_menu_pictures();
void render_error(const char* text1, const char* text2, const char* text3);

extern pic8* BufferMain;
extern pic8* BufferBall;

extern abc8* MenuFont;

extern ddpal* MenuPal;
extern unsigned char MenuPalData[768];

constexpr int MENU_LINE_LENGTH = 100;
constexpr int MENU_MAX_LINES = 30;

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

  public:
    menu_pic();
    ~menu_pic();
    void add_line(const char* text, int x, int y);
    void add_line_centered(const char* text, int x, int y);
    void set_helmet(int x, int y);
    void clear();
    void render(bool skip_balls_helmet = false);
    bool render_intro_anim(double time);
};

extern pic8* Intro;

#endif
