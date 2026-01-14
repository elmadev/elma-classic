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

sprite::sprite(double x, double y, char* kepnev_p, char* texturanev_p, char* maszknev_p) {
    if (!Plgr) {
        internal_error("7uyhfg");
    }

    r = vect2(x, y);
    if (strlen(kepnev_p) > 8 || strlen(maszknev_p) > 8 || strlen(texturanev_p) > 8) {
        internal_error("87vgrg");
    }
    strcpy(kepnev, kepnev_p);
    strcpy(texturanev, texturanev_p);
    strcpy(maszknev, maszknev_p);
    tavolsag = 100;
    hatarol = 0;

    // Default meret:
    xsize = PixelsToMeters * DEFAULTSIZE;
    ysize = PixelsToMeters * DEFAULTSIZE;

    if (kepnev[0] && (maszknev[0] || texturanev[0])) {
        internal_error("sp::sp-ban sok nev egy poligonban!");
    }
    if (kepnev[0]) {
        int index = Plgr->getkepindex(kepnev);
        if (index < 0) {
            kepnev[0] = 0;
        } else {
            // Megvan kep:
            xsize = Plgr->kepek[index].xsize;
            ysize = Plgr->kepek[index].ysize;
            xsize *= PixelsToMeters;
            ysize *= PixelsToMeters;
            tavolsag = Plgr->kepek[index].tavolsag;
            hatarol = Plgr->kepek[index].hatarol;
        }
    } else {
        // Nem kep sprite:
        if (maszknev[0]) {
            int index = Plgr->getmaszkindex(maszknev);
            if (index < 0) {
                maszknev[0] = 0;
            } else {
                xsize = Plgr->maszkok[index].xsize;
                ysize = Plgr->maszkok[index].ysize;
                xsize *= PixelsToMeters;
                ysize *= PixelsToMeters;
            }
        }
        if (texturanev[0]) {
            int index = Plgr->gettexturaindex(texturanev);
            if (index < 0) {
                texturanev[0] = 0;
            } else {
                tavolsag = Plgr->texturak[index].tavolsag;
                hatarol = Plgr->texturak[index].hatarol;
            }
        }
    }
}

void sprite::kirajzol(void) {
    render_line(r, r + vect2(xsize, 0.0), false);
    render_line(r, r + vect2(0.0, ysize), false);
    vect2 r2(xsize + r.x, ysize + r.y);
    render_line(r2, r2 - vect2(xsize, 0.0), false);
    render_line(r2, r2 - vect2(0.0, ysize), false);
}

sprite::sprite(FILE* h) {
    int meret = sizeof(tavolsag);
    if (meret != 4) {
        internal_error("sprite::sprite-ban sizeof( tavolsag ) != 4!");
    }

    if (fread(kepnev, 1, 10, h) != 10) {
        internal_error("Nemo sp!");
    }
    kepnev[9] = 0; // Csak biztonsagert
    if (fread(texturanev, 1, 10, h) != 10) {
        internal_error("Nemo sp!");
    }
    texturanev[9] = 0; // Csak biztonsagert
    if (fread(maszknev, 1, 10, h) != 10) {
        internal_error("Nemo sp!");
    }
    maszknev[9] = 0; // Csak biztonsagert

    if (fread(&r.x, 1, sizeof(r.x), h) != sizeof(r.x)) {
        internal_error("Nem olvas file-bol sprite::sprite-ban!");
    }
    if (fread(&r.y, 1, sizeof(r.y), h) != sizeof(r.y)) {
        internal_error("Nem olvas file-bol sprite::sprite-ban!");
    }
    if (fread(&tavolsag, 1, sizeof(tavolsag), h) != sizeof(tavolsag)) {
        internal_error("Nem olvas file-bol sprite::sprite-ban!");
    }
    if (fread(&hatarol, 1, sizeof(hatarol), h) != sizeof(hatarol)) {
        internal_error("Nem olvas file-bol sprite::sprite-ban!");
    }
}

void sprite::save(FILE* h) {
    if (fwrite(kepnev, 1, 10, h) != 10) {
        internal_error("Nir sp!");
    }
    if (fwrite(texturanev, 1, 10, h) != 10) {
        internal_error("Nir sp!");
    }
    if (fwrite(maszknev, 1, 10, h) != 10) {
        internal_error("Nir sp!");
    }
    if (fwrite(&r.x, 1, sizeof(r.x), h) != sizeof(r.x)) {
        internal_error("Nir sp!");
    }
    if (fwrite(&r.y, 1, sizeof(r.y), h) != sizeof(r.y)) {
        internal_error("Nir sp!");
    }
    if (fwrite(&tavolsag, 1, sizeof(tavolsag), h) != sizeof(tavolsag)) {
        internal_error("Nir sp!");
    }
    if (fwrite(&hatarol, 1, sizeof(hatarol), h) != sizeof(hatarol)) {
        internal_error("Nir sp!");
    }
}

double sprite::belyegszamitas(void) {
    double belyeg = 0;
    belyeg += r.x;
    belyeg += r.y;
    return belyeg;
}
