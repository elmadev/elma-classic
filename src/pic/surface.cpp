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

static pic8 Lockbuff = pic8();

static int Backpiclocked = 0;

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

void update_resolution(int w, int h) {
    EolSettings->persist_screen_width(w);
    EolSettings->persist_screen_height(h);
    platform_resize_window(w, h);
    on_resolution_change();
}

pic8* lockbackbuffer_pic(bool flipped) {
    if (Backpiclocked) {
        internal_error("lbb_p lock!");
    }
    Backpiclocked = 1;
    lock_backbuffer(Lockbuff, flipped);
    return &Lockbuff;
}

void unlockbackbuffer_pic() {
    if (!Backpiclocked) {
        internal_error("ulbb_p lock!");
    }
    Backpiclocked = 0;
    unlock_backbuffer();
}

void bltfront(pic8* ppic) {
    pic8* surface = lockbackbuffer_pic(false);
    blit8(surface, ppic);
    unlockbackbuffer_pic();
}
