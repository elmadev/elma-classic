#ifndef SCANCODE_H
#define SCANCODE_H

#include <directinput/scancodes.h>

constexpr int MaxKeycode = 256;

// DIK_ Windows scancode
typedef int DikScancode;

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

// If Ctrl (Windows) or Command (Mac) is held down
bool is_shortcut_modifier_down();

#endif
