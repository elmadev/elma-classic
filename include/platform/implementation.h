#ifndef PLATFORM_IMPLEMENTATION_H
#define PLATFORM_IMPLEMENTATION_H

#include <string>
#include <utility>
#include <vector>

class pic8;

class palette {
    void* data;

  public:
    palette(unsigned char* palette_data);
    ~palette();
    void set();
};

void message_box(const char* text);
bool platform_render_error(pic8* buffer);

void handle_events();

void platform_init();
void init_sound();

void lock_backbuffer(pic8& view, bool flipped);
void unlock_backbuffer();

void get_mouse_position(int* x, int* y);
void set_mouse_position(int x, int y);
bool was_left_mouse_just_clicked();
bool was_right_mouse_just_clicked();
void show_cursor();
void hide_cursor();

int get_mouse_wheel_delta();

std::string get_clipboard_text();

bool is_fullscreen();
long long get_milliseconds();

// Returns true if the resolution changed; the caller must run on_resolution_change().
bool platform_apply_fullscreen_mode();
std::vector<std::pair<int, int>> platform_get_display_modes();
std::pair<int, int> platform_get_desktop_resolution();

void platform_resize_window(int width, int height);
void platform_recreate_window();
bool has_window();
bool platform_save_screenshot();

#endif
