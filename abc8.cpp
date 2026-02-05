#include "abc8.h"
#include "main.h"
#include "menu_pic.h"
#include "pic8.h"
#include "qopen.h"
#include <cstring>

abc8::abc8(const char* filename) {
    spacing = 0;
    ppsprite = nullptr;
    y_offset = nullptr;
    ppsprite = new ptrpic8[256];
    if (!ppsprite) {
        external_error("memory");
        return;
    }
    for (int i = 0; i < 256; i++) {
        ppsprite[i] = nullptr;
    }
    y_offset = new short[256];
    if (!y_offset) {
        external_error("memory");
        return;
    }
    for (int i = 0; i < 256; i++) {
        y_offset[i] = 0;
    }
    FILE* h = qopen(filename, "rb");
    if (!h) {
        internal_error("Could not open abc8 file:: ", filename);
        return;
    }
    char tmp[20];
    if (fread(tmp, 4, 1, h) != 1) {
        internal_error("Could not read abc8 file: ", filename);
        qclose(h);
        return;
    }
    if (strcmp(tmp, "RA1") != 0) {
        internal_error("Invalid abc8 file header: ", filename);
        qclose(h);
        return;
    }
    short sprite_count = 0;
    if (fread(&sprite_count, 2, 1, h) != 1) {
        internal_error("Could not read abc8 file: ", filename);
        qclose(h);
        return;
    }
    if (sprite_count <= 0 || sprite_count > 256) {
        internal_error("Invalid codepoint count for abc8 file: ", filename);
    }
    for (int i = 0; i < sprite_count; i++) {
        if (fread(tmp, 7, 1, h) != 1) {
            internal_error("Could not read abc8 file: ", filename);
            qclose(h);
            return;
        }
        if (strcmp(tmp, "EGYMIX") != 0) {
            internal_error("Invalid sprite header in abc8 file: ", filename);
            qclose(h);
            return;
        }
        unsigned char c = -1;
        if (fread(&c, 1, 1, h) != 1) {
            internal_error("Could not read abc8 file: ", filename);
            qclose(h);
            return;
        }
        if (fread(&y_offset[c], 2, 1, h) != 1) {
            internal_error("Could not read abc8 file: ", filename);
            qclose(h);
            return;
        }
        if (ppsprite[c]) {
            internal_error("Duplicate codepoint in abc8 file: ", filename);
            return;
        }
        ppsprite[c] = new pic8(".spr", h);
    }

    qclose(h);
}

abc8::~abc8() {
    if (ppsprite) {
        for (int i = 0; i < 256; i++) {
            if (ppsprite[i]) {
                delete ppsprite[i];
                ppsprite[i] = nullptr;
            }
        }
        delete ppsprite;
        ppsprite = nullptr;
    }
    if (y_offset) {
        delete y_offset;
        y_offset = nullptr;
    }
}

static int SpaceWidth = 5;
static int SpaceWidthMenu = 10;

void abc8::write(pic8* dest, int x, int y, const char* text) {
    const char* error_text = text;
    while (*text) {
        int index = (unsigned char)*text;
        // Space character is hardcoded
        if (index == ' ') {
            if (this == MenuFont) {
                x += SpaceWidthMenu;
            } else {
                x += SpaceWidth;
            }
            text++;
            continue;
        }
        if (!ppsprite[index]) {
#ifdef DEBUG
            printf("Missing codepoint %c (0x%02X) in abc8 text: \"%s\"\n", index, index,
                   error_text);
#endif
            text++;
            continue;
        }
        blit8(dest, ppsprite[index], x, y + y_offset[index]);
        x += spacing + ppsprite[index]->get_width();

        text++;
    }
}

int abc8::len(const char* text) {
    const char* error_text = text;
    int width = 0;
    while (*text) {
        int index = (unsigned char)*text;
        // Space character is hardcoded (slightly different to abc8::write)
        if (!ppsprite[index]) {
            if (index == ' ') {
                if (this == MenuFont) {
                    width += SpaceWidthMenu;
                } else {
                    width += SpaceWidth;
                }
                text++;
                continue;
            }
#ifdef DEBUG
            printf("Missing codepoint %c (0x%02X) in abc8 text: \"%s\"\n", index, index,
                   error_text);
#endif
            text++;
            continue;
        }
        width += spacing + ppsprite[index]->get_width();

        text++;
    }
    return width;
}

void abc8::set_spacing(int new_spacing) { spacing = new_spacing; }

bool abc8::has_char(unsigned char c) const {
    // Space is always supported (handled specially in write)
    if (c == ' ') {
        return true;
    }
    return ppsprite && ppsprite[c] != nullptr;
}

void abc8::write_centered(pic8* dest, int x, int y, const char* text) {
    int width = len(text);
    write(dest, x - width / 2, y, text);
}
