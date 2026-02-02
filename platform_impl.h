#ifndef PLATFORM_IMPL_H
#define PLATFORM_IMPL_H

// DIK_ Windows scancode
typedef int DikScancode;

class palette {
    void* data;

  public:
    palette(unsigned char* palette_data);
    ~palette();
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

void get_mouse_position(int* x, int* y);
void set_mouse_position(int x, int y);
bool left_mouse_clicked();
bool right_mouse_clicked();
void show_cursor();
void hide_cursor();

bool is_key_down(DikScancode code);

bool is_fullscreen();
long long get_milliseconds();

void platform_recreate_window();
bool has_window();

#endif
