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
        hiba("kiskepben size > 255!");
    }
    hossz = ysize * 256l;
    tomb = new unsigned char[unsigned(hossz)];
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

kiskep::kiskep(int xs, int ys) {
    xsize = xs;
    ysize = ys;
    if (xsize > 255 || ysize > 255) {
        hiba("kiskepben size > 255!");
    }
    hossz = ysize * 256l;
    tomb = new unsigned char[unsigned(hossz)];
    if (!tomb) {
        uzenet("memory!");
    }
}

#define BMP_HEADER_SIZE 1078
#define BMP_WIDTH_EOLMAX 256
#define BMP_WIDTH 149
#define BMP_WIDTH_PADDED 152
#define BMP_EOL_ARRAY_SIZE (BMP_WIDTH_EOLMAX * BMP_HEIGHT)
#define BMP_HEIGHT 101
#define BMP_PIXEL_ARRAY_SIZE (BMP_WIDTH_PADDED * BMP_HEIGHT)

kiskep* kiskep::from_bmp(const char* filename) {
    unsigned char data[BMP_PIXEL_ARRAY_SIZE] = {0};
    FILE* f = fopen(filename, "rb");
    if (!f) {
        return nullptr;
    }

    fseek(f, BMP_HEADER_SIZE, SEEK_SET);
    fread(data, BMP_PIXEL_ARRAY_SIZE, 1, f);
    kiskep* k = new kiskep(149, 101);

    /*
        k->lyuk = 0;
        for (int y = 0; y < k->ysize; y++) {
            for (int x = 0; x < k->xsize; x++) {
                k->tomb[y * 256 + x] = x % 256;
            }
        }*/
    k->lyuk = data[0];
    for (int i = 0; i < BMP_EOL_ARRAY_SIZE; i++) {
        if ((i % BMP_WIDTH_EOLMAX < BMP_WIDTH) && (i / BMP_WIDTH_EOLMAX < BMP_HEIGHT)) {
            k->tomb[i] = data[BMP_WIDTH_PADDED * ((BMP_HEIGHT - 1) - i / BMP_WIDTH_EOLMAX) +
                              (i % BMP_WIDTH_EOLMAX)];
        }
    }

    return k;
}

kiskep::~kiskep(void) {
    if (tomb) {
        delete tomb;
    }
}
