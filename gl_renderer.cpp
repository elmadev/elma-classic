#include "gl_renderer.h"
#include "main.h"

#include <SDL.h>
#include <glad/glad.h>

static SDL_GLContext GLContext = nullptr;
static int FrameWidth = 0;
static int FrameHeight = 0;
static GLuint VAO = 0;
static GLuint VBO = 0;

static void setup_vertex_data() {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    float vertices[] = {
        // positions  texCoords
        -1.0f, 1.0f,  0.0f, 0.0f, // top-left
        -1.0f, -1.0f, 0.0f, 1.0f, // bottom-left
        1.0f,  -1.0f, 1.0f, 1.0f, // bottom-right
        -1.0f, 1.0f,  0.0f, 0.0f, // top-left
        1.0f,  -1.0f, 1.0f, 1.0f, // bottom-right
        1.0f,  1.0f,  1.0f, 0.0f  // top-right
    };

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

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
    setup_vertex_data();

    return 0;
}

void gl_cleanup() {
    if (VBO) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (VAO) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (GLContext) {
        SDL_GL_DeleteContext(GLContext);
        GLContext = nullptr;
    }
}
