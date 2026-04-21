#ifndef PLATFORM_SDL_KEYBOARD_H
#define PLATFORM_SDL_KEYBOARD_H

#include <SDL.h>

namespace keyboard {

void init();
void begin_frame();
void end_frame();
void record_key_down(SDL_Scancode scancode);
bool is_down(SDL_Scancode sdl_code);
bool was_just_pressed(SDL_Scancode sdl_code);
bool was_down(SDL_Scancode sdl_code);

} // namespace keyboard

#endif
