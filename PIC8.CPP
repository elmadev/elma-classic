#include "all.h"

typedef unsigned char* puchar;

/*class pic8 {
    friend blt8( pic8* pd, pic8* ps, int x = 0, int y = 0,
        int x1 = -10000, int y1 = -10000, int x2 = -1000, int y2 = -10000 );
    int fizkep;
    int lefoglal( xsize, ysize );
    int xsize, ysize, success;
    puchar* sormuttomb;
    puchar* szegmuttomb;
    int     szegnum;
public:
*/

// if true, every second row of allocated images starts on an even address
// active during LGR file read and betolthattereket
int Paratlankepsorok = 0;

int pic8::lefoglal(long xsizep, long ysizep) {
    // size must not be used, only sizep
    if (sormuttomb || szegmuttomb) {
        hiba("B-error in pic8::lefoglal!");
        return 0;
    }
    xsize = short(xsizep);
    ysize = short(ysizep);
    // make xsizep divisible by 4
    if (Paratlankepsorok) {
        xsizep++;
    }
    if (xsizep % 4) {
        xsizep += 4 - xsizep % 4;
    }
    if (xsizep <= 0 || ysizep <= 0) {
        hiba("sizep too small in pic8::lefoglal!");
        return 0;
    }
    long egyszegben = 0x0ff00l / long(xsizep); // so that segment value doesn't approach 65536
    // szegnum will be incremented at the end even with extra
    szegnum = unsigned(ysizep) / egyszegben;
    long pluszszegbensorok = ysizep - szegnum * egyszegben;
    int vanplussz = pluszszegbensorok != 0;
    sormuttomb = new puchar[unsigned(ysizep)];
    if (vanplussz) {
        szegmuttomb = new puchar[unsigned(szegnum + 1)];
    } else {
        szegmuttomb = new puchar[unsigned(szegnum)];
    }
    if (!sormuttomb || !szegmuttomb) {
        uzenet("pic8::alloc memory!");
        return 0;
    }
    for (int i = 0; i < ysizep; i++) {
        sormuttomb[i] = NULL;
    }
    for (int i = 0; i < szegnum; i++) {
        szegmuttomb[i] = NULL;
    }
    if (vanplussz) {
        szegmuttomb[unsigned(szegnum)] = NULL;
    }
    for (int i = 0; i < szegnum; i++) {
        szegmuttomb[i] = new unsigned char[unsigned(egyszegben * xsizep)];
        if (!szegmuttomb[i]) {
            uzenet("pic8::alloc memory!");
            return 0;
        }
        for (int j = 0; j < egyszegben; j++) {
            sormuttomb[unsigned(i * egyszegben + j)] = &szegmuttomb[i][unsigned(j * xsizep)];
        }
    }
    if (vanplussz) {
        szegmuttomb[unsigned(szegnum)] = new unsigned char[unsigned(pluszszegbensorok * xsizep)];
        if (!szegmuttomb[unsigned(szegnum)]) {
            uzenet("pic8::alloc memory!");
            return 0;
        }
        for (int j = 0; j < pluszszegbensorok; j++) {
            sormuttomb[unsigned(szegnum * egyszegben + j)] =
                &szegmuttomb[unsigned(szegnum)][unsigned(j * xsizep)];
        }
    }

    if (vanplussz) {
        szegnum++;
    }

    for (int y = 0; y < ysizep; y++) {
        memset(sormuttomb[y], 0, unsigned(xsizep)); // set to zero
        if (Paratlankepsorok && (y % 2)) {
            sormuttomb[y]++;
        }
    }

    return 1;
}

pic8::~pic8(void) {
    if (nemdestrukt) {
        return;
    }
    if (sormuttomb) {
        delete sormuttomb;
    }
    if (szegmuttomb) {
        for (int i = 0; i < szegnum; i++) {
            delete szegmuttomb[i];
        }
    }
    delete szegmuttomb;
    if (sprite) {
        delete sprite;
    }
}

pic8::pic8(int xsizep, int ysizep) {
    success = 0;
    nemdestrukt = 0;
    sormuttomb = NULL;
    szegmuttomb = NULL;
    fizkep = 0;
    sprite = NULL;
    spritehossz = NULL;
    success = lefoglal(xsizep, ysizep);
}

