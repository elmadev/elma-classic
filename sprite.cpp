#include "sprite.h"
#include "editor_canvas.h"
#include "LGRFILE.H"
#include "main.h"
#include "physics_init.h"
#include <cstring>

// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE

sprite::sprite(double x, double y, char* pic_name, char* text_name, char* mask_nam) {
    if (!Plgr) {
        internal_error("7uyhfg");
    }

    r = vect2(x, y);
    if (strlen(pic_name) > 8 || strlen(mask_nam) > 8 || strlen(text_name) > 8) {
        internal_error("87vgrg");
    }
    strcpy(picture_name, pic_name);
    strcpy(texture_name, text_name);
    strcpy(mask_name, mask_nam);
    distance = 100;
    clipping = 0;

    // Default meret:
    wireframe_width = PixelsToMeters * DEFAULT_SPRITE_WIREFRAME;
    wireframe_height = PixelsToMeters * DEFAULT_SPRITE_WIREFRAME;

    if (picture_name[0] && (mask_name[0] || texture_name[0])) {
        internal_error("sp::sp-ban sok nev egy poligonban!");
    }
    if (picture_name[0]) {
        int index = Plgr->getkepindex(picture_name);
        if (index < 0) {
            picture_name[0] = 0;
        } else {
            // Megvan kep:
            wireframe_width = Plgr->kepek[index].xsize;
            wireframe_height = Plgr->kepek[index].ysize;
            wireframe_width *= PixelsToMeters;
            wireframe_height *= PixelsToMeters;
            distance = Plgr->kepek[index].tavolsag;
            clipping = Plgr->kepek[index].hatarol;
        }
    } else {
        // Nem kep sprite:
        if (mask_name[0]) {
            int index = Plgr->getmaszkindex(mask_name);
            if (index < 0) {
                mask_name[0] = 0;
            } else {
                wireframe_width = Plgr->maszkok[index].xsize;
                wireframe_height = Plgr->maszkok[index].ysize;
                wireframe_width *= PixelsToMeters;
                wireframe_height *= PixelsToMeters;
            }
        }
        if (texture_name[0]) {
            int index = Plgr->gettexturaindex(texture_name);
            if (index < 0) {
                texture_name[0] = 0;
            } else {
                distance = Plgr->texturak[index].tavolsag;
                clipping = Plgr->texturak[index].hatarol;
            }
        }
    }
}

void sprite::render(void) {
    render_line(r, r + vect2(wireframe_width, 0.0), false);
    render_line(r, r + vect2(0.0, wireframe_height), false);
    vect2 r2(wireframe_width + r.x, wireframe_height + r.y);
    render_line(r2, r2 - vect2(wireframe_width, 0.0), false);
    render_line(r2, r2 - vect2(0.0, wireframe_height), false);
}

sprite::sprite(FILE* h) {
    int meret = sizeof(distance);
    if (meret != 4) {
        internal_error("sprite::sprite-ban sizeof( distance ) != 4!");
    }

    if (fread(picture_name, 1, 10, h) != 10) {
        internal_error("Nemo sp!");
    }
    picture_name[9] = 0; // Csak biztonsagert
    if (fread(texture_name, 1, 10, h) != 10) {
        internal_error("Nemo sp!");
    }
    texture_name[9] = 0; // Csak biztonsagert
    if (fread(mask_name, 1, 10, h) != 10) {
        internal_error("Nemo sp!");
    }
    mask_name[9] = 0; // Csak biztonsagert

    if (fread(&r.x, 1, sizeof(r.x), h) != sizeof(r.x)) {
        internal_error("Nem olvas file-bol sprite::sprite-ban!");
    }
    if (fread(&r.y, 1, sizeof(r.y), h) != sizeof(r.y)) {
        internal_error("Nem olvas file-bol sprite::sprite-ban!");
    }
    if (fread(&distance, 1, sizeof(distance), h) != sizeof(distance)) {
        internal_error("Nem olvas file-bol sprite::sprite-ban!");
    }
    if (fread(&clipping, 1, sizeof(clipping), h) != sizeof(clipping)) {
        internal_error("Nem olvas file-bol sprite::sprite-ban!");
    }
}

void sprite::save(FILE* h) {
    if (fwrite(picture_name, 1, 10, h) != 10) {
        internal_error("Nir sp!");
    }
    if (fwrite(texture_name, 1, 10, h) != 10) {
        internal_error("Nir sp!");
    }
    if (fwrite(mask_name, 1, 10, h) != 10) {
        internal_error("Nir sp!");
    }
    if (fwrite(&r.x, 1, sizeof(r.x), h) != sizeof(r.x)) {
        internal_error("Nir sp!");
    }
    if (fwrite(&r.y, 1, sizeof(r.y), h) != sizeof(r.y)) {
        internal_error("Nir sp!");
    }
    if (fwrite(&distance, 1, sizeof(distance), h) != sizeof(distance)) {
        internal_error("Nir sp!");
    }
    if (fwrite(&clipping, 1, sizeof(clipping), h) != sizeof(clipping)) {
        internal_error("Nir sp!");
    }
}

double sprite::checksum(void) {
    double belyeg = 0;
    belyeg += r.x;
    belyeg += r.y;
    return belyeg;
}
