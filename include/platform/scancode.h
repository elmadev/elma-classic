#ifndef SCANCODE_H
#define SCANCODE_H

#include <array>
#include <directinput/scancodes.h>

constexpr int MaxKeycode = 256;

// DIK_ Windows scancode
typedef int DikScancode;
static_assert(sizeof(DikScancode) <= 4);

constexpr std::array<DikScancode, 8> MODIFIERS{DIK_LSHIFT, DIK_RSHIFT, DIK_LCONTROL, DIK_RCONTROL,
                                               DIK_LMENU,  DIK_RMENU,  DIK_LWIN,     DIK_RWIN};

class combo_scancode {
  public:
    DikScancode modifier;
    DikScancode key;

    combo_scancode() noexcept
        : modifier(DIK_NONE),
          key(DIK_NONE) {}

    combo_scancode(unsigned long long val)
        : modifier(static_cast<DikScancode>((val >> 32) & 0xFFFFFFFFLL)),
          key(static_cast<DikScancode>(val & 0xFFFFFFFFLL)) {}

    combo_scancode(DikScancode modifier, DikScancode key)
        : modifier(modifier),
          key(key) {}

    constexpr bool operator==(const combo_scancode& b) const noexcept = default;

    constexpr explicit operator unsigned long long() const noexcept {
        return (static_cast<unsigned long long>(static_cast<unsigned int>(modifier)) << 32) |
               static_cast<unsigned int>(key);
    }
};

// Returns true if the key is currently held down.
// Used for continious input detection like game controls
bool is_key_down(DikScancode code);

// Returns true if the key was pressed this frame (edge trigger, not held).
// Used for single-press input detection like menu navigation
bool was_key_just_pressed(DikScancode code);
bool was_key_just_pressed(combo_scancode code);

// Returns the scancode of any key pressed this frame, or DIK_UNKOWN if none.
// Used for detecting any key press for "press any key" prompts
DikScancode get_any_key_just_pressed();

// Returns true if the key is held at OS-level key repeat intervals.
// Used for OS-level key repeat functionality (text input)
bool was_key_down(DikScancode code);

// If Ctrl (Windows) or Command (Mac) is held down
bool is_shortcut_modifier_down();

#endif