// adds pic8 on top of an existing array, won't be deleted by destructor
pic8::pic8(int xsizep, int ysizep, unsigned char** ppc) {
    success = 0;
    nemdestrukt = 1;
    sormuttomb = NULL;
    szegmuttomb = NULL;
    fizkep = 0;
    sprite = NULL;
    spritehossz = NULL;
    if (xsizep == -12333 && ysizep == -12334) {
        hiba("fizkep nondestruct in pic8 constructor!"); //duno correct term for this -a
        return;
    }
    xsize = short(xsizep);
    ysize = short(ysizep);
    if (!ppc) {
        hiba("!ppc nondestruct in pic8 constructor!");
    }
    sormuttomb = ppc;
    success = 1;
}

pic8::pic8(char* nev, FILE* h) {
    nemdestrukt = 0;
    success = 0;
    sormuttomb = NULL;
    szegmuttomb = NULL;
    fizkep = 0;
    sprite = NULL;
    spritehossz = NULL;
    // find correct extension
    int i = strlen(nev) - 1;
    while (i >= 0) {
        if (nev[i] == '.') {
            if (strcmpi(nev + i, ".spr") == 0) {
                spritebeolvas(nev, h);
                return;
            }
            if (strcmpi(nev + i, ".pcx") == 0) {
                pcxbeolvas(nev, h);
                return;
            }
            hiba("pic8 constructor doesn't recognize extension!: ", nev);
            return;
        }
        i--;
    }
    hiba("pic8 constructor cannot find '.'!: ", nev);
}

int pic8::save(char* nev, unsigned char* pal, FILE* h) {
    if (!success) {
        hiba("pic8::save unsuccessful!");
        return 0;
    }
    if (fizkep) {
        hiba("Call for physical screen save!");
        return 0;
    }
    // find extension
    int i = 0;
    while (nev[i]) {
        if (nev[i] == '.') {
            if (strcmpi(nev + i, ".spr") == 0) {
                return spritesave(nev, h);
            }
            if (strcmpi(nev + i, ".pcx") == 0) {
                return pcxsave(nev, pal);
            }
            // if( strcmpi( nev+i, ".mix" ) == 0 ) {
            //   return mixsave( nev );
            // }
            hiba("pic8::save doesn't recognize extension!: ", nev);
            return 0;
        }
        i++;
    }
    hiba("pic8::save cannot find '.'!: ", nev);
    return 0;
}

int pic8::getxsize(void) { return xsize; }

int pic8::getysize(void) { return ysize; }

void pic8::ppixel(int x, int y, unsigned char szin) {
#ifdef PIC8TEST
    if (!success) {
        hiba("success = 0 in pic8::ppixel!");
        return;
    }
#endif
    if (x < 0 || x >= xsize || y < 0 || y >= ysize) {
        return;
    }

    if (!fizkep) {
        sormuttomb[y][x] = szin;
        return;
    }
    // physical
    hiba("ppixel onto physical!");
}

unsigned char pic8::gpixel(int x, int y) {
#ifdef PIC8TEST
    if (!success) {
        hiba("success = 0 in pic8::ppixel!");
        return 0;
    }
#endif
    if (x < 0 || x >= xsize || y < 0 || y >= ysize) {
        return 0;
    }

    if (!fizkep) {
        return sormuttomb[y][x];
    } else {
        // physical
        hiba("gpixel onto physical!");
        // return (unsigned char)gpixel_l( x, y );
        return 0;
    }
}

#ifdef TEST

// otherwise inline

unsigned char* pic8::getptr(int y) {
#ifdef PIC8TEST
    if (!success) {
        hiba("pic8::getptr success = 0!");
        return 0;
    }
    if (y < 0 || y >= ysize) {
        hiba("pic8::getptr y outside!");
        return 0;
    }
#endif
    if (!fizkep) {
        return sormuttomb[y];
    } else {
        // physical
        hiba("pic8::getptr called for physical screen!");
        return NULL;
    }
}

#endif

void pic8::fillbox(unsigned char szin) { fillbox(0, 0, getxsize() - 1, getysize() - 1, szin); }

