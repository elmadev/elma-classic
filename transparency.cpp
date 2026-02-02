#include "transparency.h"
#include "main.h"
#include "pic8.h"
#include <cstring>

#define MAXBUFFER (20000u)
static int Maxegybefuggo = 255;

// Visszaadja hany db pixel kell szinu, max sor vegeig:
static int kellszinudb(pic8* ppic, int x, int y, unsigned char szin) {
    int darab = 0;
    while (x < ppic->get_width() && ppic->gpixel(x, y) != szin && darab < Maxegybefuggo) {
        x++;
        darab++;
    }
    return darab;
}

// Visszaadja hany db pixel nem kell szinu, max sor vegeig:
static int nemkellszinudb(pic8* ppic, int x, int y, unsigned char szin) {
    int darab = 0;
    while (x < ppic->get_width() && ppic->gpixel(x, y) == szin && darab < Maxegybefuggo) {
        x++;
        darab++;
    }
    return darab;
}

unsigned char* spriteadat8(pic8* ppic, unsigned char szin, unsigned short* pspritehossz) {
    *pspritehossz = 0;
    unsigned char* buffer = nullptr;
    buffer = new unsigned char[MAXBUFFER];
    if (!buffer) {
        external_error("spriteadat8-ban memory!");
        return nullptr;
    }
    int xsize = ppic->get_width();
    int ysize = ppic->get_height();
    unsigned buf = 0;
    for (int y = 0; y < ysize; y++) {
        // Egy sor feldolgozasa:
        int x = 0;
        while (x < xsize) {
            int kellhossz = kellszinudb(ppic, x, y, szin);
            if (kellhossz) {
                // kellek jonnek:
                x += kellhossz;
                buffer[buf++] = 'K';
                buffer[buf++] = kellhossz;
            } else {
                // nemkellek jonnek:
                int nemkellhossz = nemkellszinudb(ppic, x, y, szin);
                x += nemkellhossz;
                buffer[buf++] = 'N';
                buffer[buf++] = nemkellhossz;
            }
            if (buf >= MAXBUFFER) {
                internal_error("Nem fer be egy szegmensbe spriteadat!");
                delete buffer;
                return nullptr;
            }
        }
    }

    // Most letrehozunk egy szegmenst, ami csak olyan hosszu, amilyen kell:

    unsigned char* jobuffer = nullptr;
    jobuffer = new unsigned char[buf];
    if (!jobuffer) {
        external_error("spriteadat8-ban memory!");
        delete buffer;
        return nullptr;
    }
    memcpy(jobuffer, buffer, buf);

    *pspritehossz = buf;

    delete buffer;
    return jobuffer;
}

void spriteosit(pic8* ppic, int index) {
    ppic->transparency_data = spriteadat8(ppic, index, &ppic->transparency_data_length);
}

void spriteosit(pic8* ppic) { spriteosit(ppic, ppic->gpixel(0, 0)); }
