#include "all.h"

static int ErrorOnMissingCodepoint = 0;

int torolfv(void) {
    if (fabs(Pmot1->kor2.omega) > 0.02) {
        return 1;
    } else {
        return 0;
    }
}

abc8::abc8(char* filename) {
    default_spacing = 0;
    ppsprite = NULL;
    y_offset = NULL;
    ppsprite = new ptrpic8[256];
    if (!ppsprite) {
        uzenet("memory");
        return;
    }
    for (int i = 0; i < 256; i++) {
        ppsprite[i] = NULL;
    }
    y_offset = new short[256];
    if (!y_offset) {
        uzenet("memory");
        return;
    }
    for (int i = 0; i < 256; i++) {
        y_offset[i] = 0;
    }
    FILE* h = qopen(filename, "rb");
    if (!h) {
        hiba("Could not open abc8 file:: ", filename);
        return;
    }
    char tmp[20];
    if (fread(tmp, 4, 1, h) != 1) {
        hiba("Could not read abc8 file: ", filename);
        qclose(h);
        return;
    }
    if (strcmp(tmp, "RA1") != 0) {
        hiba("Invalid abc8 file header: ", filename);
        qclose(h);
        return;
    }
    short sprite_count = 0;
    if (fread(&sprite_count, 2, 1, h) != 1) {
        hiba("Could not read abc8 file: ", filename);
        qclose(h);
        return;
    }
    if (sprite_count <= 0 || sprite_count > 256) {
        hiba("Invalid codepoint count for abc8 file: ", filename);
    }
    for (int i = 0; i < sprite_count; i++) {
        if (fread(tmp, 7, 1, h) != 1) {
            hiba("Could not read abc8 file: ", filename);
            qclose(h);
            return;
        }
        if (strcmp(tmp, "EGYMIX") != 0) {
            hiba("Invalid sprite header in abc8 file: ", filename);
            qclose(h);
            return;
        }
        unsigned char c = -1;
        if (fread(&c, 1, 1, h) != 1) {
            hiba("Could not read abc8 file: ", filename);
            qclose(h);
            return;
        }
        if (fread(&y_offset[c], 2, 1, h) != 1) {
            hiba("Could not read abc8 file: ", filename);
            qclose(h);
            return;
        }
        if (ppsprite[c]) {
            hiba("Duplicate codepoint in abc8 file: ", filename);
            return;
        }
        ppsprite[c] = new pic8(".spr", h);
        if (!ppsprite[c]->success) {
            hiba("Nem tudta beolvasni SPRITE-ot abc file-bol!: ", filename);
            return;
        }
    }

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
    if (y_offset) {
        delete y_offset;
        y_offset = NULL;
    }
}

static int SpaceWidth = 5;
static int SpaceWidthMenu = 10;

void abc8::write(pic8* dest, int x, int y, char* text) {
    while (*text) {
        int index = (unsigned char)*text;
        if (index == ' ') {
            if (this == Pmenuabc) {
                x += SpaceWidthMenu;
            } else {
                x += SpaceWidth;
            }
            text++;
            continue;
        }
        if (!ppsprite[index]) {
            if (ErrorOnMissingCodepoint) {
                hiba("Missing codepoint in abc8!");
                return;
            } else {
                text++;
                continue;
            }
        }
        blt8(dest, ppsprite[index], x, y + y_offset[index]);
        // blt8( pdest, ppsprite[index], x, y );
        x += default_spacing + ppsprite[index]->getxsize();

        text++;
    }
}

int abc8::len(char* text) {
    int width = 0;
    while (*text) {
        int index = (unsigned char)*text;
        if (!ppsprite[index]) {
            if (index == ' ') {
                if (this == Pmenuabc) {
                    width += SpaceWidthMenu;
                } else {
                    width += SpaceWidth;
                }
                text++;
                continue;
            }
            if (ErrorOnMissingCodepoint) {
                hiba("Missing codepoint in abc8!");
                return 0;
            } else {
                text++;
                continue;
            }
        }
        width += default_spacing + ppsprite[index]->getxsize();

        text++;
    }
    return width;
}

void abc8::set_spacing(int spacing) { default_spacing = spacing; }

void abc8::write_centered(pic8* dest, int x, int y, char* text) {
    int hossz = len(text);
    write(dest, x - hossz / 2, y, text);
}
