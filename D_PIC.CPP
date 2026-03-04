#include "M_PIC.H"
#include "EDITUJ.H"
#include "eol_settings.h"
#include "editor_canvas.h"
#include "lgr.h"
#include "main.h"
#include "menu_pic.h"
#include "pic8.h"
#include "platform_impl.h"

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

static pic8 Lockbuff = pic8();

static int Backpiclocked = 0, Frontpiclocked = 0;

static void on_resolution_change() {
    editor_canvas_update_resolution();
    reinit_menu_pictures();
    if (Ptop && Lgr) {
        Lgr->reload_default_textures(true);
    }
}

void update_resolution(int w, int h) {
    EolSettings->set_screen_width(w);
    EolSettings->set_screen_height(h);
    platform_resize_window(w, h);
    on_resolution_change();
}

pic8* lockbackbuffer_pic(bool flipped) {
    if (Backpiclocked || Frontpiclocked) {
        internal_error("lbb_p lock!");
    }
    Backpiclocked = 1;
    lock_backbuffer(Lockbuff, flipped);
    return &Lockbuff;
}

void unlockbackbuffer_pic(void) {
    if (!Backpiclocked || Frontpiclocked) {
        internal_error("ulbb_p lock!");
    }
    Backpiclocked = 0;
    unlock_backbuffer();
}

void lockfrontbuffer_pic(bool flipped) {
    if (Frontpiclocked || Backpiclocked) {
        internal_error("lfb_pw lock!");
    }
    Frontpiclocked = 1;
    lock_frontbuffer(Lockbuff, flipped);
}

void unlockfrontbuffer_pic(void) {
    if (!Frontpiclocked || Backpiclocked) {
        internal_error("ulfb_pw lock!");
    }
    Frontpiclocked = 0;
    unlock_frontbuffer();
}

void ppixelfront(int x, int y, unsigned char szin) {
    if (!Frontpiclocked) {
        internal_error("u53e983w");
    }
    Lockbuff.ppixel(x, y, szin);
}

void bltfront(pic8* ppic) {
    lockfrontbuffer_pic();
    blit8(&Lockbuff, ppic);
    unlockfrontbuffer_pic();
}
void bltfront(pic8* ppic, int x1, int y1, int x2, int y2) {
    lockfrontbuffer_pic();
    blit8(&Lockbuff, ppic, x1, y1, x1, y1, x2, y2);
    unlockfrontbuffer_pic();
}
