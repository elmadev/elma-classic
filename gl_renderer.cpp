#include "gl_renderer.h"
#include "main.h"

#include <SDL.h>
#include <glad/glad.h>

static SDL_GLContext GLContext = nullptr;
static int FrameWidth = 0;
static int FrameHeight = 0;

int gl_init(SDL_Window* sdl_window, int width, int height) {
    FrameWidth = width;
    FrameHeight = height;

    GLContext = SDL_GL_CreateContext(sdl_window);
    if (!GLContext) {
        internal_error("Failed to create OpenGL context:", SDL_GetError());
        return -1;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        internal_error("Failed to initialize GLAD");
        return -1;
    }

    glViewport(0, 0, width, height);

    // Disable VSync
    SDL_GL_SetSwapInterval(0);

    return 0;
}

void gl_cleanup() {
    if (GLContext) {
        SDL_GL_DeleteContext(GLContext);
        GLContext = nullptr;
    }
}
