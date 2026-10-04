#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include "platform/scancode.h"

bool is_game_key_down(DikScancode code);
template <typename Scancode> bool was_game_key_just_pressed(Scancode code);

#endif
