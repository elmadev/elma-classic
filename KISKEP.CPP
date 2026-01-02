#include "ALL.H"

kiskep::kiskep(const char* pic8nev, pic8* ppic) {
    tomb = NULL;
    if (!ppic) {
        ppic = new pic8(pic8nev);
    }

    lyuk = ppic->gpixel(0, 0);

    xsize = ppic->get_width();
    ysize = ppic->get_height();
    if (xsize > 255 || ysize > 255) {
        internal_error("kiskepben size > 255!");
    }
    hossz = ysize * 256l;
    tomb = new unsigned char[unsigned(hossz)];
    if (!tomb) {
        external_error("memory!");
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
