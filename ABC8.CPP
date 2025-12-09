#include "all.h"

static int Hibaismeretlenkar = 0;

int torolfv(void) {
    if (fabs(Pmot1->kor2.omega) > 0.02) {
        return 1;
    } else {
        return 0;
    }
}

abc8::abc8(char* nev) {
    success = 0;
    tav = 0;
    ppsprite = NULL;
    ytomb = NULL;
    {
        ppsprite = new ptrpic8[256];
    }
    if (!ppsprite) {
        uzenet("memory");
        return;
    }
    for (int i = 0; i < 256; i++) {
        ppsprite[i] = NULL;
    }
    {
        ytomb = new short[256];
    }
    if (!ytomb) {
        uzenet("memory");
        return;
    }
    for (int i = 0; i < 256; i++) {
        ytomb[i] = 0;
    }
    FILE* h = qopen(nev, "rb");
    if (!h) {
        hiba("Nem tudta megnyitni abc konstruktor file-t!: ", nev);
        return;
    }
    char tmp[20];
    if (fread(tmp, 4, 1, h) != 1) {
        hiba("Nem tudja olvasni abc-t!: ", nev);
        qclose(h);
        return;
    }
    if (strcmp(tmp, "RA1") != 0) {
        hiba("Nem ervenyes abc file elso negy byte alapjan!: ", nev);
        qclose(h);
        return;
    }
    short darabszam = 0;
    if (fread(&darabszam, 2, 1, h) != 1) {
        hiba("Nem tudja olvasni abc-t!: ", nev);
        qclose(h);
        return;
    }
    if (darabszam <= 0 || darabszam > 256) {
        hiba("abc file darabszama nem stimmel!");
    }
    for (int kkk = 0; kkk < darabszam; kkk++) {
        // Sorban beolvassa betuk spritejait:
        if (fread(tmp, 7, 1, h) != 1) {
            hiba("Nem tudja olvasni abc-t!: ", nev);
            qclose(h);
            return;
        }
        if (strcmp(tmp, "EGYMIX") != 0) {
            hiba("Nem talalta meg EGYMIX-et!: ", nev);
            qclose(h);
            return;
        }
        unsigned char c = -1;
        if (fread(&c, 1, 1, h) != 1) {
            hiba("Nem tudja olvasni abc-t!: ", nev);
            qclose(h);
            return;
        }
        if (fread(&ytomb[c], 2, 1, h) != 1) {
            hiba("Nem tudja olvasni abc-t!: ", nev);
            qclose(h);
            return;
        }
        if (ppsprite[c]) {
            hiba("Egy karakter ketszer van meg abc file-ban!: ", nev);
            return;
        }
        {
            ppsprite[c] = new pic8(".spr", h);
        }
        if (!ppsprite[c]->success) {
            hiba("Nem tudta beolvasni SPRITE-ot abc file-bol!: ", nev);
            return;
        }
    }
    success = 1;
    qclose(h);
}

abc8::~abc8(void) {
    if (ppsprite) {
        for (int i = 0; i < 256; i++) {
            if (ppsprite[i]) {
                delete ppsprite[i];
                ppsprite[i] = NULL;
            }
        }
        delete ppsprite;
        ppsprite = NULL;
    }
    if (ytomb) {
        delete ytomb;
        ytomb = NULL;
    }
}

static int Spacehossz = 5;
static int Spacehossz_menu = 10;

void abc8::write(pic8* pdest, int x, int y, char* text, int tavpar) {
    if (!success) {
        hiba("abc8::write, pedig nem success!");
        return;
    }
    if (tavpar == -1000) {
        tavpar = tav;
    }
    while (*text) {
        int index = (unsigned char)*text;
        if (index == ' ') {
            if (this == Pmenuabc) {
                x += Spacehossz_menu;
            } else {
                x += Spacehossz;
            }
            text++;
            continue;
        }
        if (!ppsprite[index]) {
            if (Hibaismeretlenkar) {
                hiba("abc8::len-ben ismeretlen karakter!");
                return;
            } else {
                text++;
                continue;
            }
        }
        blt8(pdest, ppsprite[index], x, y + ytomb[index]);
        // blt8( pdest, ppsprite[index], x, y );
        x += tavpar + ppsprite[index]->getxsize();

        text++;
    }
}

int abc8::len(char* text, int tavpar) {
    if (!success) {
        hiba("abc8::len, pedig nem success!");
        return 0;
    }
    if (tavpar == -1000) {
        tavpar = tav;
    }
    int hossz = 0;
    while (*text) {
        int index = (unsigned char)*text;
        if (!ppsprite[index]) {
            if (index == ' ') {
                if (this == Pmenuabc) {
                    hossz += Spacehossz_menu;
                } else {
                    hossz += Spacehossz;
                }
                text++;
                continue;
            }
            if (Hibaismeretlenkar) {
                hiba("abc8::len-ben ismeretlen karakter!");
                return 0;
            } else {
                text++;
                continue;
            }
        }
        hossz += tavpar + ppsprite[index]->getxsize();

        text++;
    }
    return hossz;
}

void abc8::settav(int tavpar) { tav = tavpar; }

void abc8::writekozep(pic8* pdest, int x, int y, char* text, int tavpar) {
    int hossz = len(text, tav);
    write(pdest, x - hossz / 2, y, text, tavpar);
}
