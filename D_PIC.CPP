#include "M_PIC.H"
#include "main.h"
#include "pic8.h"
#include "platform_impl.h"

int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;

static pic8* Lockbuff = NULL;

static int Backpiclocked = 0, Frontpiclocked = 0;

pic8* lockbackbuffer_pic(bool flipped) {
    if (Backpiclocked || Frontpiclocked) {
        internal_error("lbb_p lock!");
    }
    Backpiclocked = 1;

    if (!Lockbuff) {
        Lockbuff = new pic8(10, 10);
        Lockbuff->width = SCREEN_WIDTH;
        Lockbuff->height = SCREEN_HEIGHT;
        Lockbuff->rows = NULL;
    }
    Lockbuff->rows = lock_backbuffer(flipped);
    return Lockbuff;
}

void unlockbackbuffer_pic(void) {
    if (!Backpiclocked || Frontpiclocked) {
        internal_error("ulbb_p lock!");
    }
    Backpiclocked = 0;

    unlock_backbuffer();
    Lockbuff->rows = NULL;
}

void lockfrontbuffer_pic(bool flipped) {
    if (Frontpiclocked || Backpiclocked) {
        internal_error("lfb_pw lock!");
    }
    Frontpiclocked = 1;

    if (!Lockbuff) {
        Lockbuff = new pic8(10, 10);
        Lockbuff->width = SCREEN_WIDTH;
        Lockbuff->height = SCREEN_HEIGHT;
        Lockbuff->rows = NULL;
    }

    Lockbuff->rows = lock_frontbuffer(flipped);
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
    Lockbuff->ppixel(x, y, szin);
}

void bltfront(pic8* ppic) {
    lockfrontbuffer_pic();
    blit8(Lockbuff, ppic);
    unlockfrontbuffer_pic();
}
void bltfront(pic8* ppic, int x1, int y1, int x2, int y2) {
    lockfrontbuffer_pic();
    blit8(Lockbuff, ppic, x1, y1, x1, y1, x2, y2);
    unlockfrontbuffer_pic();
}
