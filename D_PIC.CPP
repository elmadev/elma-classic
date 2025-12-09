#include "all.h"

static pic8* Lockbuff = NULL;

static int Backpiclocked = 0, Frontpiclocked = 0;

// int Mindigfront = 0;

static pic8* Pmindigfpic = NULL;

pic8* lockbackbuffer_pic(int fizikai) {
    if (Backpiclocked || Frontpiclocked) {
        hiba("lbb_p lock!");
    }
    Backpiclocked = 1;

    if (!Lockbuff) {
        Lockbuff = new pic8(10, 10);
        Lockbuff->xsize = 640;
        Lockbuff->ysize = 480;
        Lockbuff->sormuttomb = NULL;
    }

    if (Waitforretrace) {
        Lockbuff->sormuttomb = lockbackbuffer(fizikai);
        return Lockbuff;
    } else {
        if (!Pmindigfpic) {
            Pmindigfpic = new pic8(640, 480);
        }
        return Pmindigfpic;
    }
}

void unlockbackbuffer_pic(void) {
    if (!Backpiclocked || Frontpiclocked) {
        hiba("ulbb_p lock!");
    }
    Backpiclocked = 0;

    if (Waitforretrace) {
        unlockbackbuffer();
        Lockbuff->sormuttomb = NULL;
    } else {
        bltfront(Pmindigfpic);
    }
}

void lockfrontbuffer_pic(void) {
    if (Frontpiclocked || Backpiclocked) {
        hiba("lfb_pw lock!");
    }
    Frontpiclocked = 1;

    if (!Lockbuff) {
        Lockbuff = new pic8(10, 10);
        Lockbuff->xsize = 640;
        Lockbuff->ysize = 480;
        Lockbuff->sormuttomb = NULL;
    }

    Lockbuff->sormuttomb = lockfrontbuffer();
}

void unlockfrontbuffer_pic(void) {
    if (!Frontpiclocked || Backpiclocked) {
        hiba("ulfb_pw lock!");
    }
    Frontpiclocked = 0;
    unlockfrontbuffer();
}

void ppixelfront(int x, int y, unsigned char szin) {
    if (!Frontpiclocked) {
        hiba("u53e983w");
    }
    Lockbuff->ppixel(x, y, szin);
}

void bltfront(pic8* ppic) {
    lockfrontbuffer_pic();
    blt8(Lockbuff, ppic);
    unlockfrontbuffer_pic();
}
void bltfront(pic8* ppic, int x1, int y1, int x2, int y2) {
    lockfrontbuffer_pic();
    blt8(Lockbuff, ppic, x1, y1, x1, y1, x2, y2);
    unlockfrontbuffer_pic();
}
