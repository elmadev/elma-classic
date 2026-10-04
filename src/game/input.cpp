#include "game/input.h"
#include "eol/console.h"

bool is_game_key_down(DikScancode code) {
    if (Console->is_input_active()) {
        return false;
    }
    return is_key_down(code);
}

template <typename Scancode> bool was_game_key_just_pressed(Scancode code) {
    if (Console->is_input_active()) {
        return false;
    }
    return was_key_just_pressed(code);
}

template bool was_game_key_just_pressed<DikScancode>(DikScancode code);
template bool was_game_key_just_pressed<combo_scancode>(combo_scancode code);