void pic8::fillbox(int x1, int y1, int x2, int y2, unsigned char szin) {
    if (fizkep) {
        hiba("pic8::fillbox called for physical screen!");
    }
    if (x1 > x2) {
        int tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y1 > y2) {
        int tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    if (x1 < 0) {
        x1 = 0;
    }
    if (y1 < 0) {
        y1 = 0;
    }
    if (x2 >= xsize) {
        x2 = xsize - 1;
    }
    if (y2 >= ysize) {
        y2 = ysize - 1;
    }
    int xhossz = x2 - x1 + 1;
    for (int y = y1; y <= y2; y++) {
        memset(sormuttomb[y] + x1, szin, xhossz);
    }
}

void pic8::vizszegmens_look(int x, int y, int size, unsigned char* lookup) {
#ifdef PIC8TEST
    if (x < 0 || y < 0 || x + size - 1 >= xsize || y >= ysize) {
        hiba("x, y outside in pic8::vizszegmens!");
    }
#endif
    unsigned char* psor = getptr(y);
    for (int i = 0; i < size; i++) {
        psor[x + i] = lookup[psor[x + i]];
    }
    // memset( &psor[x], szin, size );
}

void pic8::fuggszegmens_look(int x, int y, int size, unsigned char* lookup) {
#ifdef PIC8TEST
    if (x < 0 || y < 0 || x >= xsize || y + size - 1 >= ysize) {
        hiba("x, y outside in pic8::vizszegmens!");
    }
#endif
    for (int i = 0; i < size; i++) {
        unsigned char* psor = getptr(y + i);
        psor[x] = lookup[psor[x]];
    }

    // for( int i = 0; i < size; i++ )
    //	ppixel( x, y+i, szin );
}

// SPRITE:

/*static void dummy( long l ) {
    l++;
} */

void pic8::spritebeolvas(char* nev, FILE* h) {
    int volth = 1;
    if (!h) {
        volth = 0;
        h = qopen(nev, "rb");
        if (!h) {
            hiba("pic8 sprite reader cannot open file!: ", nev);
            return;
        }
    }
    unsigned char c = 0;
    if (fread(&c, 1, 1, h) != 1) {
        hiba("Cannot read from file!: ", nev);
        if (!volth) {
            qclose(h);
        }
        return;
    }
    if (c != 0x2d) {
        hiba("SPRITE file first character not 0x2d!: ", nev);
        if (!volth) {
            qclose(h);
        }
        return;
    }

    unsigned short xsize_s = -1, ysize_s = -1;
    if (fread(&xsize_s, 2, 1, h) != 1 || fread(&ysize_s, 2, 1, h) != 1) {
        hiba("Cannot read from SPRITE file!: ", nev);
        if (!volth) {
            qclose(h);
        }
        return;
    }
    xsize = xsize_s;
    ysize = ysize_s;
    if (xsize < 1 || ysize < 1) {
        hiba("Small sizes in SPRITE file!: ", nev);
        if (!volth) {
            qclose(h);
        }
        return;
    }
    if (!lefoglal(xsize, ysize)) {
        return;
    }

    for (int y = 0; y < ysize; y++) {
        // read one line
        if (fread(sormuttomb[y], xsize, 1, h) != 1) {
            hiba("Cannot read from SPRITE file!: ", nev);
            if (!volth) {
                qclose(h);
            }
            return;
        }
    }

    // read SPRITE parts
    char tmp[10] = "";
    if (fread(tmp, 7, 1, h) != 1) {
        hiba("Cannot read from SPRITE file!: ", nev);
        if (!volth) {
            qclose(h);
        }
        return;
    }
    if (strcmp(tmp, "SPRITE") != 0) {
        hiba("Sprite keyword wrong in SPRITE file!: ", nev);
        if (!volth) {
            qclose(h);
        }
        return;
    }

    spritehossz = -1;
    if (fread(&spritehossz, 2, 1, h) != 1) {
        hiba("Cannot read from SPRITE file!: ", nev);
        if (!volth) {
            qclose(h);
        }
        return;
    }
    if (spritehossz < 1) {
        hiba("Sprite length too short in SPRITE file!: ", nev);
        if (!volth) {
            qclose(h);
        }
        return;
    }

    // long l = farcoreleft();
    // dummy( l );
    sprite = new unsigned char[spritehossz];
    if (!sprite) {
        hiba("Not enough space in SPRITE reader!: ", nev);
        if (!volth) {
            qclose(h);
        }
        return;
    }
    if (fread(sprite, spritehossz, 1, h) != 1) {
        hiba("Cannot read sprite from SPRITE file!: ", nev);
        if (!volth) {
            qclose(h);
        }
        return;
    }
    if (!volth) {
        qclose(h);
    }
    success = 1;
}

int pic8::spritesave(char* nev, FILE* h) {
    int volth = 1;
    if (!h) {
        volth = 0;
        h = fopen(nev, "wb");
        if (!h) {
            hiba("pic8::spritesave cannot open file!: ", nev);
            return 0;
        }
    }
    unsigned char c = 0x2d;
    if (fwrite(&c, 1, 1, h) != 1 || fwrite(&xsize, 2, 1, h) != 1 || fwrite(&ysize, 2, 1, h) != 1) {
        hiba("pic8::spritesave cannot write to file!: ", nev);
        if (!volth) {
            fclose(h);
        }
        return 0;
    }

    for (int y = 0; y < ysize; y++) {
        // write one line
        if (fwrite(sormuttomb[y], xsize, 1, h) != 1) {
            hiba("Cannot write to SPRITE file!: ", nev);
            if (!volth) {
                fclose(h);
            }
            return 0;
        }
    }

    // write sprite part
    if (fwrite("SPRITE", 7, 1, h) != 1 || fwrite(&spritehossz, 2, 1, h) != 1 ||
        fwrite(sprite, spritehossz, 1, h) != 1) {
        hiba("Cannot write to SPRITE file!: ", nev);
        if (!volth) {
            fclose(h);
        }
        return 0;
    }
    if (!volth) {
        fclose(h);
    }
    return 1;
}

// PCX:
struct pcxdescriptor {
    unsigned char ManufactId, VersionNum, EncodingTech, BitsPerPlane;
    short Xmin, Ymin, Xmax, Ymax;
    short HorRes, VertRes;
    unsigned char ColorMap[48], Reserved, NumberOfBitPlanes;
    short BytesPerScanLine, PaletteInf;
    unsigned char Padding[127 - 70 + 1];
};

void pic8::pcxbeolvas(char* nev, FILE* h) {
    // slop
    success = 1;

    int zarando = 0;
    if (!h) {
        zarando = 1;
        h = qopen(nev, "rb");
        if (!h) {
            hiba("Cannot open PCX file!: ", nev);
        }
    }
    pcxdescriptor desc;
    if (fread(&desc, sizeof(desc), 1, h) != 1) {
        hiba("Cannot read from PCX file!: ", nev);
    }
    if ((desc.VersionNum != 5) || (desc.ManufactId != 10) || (desc.EncodingTech != 1) ||
        (desc.BitsPerPlane != 8) || (desc.NumberOfBitPlanes != 1)) {
        hiba("\nWrong PCX file header!", nev);
    }
    // if( !lefoglal( desc.Xmax-desc.Xmin+1, desc.Ymax-desc.Ymin+1 ) ) {
    if (!lefoglal(desc.Xmax - desc.Xmin + 1, desc.Ymax - desc.Ymin + 1)) {
        hiba("pcx !lefoglal!");
    }
    for (int y = 0; y < ysize; y++) {
        // read one line
        short nnn = 0, ccc, iii;

        do {
            unsigned char szin;
            long l = fread(&szin, 1, 1, h);
            ccc = szin;
            if (l != 1) {
                  hiba("Cannot read from PCX file!: ", nev);
            }

            if ((ccc & 0xc0) == 0xc0) {
                iii = ccc & (short)0x3f;
                l = fread(&szin, 1, 1, h);
                ccc = szin;
                if (l != 1) {
                       hiba("Cannot read from PCX file!: ", nev);
                }

                while (iii--) {
                    if (nnn < xsize) {
                        ppixel(nnn, y, (unsigned char)ccc);
                    }
                    nnn++;
                }
            } else {
                if (nnn < xsize) {
                    ppixel(nnn, y, (unsigned char)ccc);
                }
                nnn++;
            }
        } while (nnn < desc.BytesPerScanLine);
    }

    // success already set to 1 at the start
    if (zarando) {
        qclose(h);
    }
}

static int numberofrepeats(pic8* ppic, int x, int y, int xsize) {
    unsigned char szin = ppic->gpixel(x, y);
    x++;
    int db = 1;
    while (x < xsize && ppic->gpixel(x, y) == szin) {
        x++;
        db++;
    }
    return db;
}

int pic8::pcxsave(char* nev, unsigned char* pal) {
    FILE* h = fopen(nev, "wb");
    if (!h) {
        hiba("pcxsave cannot open file!: ", nev);
        return 0;
    }
    pcxdescriptor desc;
    desc.ManufactId = 10;
    desc.VersionNum = 5;
    desc.EncodingTech = 1;
    desc.BitsPerPlane = 8;
    desc.Xmin = desc.Ymin = 0;
    desc.Xmax = (short)(xsize - 1);
    desc.Ymax = (short)(ysize - 1);
    desc.HorRes = 10;
    desc.VertRes = 10;
    desc.Reserved = 0;
    desc.NumberOfBitPlanes = 1;
    desc.BytesPerScanLine = (unsigned short)xsize;
    desc.PaletteInf = 1;
    if (fwrite(&desc, sizeof(desc), 1, h) != 1) {
        hiba("pcxsave cannot write to file!: ", nev);
        fclose(h);
        return 0;
    }
    for (int y = 0; y < ysize; y++) {
        int x = 0;
        while (x < xsize) {
            int i = numberofrepeats(this, x, y, xsize);
            if (i > 1) {
                if (i > 63) {
                    i = 63;
                }
                unsigned char controll = (unsigned char)(i + 192);
                if (fwrite(&controll, 1, 1, h) != 1) {
                        hiba("pcxsave cannot write to file!: ", nev);
                    fclose(h);
                    return 0;
                }
                unsigned char szin = gpixel(x, y);
                if (fwrite(&szin, 1, 1, h) != 1) {
                        hiba("pcxsave cannot write to file!: ", nev);
                    fclose(h);
                    return 0;
                }
                x += i;
            } else {
                unsigned char szin = gpixel(x, y);
                if (szin < 64) {
                    if (fwrite(&szin, 1, 1, h) != 1) {
                        hiba("pcxsave cannot write to file!: ", nev);
                        fclose(h);
                        return 0;
                    }
                } else {
                    unsigned char controll = 193;
                    if (fwrite(&controll, 1, 1, h) != 1) {
                        hiba("pcxsave cannot write to file!: ", nev);
                        fclose(h);
                        return 0;
                    }
                    szin = gpixel(x, y);
                    if (fwrite(&szin, 1, 1, h) != 1) {
                        hiba("pcxsave cannot write to file!: ", nev);
                        fclose(h);
                        return 0;
                    }
                }
                x++;
            }
        }
    }
    unsigned char magikus = 0x0c;
    if (fwrite(&magikus, 1, 1, h) != 1) {
        hiba("pcxsave cannot write to file!: ", nev);
        fclose(h);
        return 0;
    }
    if (pal) {
        // palette exists
        for (int i = 0; i < 768; i++) {
            unsigned char c = (unsigned char)(pal[i] << 2);
            if (fwrite(&c, 1, 1, h) != 1) {
                hiba("pcxsave cannot write to file!: ", nev);
                fclose(h);
                return 0;
            }
        }
    } else {
        // no palette given
        for (int i = 0; i < 256; i++) {
            unsigned char c = (unsigned char)i;
            for (int j = 0; j < 3; j++) {
                if (fwrite(&c, 1, 1, h) != 1) {
                    hiba("pcxsave cannot write to file!: ", nev);
                    fclose(h);
                    return 0;
                }
            }
        }
    }

    fclose(h);
    return 1;
}

// int Debugblt = 0;
void blt8(pic8* pd, pic8* ps, int x, int y, int x1, int y1, int x2, int y2) {
#ifdef PIC8TEST
    if (y2 == -10000 && (x1 != -10000 || y1 != -10000 || x2 != -10000)) {
        hiba("blt8 calls y2 == -10000 (default), but not all before!");
        return;
    }
    if (!pd || !ps) {
        hiba("blt8 call, dest or source NULL!");
        return;
    }
#endif

    // if( Debugblt ) came here
    //	hiba( "Here 17!" ); 

    if (pd->sprite) {
        hiba("blt8 called, but dest sprite!");
        return;
    }
    if (x1 == -10000) {
        x1 = y1 = 0;
        x2 = ps->xsize - 1;
        y2 = ps->ysize - 1;
    } else {
        // set order
        if (x2 < x1) {
            int tmp = x1;
            x1 = x2;
            x2 = tmp;
        }
        if (y2 < y1) {
            int tmp = y1;
            y1 = y2;
            y2 = tmp;
        }
    }
    // cut parts in x direction
    // first according to source picture
    if (x1 < 0) {
        x += -x1;
        x1 = 0;
    }
    if (x2 >= ps->xsize) {
        x -= x2 - (ps->xsize - 1);
        x2 = ps->xsize - 1;
    }
    // now according to dest picture
    if (x < 0) {
        x1 += -x;
        x = 0;
    }
    if (x + (x2 - x1) >= pd->xsize) {
        x2 -= x + (x2 - x1) - (pd->xsize - 1);
    }
    if (x1 > x2) {
        return;
    }

    // cut parts in y direction
    // first according to source picture
    if (y1 < 0) {
        y += -y1;
        y1 = 0;
    }
    if (y2 >= ps->ysize) {
        y -= y2 - (ps->ysize - 1);
        y2 = ps->ysize - 1;
    }
    // now according to dest picture
    if (y < 0) {
        y1 += -y;
        y = 0;
    }
    if (y + (y2 - y1) >= pd->ysize) {
        y2 -= y + (y2 - y1) - (pd->ysize - 1);
    }
    if (y1 > y2) {
        return;
    }

    if (ps->sprite) {
        // play sprite
        // need to go through whole sprite and cut
        unsigned buf = 0;
        unsigned char* buffer = ps->sprite;
        int desty = y - y1;
        for (int sy = 0; sy < ps->ysize; sy++) {
            int ybentvan = 1;
            if (sy < y1 || sy > y2) {
                ybentvan = 0;
            }
            int sx = 0;
            while (sx < ps->xsize) {
                switch (buffer[buf++]) {
                case 'K':
                    if (ybentvan) {
                        if (sx < x1 || sx + buffer[buf] - 1 > x2) {
                            if (!(sx > x2 || sx + buffer[buf] - 1 < x1)) {
                                // excess, but has part inside
                                int xkezd = sx;
                                int xveg = sx + buffer[buf] - 1;
                                if (xkezd < x1) {
                                    xkezd = x1;
                                }
                                if (xveg > x2) {
                                    xveg = x2;
                                }
                                if (pd->fizkep) {
                                    hiba("t4ruh5t");
                                }
                                // putline8_l( x+xkezd-x1, desty,
                                //&ps->sormuttomb[sy][xkezd],
                                // xveg-xkezd+1 );
                                else {
                                    memcpy(&pd->sormuttomb[desty][x + xkezd - x1],
                                           &ps->sormuttomb[sy][xkezd], xveg - xkezd + 1);
                                }
                            }
                        } else {
                            // no excess
                            if (pd->fizkep) {
                                hiba("68464");
                            }
                            // putline8_l( x+sx-x1, desty,
                            //&ps->sormuttomb[sy][sx],
                            // buffer[buf] );
                            else {
                                memcpy(&pd->sormuttomb[desty][x + sx - x1], &ps->sormuttomb[sy][sx],
                                       buffer[buf]);
                            }
                        }
                    }
                    sx += buffer[buf++];
                    break;
                case 'N':
                    sx += buffer[buf++];
                    break;
                default:

                    hiba("Sprite in blt with default interpreter!");
                    return;
                }
            }
            desty++;
        }

        return;
    }

    // now the coords are correct
    if (pd->fizkep) {
        // write to physical screen:
        hiba("0895t");
    }
    if (ps->fizkep) {
        // read from physical screen
        hiba("8795t");
    }
    // memory-memory transfer
    int xmeret = x2 - x1 + 1;
    int dfy = y;

    // if( Debugblt )	came here
    //	hiba( "Here 18!" );

    for (int fy = y1; fy <= y2; fy++) {
        memcpy(&pd->sormuttomb[dfy++][x], &ps->sormuttomb[fy][x1], xmeret);
    }
    // if( Debugblt )   came here
    //	hiba( "Here 19!" );
}

/*void blt8fizrefejjelle( pic8* ps ) {
    int fizxsize = Pscr8->getxsize();
    int fizysize = Pscr8->getysize();
    #ifdef PIC8TEST
        if( ps == Pscr8 ) {
            hiba( "blt8fizrefejjelle called with physical screen param!" );
            return;
        }
        if( ps->sprite ) {
            hiba( "blt8fizrefejjelle called with sprite param!" );
            return;
        }
        if( fizxsize != ps->getxsize() || fizysize != ps->getysize() ) {
            hiba( "blt8fizrefejjelle: par size != fizikai size!" );
            return;
        }
    #endif
    for( int y = 0; y < fizysize; y++ ) {
        putbytes_l( 0, y, ps->sormuttomb[fizysize-y-1], fizxsize );
    }
} */

int pcxtopal(char* nev, unsigned char* pal) {
    // read palette
    FILE* h = qopen(nev, "rb");
    if (!h) {
        hiba("pcxtopal cannot open file!: ", nev);
        return 0;
    }
    long l = -769;
    if (qseek(h, l, SEEK_END) != 0) {
        hiba("Cannot return by 768 to palette in PCX!: ", nev);
        qclose(h);
        return 0;
    }
    char c;
    l = fread(&c, 1, 1, h);
    if (l != 1) {
        hiba("Cannot read from PCX!:", nev);
        qclose(h);
        return 0;
    }
    if (c != 0x0c) {
        hiba("Byte before palette in PCX not 0x0C!: ", nev);
        qclose(h);
        return 0;
    }
    if (fread(pal, 768, 1, h) != 1) {
        hiba("Cannot read from PCX!:", nev);
        qclose(h);
        return 0;
    }
    for (int i = 0; i < 768; i++) {
        pal[i] = (unsigned char)(pal[i] >> 2);
    }
    qclose(h);
    return 1;
}

int pcxtopal(char* nev, ddpal** ppddpal) {
    unsigned char pal[768];
    pcxtopal(nev, pal);
    if (strcmp(nev, "intro.pcx") == 0) {
        // slightly sloppy, called with this param from teljes.cpp
        pal[0] = pal[1] = pal[2] = 0;
    }
    *ppddpal = new ddpal(pal);
    return 1;
}
void mintavetel8(pic8* pdest, pic8* psour, int x1, int y1, int x2, int y2) {
// checks
#ifdef PIC8TEST
    if (!pdest || !psour) {
        hiba("!pdest || !psour in mintavetel8!");
    }
#endif

    if (x1 > x2) {
        int tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y1 > y2) {
        int tmp = y1;
        y1 = y2;
        y2 = tmp;
    }

#ifdef PIC8TEST
    if (x1 < 0 || y1 < 0 || x2 >= pdest->getxsize() || y2 >= pdest->getysize()) {
        hiba("x1 < 0 || y1 < 0 || x2 >= xsize || y2 >= ysize in pic8::mintavetel!");
    }
#endif

    int xsd = x2 - x1 + 1;
    int ysd = y2 - y1 + 1;
    int xss = psour->getxsize();
    int yss = psour->getysize();
    double s_per_d_y = (double)yss / ysd;
    double s_per_d_x = (double)xss / xsd;
    for (int y = 0; y < ysd; y++) {
        double sy = (y + 0.5) * s_per_d_y;
        for (int x = 0; x < xsd; x++) {
            double sx = (x + 0.5) * s_per_d_x;
            unsigned char c = psour->gpixel(sx, sy);
            pdest->ppixel(x1 + x, y1 + y, c);
        }
    }
}

void mintavetel8(pic8* pdest, pic8* psour) {
    mintavetel8(pdest, psour, 0, 0, pdest->getxsize() - 1, pdest->getysize() - 1);
}

// only vertical and horizontal lines for now
void pic8::line(int x1, int y1, int x2, int y2, unsigned char szin) {
    // order
    if (x2 < x1) {
        int tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y2 < y1) {
        int tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    if (x1 == x2) {
        // vertical
        for (int y = y1; y <= y2; y++) {
            ppixel(x1, y, szin);
        }
        return;
    }
    if (y1 == y2) {
        // horizontal
        for (int x = x1; x <= x2; x++) {
            ppixel(x, y1, szin);
        }
        return;
    }
    hiba("pic8::line can only handle vertical and horizontal lines for now!");
}

void pic8::keszitbelsot(int x1, int y1, int x2, int y2, pic8* ppic) {
    xsize = x2 - x1 + 1;
    ysize = y2 - y1 + 1;
#ifdef TEST
    if (x1 < 0 || x2 >= 640 || y1 < 0 || y2 >= 480) {
        hiba("pic8::keszitbelsot!");
    }
#endif
    for (int y = 0; y < ysize; y++) {
        sormuttomb[y] = ppic->getptr(y + y1) + x1;
    }
}
