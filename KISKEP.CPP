#include "all.h"

kiskep::kiskep(char* pic8nev, pic8* ppic) {
    tomb = NULL;
    if (!ppic) {
        {
            ppic = new pic8(pic8nev);
        }
    }

    lyuk = ppic->gpixel(0, 0);

    xsize = ppic->getxsize();
    ysize = ppic->getysize();
    if (xsize > 255 || ysize > 255) {
        hiba("kiskepben size > 255!");
    }
    hossz = ysize * 256l;
    {
        tomb = new unsigned char[unsigned(hossz)];
    }
    if (!tomb) {
        uzenet("memory!");
    }
    for (int y = 0; y < ysize; y++) {
        for (int x = 0; x < xsize; x++) {
            tomb[y * 256 + x] = ppic->gpixel(x, y);
        }
    }

    delete ppic;
}

kiskep::~kiskep(void) {
    if (tomb) {
        delete tomb;
    }
}
