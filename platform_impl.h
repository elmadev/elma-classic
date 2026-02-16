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
bool was_left_mouse_just_clicked();
bool was_right_mouse_just_clicked();
void show_cursor();
void hide_cursor();

// Returns true if the key is currently held down.
// Used for continious input detection like game controls
bool is_key_down(DikScancode code);

// Returns true if the key was pressed this frame (edge trigger, not held).
// Used for single-press input detection like menu navigation
bool was_key_just_pressed(DikScancode code);

// Returns the scancode of any key pressed this frame, or DIK_UNKOWN if none.
// Used for detecting any key press for "press any key" prompts
DikScancode get_any_key_just_pressed();

// Returns true if the key is held at OS-level key repeat intervals.
// Used for OS-level key repeat functionality (text input)
bool was_key_down(DikScancode code);

int get_mouse_wheel_delta();

bool is_fullscreen();
long long get_milliseconds();

void platform_recreate_window();
bool has_window();

#endif
