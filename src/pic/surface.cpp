#include "pic/surface.h"
#include "editor/canvas.h"
#include "editor/editor.h"
#include "eol/settings.h"
#include "main.h"
#include "menu/ball.h"
#include "menu/pic.h"
#include "pic/lgr.h"
#include "pic/pic8.h"
#include "platform/implementation.h"
#include "renderer/render.h"

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

static pic8 Buffer = pic8();

static int BufferLocked = 0;

void on_resolution_change() {
    editor_canvas_update_resolution();
    reinit_menu_pictures();
    if (Level && Lgr) {
        Lgr->reload_default_textures(true);
    }
    invalidate_editor_gui();
    reset_game_background();
    balls_resolution_change();
}

void update_resolution(int width, int height) {
    EolSettings->persist_screen_width(width);
    EolSettings->persist_screen_height(height);
    platform_resize_window(width, height);
    on_resolution_change();
}

pic8* lock_backbuffer_pic(bool flipped) {
    if (BufferLocked) {
        internal_error("lock_backbuffer_pic lock!");
    }
    BufferLocked = 1;
    lock_backbuffer(Buffer, flipped);
    return &Buffer;
}

void unlock_backbuffer_pic() {
    if (!BufferLocked) {
        internal_error("unlock_backbuffer_pic lock!");
    }
    BufferLocked = 0;
    unlock_backbuffer();
}

void blit_to_screen(pic8* pic) {
    pic8* surface = lock_backbuffer_pic(false);
    blit8(surface, pic);
    unlock_backbuffer_pic();
}
