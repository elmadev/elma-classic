#include "ALL.H"

static pic8* Lockbuff = NULL;

static int Backpiclocked = 0, Frontpiclocked = 0;

// int Mindigfront = 0;

static pic8* Pmindigfpic = NULL;

pic8* lockbackbuffer_pic() {
    if (Backpiclocked || Frontpiclocked) {
        hiba("lbb_p lock!");
    }
    Backpiclocked = 1;

    if (!Lockbuff) {
        Lockbuff = new pic8(10, 10);
        Lockbuff->width = SCREEN_WIDTH;
        Lockbuff->height = SCREEN_HEIGHT;
        Lockbuff->rows = NULL;
    }

    if (Waitforretrace) {
        Lockbuff->rows = lockbackbuffer();
        return Lockbuff;
    } else {
        if (!Pmindigfpic) {
            Pmindigfpic = new pic8(SCREEN_WIDTH, SCREEN_HEIGHT);
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
        Lockbuff->rows = NULL;
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
        Lockbuff->width = SCREEN_WIDTH;
        Lockbuff->height = SCREEN_HEIGHT;
        Lockbuff->rows = NULL;
    }

    Lockbuff->rows = lockfrontbuffer();
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
    blit8(Lockbuff, ppic);
    unlockfrontbuffer_pic();
}
void bltfront(pic8* ppic, int x1, int y1, int x2, int y2) {
    lockfrontbuffer_pic();
    blit8(Lockbuff, ppic, x1, y1, x1, y1, x2, y2);
    unlockfrontbuffer_pic();
}
