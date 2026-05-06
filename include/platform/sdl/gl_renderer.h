#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include <SDL.h>

void gl_init(SDL_Window* sdl_window, int width, int height, int pitch);
void gl_resize(int width, int height, int pitch);
void gl_upload_frame(const unsigned char* indices, int pitch);
void gl_update_palette(const void* palette);
void gl_present();
void gl_cleanup();

#endif // GL_RENDERER_H
