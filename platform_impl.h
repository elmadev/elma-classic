#ifndef PLATFORM_IMPL_H
#define PLATFORM_IMPL_H

// DIK_ Windows scancode
struct DikScancode {
    short key;
    short control;

    constexpr DikScancode() noexcept
        : control(0),
          key(0) {}

    constexpr DikScancode(int val) noexcept
        : control((val & 0xFFFF0000) >> 16),
          key(val & 0x0000FFFF) {}

    constexpr DikScancode(short control, short k) noexcept
        : control(control),
          key(k) {}

    constexpr operator int() const noexcept { return (control << 16) | key; }

    constexpr bool operator==(const DikScancode& b) const noexcept {
        return key == b.key && control == b.control;
    }

    constexpr bool operator==(int b) const noexcept { return *this == DikScancode(b); }

    constexpr bool operator!=(const DikScancode& b) const noexcept { return !(*this == b); }
};

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
