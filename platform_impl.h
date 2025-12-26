#ifndef PLATFORM_IMPL_H
#define PLATFORM_IMPL_H

#include <SDL_pixels.h>

class ddpal {
    SDL_Color pal[256];

  public:
    ddpal(unsigned char* tomb);
    void set();
};

void message_box(const char* text);

void handle_events();

void platform_init();
void init_sound();

unsigned char** lock_backbuffer(bool flipped);
void unlock_backbuffer();
unsigned char** lock_frontbuffer(bool flipped);
void unlock_frontbuffer();

void fill_key_state(char* buffer);

void get_mouse_position(int* x, int* y);
void set_mouse_position(int x, int y);
bool left_mouse_clicked();
bool right_mouse_clicked();
void show_cursor();
void hide_cursor();

#endif
