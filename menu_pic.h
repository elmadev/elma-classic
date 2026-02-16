#ifndef MENU_PIC_H
#define MENU_PIC_H

#include <string>
#include <vector>
#include <platform_impl.h>

constexpr unsigned char BLACK_PALETTE_ID = 254;
constexpr unsigned char GREEN_PALETTE_ID = 248;

class abc8;
class palette;
class pic8;

extern pic8* BufferMain;
extern pic8* BufferBall;
extern abc8* MenuFont;
extern palette* MenuPalette;
extern pic8* Intro;

struct text_line {
    std::string text;
    int x;
    int y;
};

class menu_pic {
    int helmet_x;
    int helmet_y;
    std::vector<text_line> lines;
    bool image_valid;
    bool center_vertically;

  public:
    menu_pic(bool center_vert = true);
    void add_line(std::string text, int x, int y);
    void add_line_centered(const std::string& text, int x, int y);
    void set_helmet(int x, int y);
    void clear();
    void loop();
    DikScancode loop_until_any_key();
    void render(bool skip_balls_helmet = false);
    bool render_intro_anim(double time);
};

void init_menu_pictures();
void render_error(const char* text1, const char* text2, const char* text3, const char* text4);

#endif
