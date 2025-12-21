#ifndef MENUKEP_H
#define MENUKEP_H

// void init_menu_pictures( void );

void render_error(const char* text1, const char* text2 = NULL, const char* text3 = NULL);

extern pic8* BufferMain;
extern pic8* BufferBall;

extern abc8* MenuFont;

extern ddpal* MenuPal;
extern unsigned char MenuPalData[768];

struct text_line {
    char text[102];
    int x, y;
};

#define MENU_MAX_LINES (30)

class menu_pic {
    int helmet_x, helmet_y;

    text_line* lines;
    int line_count;
    int image_valid;

  public:
    menu_pic(void);
    ~menu_pic(void);
    void add_line(const char* text, int x, int y);
    void add_line_centered(const char* text, int x, int y);
    void set_helmet(int x, int y);
    void clear(void);
    void render(int skip_balls_helmet = 0);
    int render_intro_anim(double time);
};

extern pic8* Intro;

#endif
