#include "gl_renderer.h"
#include "main.h"

#include <SDL.h>
#include <glad/glad.h>

static SDL_GLContext GLContext = nullptr;
static int FrameWidth = 0;
static int FrameHeight = 0;
static GLuint VAO = 0;
static GLuint VBO = 0;
static GLuint IndexTexture = 0;
static GLuint PaletteTexture = 0;

static void setup_textures(int width, int height) {
    // Create index texture (R8)
    glGenTextures(1, &IndexTexture);
    glBindTexture(GL_TEXTURE_2D, IndexTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    // Create palette texture (1D, 256 entries)
    glGenTextures(1, &PaletteTexture);
    glBindTexture(GL_TEXTURE_1D, PaletteTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}

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
    setup_textures(width, height);

    return 0;
}

void gl_cleanup() {
    if (VBO) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (IndexTexture) {
        glDeleteTextures(1, &IndexTexture);
        IndexTexture = 0;
    }
    if (PaletteTexture) {
        glDeleteTextures(1, &PaletteTexture);
        PaletteTexture = 0;
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
