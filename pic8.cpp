#include "ALL.H"

typedef unsigned char* puchar;

// Ha igaz, akkor lefoglalt kepek minden masodik sora paratlan cimen
// kezdodik. LGR file beolvasas es betolthattereket alatt aktiv:
int Paratlankepsorok = 0;

int pic8::allocate(long w, long h) {
    // nem szabad hasznalni size-eket, hanem sizep-eket!
    if (rows || szegmuttomb) {
        hiba("pic8 already allocated!");
        return 0;
    }
    width = short(w);
    height = short(h);
    // width-ot 4-el oszthatova valtoztatjuk:
    if (Paratlankepsorok) {
        w++;
    }
    if (w % 4) {
        w += 4 - w % 4;
    }
    if (w <= 0 || h <= 0) {
        hiba("pic8 invalid width/height!");
        return 0;
    }
    long egyszegben = 0x0ff00l / long(w); // Nehogy szegmens merete
                                          // megkozelitse 65536-ot
    // szegnum meg inkrementalva lesz a vegen, ha plussz is van!:
    szegnum = unsigned(h) / egyszegben;
    long pluszszegbensorok = h - szegnum * egyszegben;
    int vanplussz = pluszszegbensorok != 0;
    rows = new puchar[unsigned(h)];
    if (vanplussz) {
        szegmuttomb = new puchar[unsigned(szegnum + 1)];
    } else {
        szegmuttomb = new puchar[unsigned(szegnum)];
    }
    if (!rows || !szegmuttomb) {
        uzenet("pic8::alloc memory!");
        return 0;
    }
    for (int i = 0; i < h; i++) {
        rows[i] = NULL;
    }
    for (int i = 0; i < szegnum; i++) {
        szegmuttomb[i] = NULL;
    }
    if (vanplussz) {
        szegmuttomb[unsigned(szegnum)] = NULL;
    }
    for (int i = 0; i < szegnum; i++) {
        szegmuttomb[i] = new unsigned char[unsigned(egyszegben * w)];
        if (!szegmuttomb[i]) {
            uzenet("pic8::alloc memory!");
            return 0;
        }
        for (int j = 0; j < egyszegben; j++) {
            rows[unsigned(i * egyszegben + j)] = &szegmuttomb[i][unsigned(j * w)];
        }
    }
    if (vanplussz) {
        szegmuttomb[unsigned(szegnum)] = new unsigned char[unsigned(pluszszegbensorok * w)];
        if (!szegmuttomb[unsigned(szegnum)]) {
            uzenet("pic8::alloc memory!");
            return 0;
        }
        for (int j = 0; j < pluszszegbensorok; j++) {
            rows[unsigned(szegnum * egyszegben + j)] =
                &szegmuttomb[unsigned(szegnum)][unsigned(j * w)];
        }
    }

    if (vanplussz) {
        szegnum++;
    }

    for (int y = 0; y < h; y++) {
        memset(rows[y], 0, unsigned(w)); // nullazunk
        if (Paratlankepsorok && (y % 2)) {
            rows[y]++;
        }
    }

    return 1;
}

pic8::~pic8(void) {
    if (nemdestrukt) {
        return;
    }
    if (rows) {
        delete rows;
    }
    if (szegmuttomb) {
        for (int i = 0; i < szegnum; i++) {
            delete szegmuttomb[i];
        }
    }
    delete szegmuttomb;
    if (transparency_data) {
        delete transparency_data;
    }
}

pic8::pic8(int w, int h) {
    success = 0;
    nemdestrukt = 0;
    rows = NULL;
    szegmuttomb = NULL;
    fizkep = 0;
    transparency_data = NULL;
    transparency_data_length = NULL;
    success = allocate(w, h);
}

// Mar meglevo tombre epiti ra pic8-at, destruktor nem torli:
pic8::pic8(int xsizep, int ysizep, unsigned char** ppc) {
    success = 0;
    nemdestrukt = 1;
    rows = NULL;
    szegmuttomb = NULL;
    fizkep = 0;
    transparency_data = NULL;
    transparency_data_length = NULL;
    if (xsizep == -12333 && ysizep == -12334) {
        hiba("fizkep nemdestruktos pic8 konstruktorban!");
        return;
    }
    width = short(xsizep);
    height = short(ysizep);
    if (!ppc) {
        hiba("!ppc nemdestruktos pic8 konstruktorban!");
    }
    rows = ppc;
    success = 1;
}

pic8::pic8(const char* filename, FILE* h) {
    nemdestrukt = 0;
    success = 0;
    rows = NULL;
    szegmuttomb = NULL;
    fizkep = 0;
    transparency_data = NULL;
    transparency_data_length = NULL;
    // Kiterjesztes megkeresese:
    int i = strlen(filename) - 1;
    while (i >= 0) {
        if (filename[i] == '.') {
            if (strcmpi(filename + i, ".spr") == 0) {
                spr_open(filename, h);
                return;
            }
            if (strcmpi(filename + i, ".pcx") == 0) {
                pcx_open(filename, h);
                return;
            }
            hiba("pic8 unknown file extension: ", filename);
            return;
        }
        i--;
    }
    hiba("pic8 could not find file extension: ", filename);
}

int pic8::save(const char* filename, unsigned char* pal, FILE* h) {
    if (!success) {
        hiba("pic8::save-ben nem success!");
        return 0;
    }
    if (fizkep) {
        hiba("Fizikai kepernyore save hivasa!");
        return 0;
    }
    // Kiterjesztes megkeresese:
    int i = 0;
    while (filename[i]) {
        if (filename[i] == '.') {
            if (strcmpi(filename + i, ".spr") == 0) {
                return spr_save(filename, h);
            }
            if (strcmpi(filename + i, ".pcx") == 0) {
                return pcx_save(filename, pal);
            }
            // if( strcmpi( filename+i, ".mix" ) == 0 ) {
            //   return mixsave( filename );
            // }
            hiba("pic8::save unknown file extension: ", filename);
            return 0;
        }
        i++;
    }
    hiba("pic8::save could not find file extension: ", filename);
    return 0;
}

int pic8::get_width(void) { return width; }

int pic8::get_height(void) { return height; }

void pic8::ppixel(int x, int y, unsigned char index) {
#ifdef PIC8TEST
    if (!success) {
        hiba("pic8::ppixelben success = 0!");
        return;
    }
#endif
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }

    if (!fizkep) {
        rows[y][x] = index;
        return;
    }
    // Fizikai:
    hiba("ppixel fizikaira!");
}

unsigned char pic8::gpixel(int x, int y) {
#ifdef PIC8TEST
    if (!success) {
        hiba("pic8::ppixelben success = 0!");
        return 0;
    }
#endif
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return 0;
    }

    if (!fizkep) {
        return rows[y][x];
    } else {
        // Fizikai:
        hiba("gpixel fizikaira!");
        // return (unsigned char)gpixel_l( x, y );
        return 0;
    }
}

#ifdef TEST

// Egyebkent inline:

unsigned char* pic8::get_row(int y) {
#ifdef PIC8TEST
    if (!success) {
        hiba("pic8::get_row success = 0!");
        return 0;
    }
    if (y < 0 || y >= height) {
        hiba("pic8::get_row y out of bounds!");
        return 0;
    }
#endif
    if (!fizkep) {
        return rows[y];
    } else {
        // Fizikai:
        hiba("pic8::get_row meghivasa fizikai kepernyore!");
        return NULL;
    }
}

#endif

void pic8::fill_box(unsigned char index) {
    fill_box(0, 0, get_width() - 1, get_height() - 1, index);
}

void pic8::fill_box(int x1, int y1, int x2, int y2, unsigned char index) {
    if (fizkep) {
        hiba("pic8::fill_box meghivasa fizikai kepernyore!");
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
    if (x2 >= width) {
        x2 = width - 1;
    }
    if (y2 >= height) {
        y2 = height - 1;
    }
    int length = x2 - x1 + 1;
    for (int y = y1; y <= y2; y++) {
        memset(rows[y] + x1, index, length);
    }
}

void pic8::horizontal_line(int x, int y, int size, unsigned char* lookup) {
#ifdef PIC8TEST
    if (x < 0 || y < 0 || x + size - 1 >= width || y >= height) {
        hiba("pic8::horizontal_line x/y out of range!");
    }
#endif
    unsigned char* row = get_row(y);
    for (int i = 0; i < size; i++) {
        row[x + i] = lookup[row[x + i]];
    }
    // memset( &row[x], szin, size );
}

void pic8::vertical_line(int x, int y, int size, unsigned char* lookup) {
#ifdef PIC8TEST
    if (x < 0 || y < 0 || x >= width || y + size - 1 >= height) {
        hiba("pic8::vertical_line x/y out of range!");
    }
#endif
    for (int i = 0; i < size; i++) {
        unsigned char* row = get_row(y + i);
        row[x] = lookup[row[x]];
    }

    // for( int i = 0; i < size; i++ )
    //	ppixel( x, y+i, szin );
}

// SPRITE:

/*static void dummy( long l ) {
    l++;
} */

void pic8::spr_open(const char* filename, FILE* h) {
    int h_provided = 1;
    if (!h) {
        h_provided = 0;
        h = qopen(filename, "rb");
        if (!h) {
            hiba("Failed to open sprite file!: ", filename);
            return;
        }
    }
    unsigned char c = 0;
    if (fread(&c, 1, 1, h) != 1) {
        hiba("Error reading sprite file: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }
    if (c != 0x2d) {
        hiba("Sprite file header invalid:", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }

    unsigned short sprite_width = -1, sprite_height = -1;
    if (fread(&sprite_width, 2, 1, h) != 1 || fread(&sprite_height, 2, 1, h) != 1) {
        hiba("Error reading sprite file: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }
    width = sprite_width;
    height = sprite_height;
    if (width < 1 || height < 1) {
        hiba("Sprite file width/height invalid: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }
    if (!allocate(width, height)) {
        return;
    }

    for (int y = 0; y < height; y++) {
        // Egy sor beolvasasa:
        if (fread(rows[y], width, 1, h) != 1) {
            hiba("Error reading sprite file: ", filename);
            if (!h_provided) {
                qclose(h);
            }
            return;
        }
    }

    // SPRITE reszek beolvasasa:
    char tmp[10] = "";
    if (fread(tmp, 7, 1, h) != 1) {
        hiba("Error reading sprite file: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }
    if (strcmp(tmp, "SPRITE") != 0) {
        hiba("Sprite file data header invalid: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }

    transparency_data_length = -1;
    if (fread(&transparency_data_length, 2, 1, h) != 1) {
        hiba("Error reading sprite file: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }
    if (transparency_data_length < 1) {
        hiba("Sprite file transparency data length invalid: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }

    // long l = farcoreleft();
    // dummy( l );
    transparency_data = new unsigned char[transparency_data_length];
    if (!transparency_data) {
        hiba("Could not allocate memory for sprite file: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }
    if (fread(transparency_data, transparency_data_length, 1, h) != 1) {
        hiba("Error reading sprite file transparency data: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }
    if (!h_provided) {
        qclose(h);
    }
    success = 1;
}

int pic8::spr_save(const char* filename, FILE* h) {
    int h_provided = 1;
    if (!h) {
        h_provided = 0;
        h = fopen(filename, "wb");
        if (!h) {
            hiba("pic8::spr_save failed to open file: ", filename);
            return 0;
        }
    }
    unsigned char c = 0x2d;
    if (fwrite(&c, 1, 1, h) != 1 || fwrite(&width, 2, 1, h) != 1 || fwrite(&height, 2, 1, h) != 1) {
        hiba("pic8::spr_save failed to write to file: ", filename);
        if (!h_provided) {
            fclose(h);
        }
        return 0;
    }

    for (int y = 0; y < height; y++) {
        // Egy sor kiirasa:
        if (fwrite(rows[y], width, 1, h) != 1) {
            hiba("pic8::spr_save failed to write to file: ", filename);
            if (!h_provided) {
                fclose(h);
            }
            return 0;
        }
    }

    // Sprite resz kiirasa:
    if (fwrite("SPRITE", 7, 1, h) != 1 || fwrite(&transparency_data_length, 2, 1, h) != 1 ||
        fwrite(transparency_data, transparency_data_length, 1, h) != 1) {
        hiba("pic8::spr_save failed to write to file: ", filename);
        if (!h_provided) {
            fclose(h);
        }
        return 0;
    }
    if (!h_provided) {
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

void pic8::pcx_open(const char* filename, FILE* h) {
    // Gany:
    success = 1;

    int h_not_provided = 0;
    if (!h) {
        h_not_provided = 1;
        h = qopen(filename, "rb");
        if (!h) {
            hiba("Failed to open PCX file!: ", filename);
        }
    }
    pcxdescriptor desc;
    if (fread(&desc, sizeof(desc), 1, h) != 1) {
        hiba("Failed to read PCX file: ", filename);
    }
    if ((desc.VersionNum != 5) || (desc.ManufactId != 10) || (desc.EncodingTech != 1) ||
        (desc.BitsPerPlane != 8) || (desc.NumberOfBitPlanes != 1)) {
        hiba("PCX file header invalid or not supported: ", filename);
    }
    // if( !allocate( desc.Xmax-desc.Xmin+1, desc.Ymax-desc.Ymin+1 ) ) {
    if (!allocate(desc.Xmax - desc.Xmin + 1, desc.Ymax - desc.Ymin + 1)) {
        hiba("pcx !allocate!");
    }
    for (int y = 0; y < height; y++) {
        // Egy sor beolvasasa:
        short nnn = 0, ccc, iii;

        do {
            unsigned char index;
            long l = fread(&index, 1, 1, h);
            ccc = index;
            if (l != 1) {
                hiba("Failed to read PCX file: ", filename);
            }

            if ((ccc & 0xc0) == 0xc0) {
                iii = ccc & (short)0x3f;
                l = fread(&index, 1, 1, h);
                ccc = index;
                if (l != 1) {
                    hiba("Failed to read PCX file: ", filename);
                }

                while (iii--) {
                    if (nnn < width) {
                        ppixel(nnn, y, (unsigned char)ccc);
                    }
                    nnn++;
                }
            } else {
                if (nnn < width) {
                    ppixel(nnn, y, (unsigned char)ccc);
                }
                nnn++;
            }
        } while (nnn < desc.BytesPerScanLine);
    }

    // success az elejen mar 1-re van allitva!
    if (h_not_provided) {
        qclose(h);
    }
}

static int pcx_count_repeats(pic8* ppic, int x, int y, int width) {
    unsigned char index = ppic->gpixel(x, y);
    x++;
    int repeats = 1;
    while (x < width && ppic->gpixel(x, y) == index) {
        x++;
        repeats++;
    }
    return repeats;
}

int pic8::pcx_save(const char* filename, unsigned char* pal) {
    FILE* h = fopen(filename, "wb");
    if (!h) {
        hiba("pcx_save failed to open file: ", filename);
        return 0;
    }
    pcxdescriptor desc;
    desc.ManufactId = 10;
    desc.VersionNum = 5;
    desc.EncodingTech = 1;
    desc.BitsPerPlane = 8;
    desc.Xmin = desc.Ymin = 0;
    desc.Xmax = (short)(width - 1);
    desc.Ymax = (short)(height - 1);
    desc.HorRes = 10;
    desc.VertRes = 10;
    desc.Reserved = 0;
    desc.NumberOfBitPlanes = 1;
    desc.BytesPerScanLine = (unsigned short)width;
    desc.PaletteInf = 1;
    if (fwrite(&desc, sizeof(desc), 1, h) != 1) {
        hiba("pcx_save failed to write header to file: ", filename);
        fclose(h);
        return 0;
    }
    for (int y = 0; y < height; y++) {
        int x = 0;
        while (x < width) {
            int i = pcx_count_repeats(this, x, y, width);
            if (i > 1) {
                if (i > 63) {
                    i = 63;
                }
                unsigned char controll = (unsigned char)(i + 192);
                if (fwrite(&controll, 1, 1, h) != 1) {
                    hiba("pcx_save failed to write to file: ", filename);
                    fclose(h);
                    return 0;
                }
                unsigned char index = gpixel(x, y);
                if (fwrite(&index, 1, 1, h) != 1) {
                    hiba("pcx_save failed to write to file: ", filename);
                    fclose(h);
                    return 0;
                }
                x += i;
            } else {
                unsigned char index = gpixel(x, y);
                if (index < 64) {
                    if (fwrite(&index, 1, 1, h) != 1) {
                        hiba("pcx_save failed to write to file: ", filename);
                        fclose(h);
                        return 0;
                    }
                } else {
                    unsigned char controll = 193;
                    if (fwrite(&controll, 1, 1, h) != 1) {
                        hiba("pcx_save failed to write to file: ", filename);
                        fclose(h);
                        return 0;
                    }
                    index = gpixel(x, y);
                    if (fwrite(&index, 1, 1, h) != 1) {
                        hiba("pcx_save failed to write to file: ", filename);
                        fclose(h);
                        return 0;
                    }
                }
                x++;
            }
        }
    }
    unsigned char palette_header = 0x0c;
    if (fwrite(&palette_header, 1, 1, h) != 1) {
        hiba("pcx_save failed to write to file: ", filename);
        fclose(h);
        return 0;
    }
    if (pal) {
        // Van paletta!:
        for (int i = 0; i < 768; i++) {
            unsigned char c = (unsigned char)(pal[i] << 2);
            if (fwrite(&c, 1, 1, h) != 1) {
                hiba("pcx_save failed to write to file: ", filename);
                fclose(h);
                return 0;
            }
        }
    } else {
        // Nincs paletta megadva!
        for (int i = 0; i < 256; i++) {
            unsigned char c = (unsigned char)i;
            for (int j = 0; j < 3; j++) {
                if (fwrite(&c, 1, 1, h) != 1) {
                    hiba("pcx_save failed to write to file: ", filename);
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
void blit8(pic8* dest, pic8* source, int x, int y, int x1, int y1, int x2, int y2) {
#ifdef PIC8TEST
    if (y2 == -10000 && (x1 != -10000 || y1 != -10000 || x2 != -10000)) {
        hiba("blit8 y2 == PIC8_BLIT_ALL, but x1/y1/x2 are not!");
        return;
    }
    if (!dest || !source) {
        hiba("blit8 missing dest or source!");
        return;
    }
#endif

    // if( Debugblt ) idejott
    //	hiba( "Itt van 17!" );

    if (dest->transparency_data) {
        hiba("blit8 destination has transparency_data!");
        return;
    }
    if (x1 == -10000) {
        x1 = y1 = 0;
        x2 = source->width - 1;
        y2 = source->height - 1;
    } else {
        // Sorrend beallitasa:
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
    // Levagdosasa reszeknek x iranyban:
    // Eloszor source kepnek megfeleloen:
    if (x1 < 0) {
        x += -x1;
        x1 = 0;
    }
    if (x2 >= source->width) {
        x -= x2 - (source->width - 1);
        x2 = source->width - 1;
    }
    // Most destinacionak megfeleloen:
    if (x < 0) {
        x1 += -x;
        x = 0;
    }
    if (x + (x2 - x1) >= dest->width) {
        x2 -= x + (x2 - x1) - (dest->width - 1);
    }
    if (x1 > x2) {
        return;
    }

    // Levagdosasa reszeknek y iranyban:
    // Eloszor source kepnek megfeleloen:
    if (y1 < 0) {
        y += -y1;
        y1 = 0;
    }
    if (y2 >= source->height) {
        y -= y2 - (source->height - 1);
        y2 = source->height - 1;
    }
    // Most destinacionak megfeleloen:
    if (y < 0) {
        y1 += -y;
        y = 0;
    }
    if (y + (y2 - y1) >= dest->height) {
        y2 -= y + (y2 - y1) - (dest->height - 1);
    }
    if (y1 > y2) {
        return;
    }

    if (source->transparency_data) {
        // sprite lejatszas:
        // Itt vegig kell futni teljes sprite kepet es kozben vagni!
        unsigned buf = 0;
        unsigned char* buffer = source->transparency_data;
        int desty = y - y1;
        for (int sy = 0; sy < source->height; sy++) {
            int y_in_range = 1;
            if (sy < y1 || sy > y2) {
                y_in_range = 0;
            }
            int sx = 0;
            while (sx < source->width) {
                switch (buffer[buf++]) {
                case 'K':
                    if (y_in_range) {
                        if (sx < x1 || sx + buffer[buf] - 1 > x2) {
                            if (!(sx > x2 || sx + buffer[buf] - 1 < x1)) {
                                // Kilog, de van bent resze:
                                int xstart = sx;
                                int xend = sx + buffer[buf] - 1;
                                if (xstart < x1) {
                                    xstart = x1;
                                }
                                if (xend > x2) {
                                    xend = x2;
                                }
                                if (dest->fizkep) {
                                    hiba("t4ruh5t");
                                }
                                // putline8_l( x+xkezd-x1, desty,
                                //&source->rows[sy][xkezd],
                                // xveg-xkezd+1 );
                                else {
                                    memcpy(&dest->rows[desty][x + xstart - x1],
                                           &source->rows[sy][xstart], xend - xstart + 1);
                                }
                            }
                        } else {
                            // Nem log ki:
                            if (dest->fizkep) {
                                hiba("68464");
                            }
                            // putline8_l( x+sx-x1, desty,
                            //&source->rows[sy][sx],
                            // buffer[buf] );
                            else {
                                memcpy(&dest->rows[desty][x + sx - x1], &source->rows[sy][sx],
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

                    hiba("Sprite blit8 unknown data block!");
                    return;
                }
            }
            desty++;
        }

        return;
    }

    // Most mar tuti jok koordok!
    if (dest->fizkep) {
        // Fizikai kepernyore iras:
        hiba("0895t");
    }
    if (source->fizkep) {
        // Fizikai kepernyorol olvasas:
        hiba("8795t");
    }
    // Memoria-memoria atvitel!
    int length = x2 - x1 + 1;
    int dfy = y;

    // if( Debugblt )	idejott
    //	hiba( "Itt van 18!" );

    for (int fy = y1; fy <= y2; fy++) {
        memcpy(&dest->rows[dfy++][x], &source->rows[fy][x1], length);
    }
    // if( Debugblt )
    //	hiba( "Itt van 19!" ); idejott
}

/*void blt8fizrefejjelle( pic8* ps ) {
    int fizxsize = Pscr8->get_width();
    int fizysize = Pscr8->get_height();
    #ifdef PIC8TEST
        if( ps == Pscr8 ) {
            hiba( "blt8fizrefejjelle hivasa, fizikai kepernyo parral!" );
            return;
        }
        if( ps->transparency_data ) {
            hiba( "blt8fizrefejjelle hivasa, sprite parral!" );
            return;
        }
        if( fizxsize != ps->get_width() || fizysize != ps->get_height() ) {
            hiba( "blt8fizrefejjelle: par size != fizikai size!" );
            return;
        }
    #endif
    for( int y = 0; y < fizysize; y++ ) {
        putbytes_l( 0, y, ps->rows[fizysize-y-1], fizxsize );
    }
} */

int get_pcx_pal(const char* filename, unsigned char* pal) {
    // Paletta beolvasasa:
    FILE* h = qopen(filename, "rb");
    if (!h) {
        hiba("get_pcx_pal failed to open file: ", filename);
        return 0;
    }
    long l = -769;
    if (qseek(h, l, SEEK_END) != 0) {
        hiba("get_pcx_pal failed to seek to palette data: ", filename);
        qclose(h);
        return 0;
    }
    char palette_header;
    l = fread(&palette_header, 1, 1, h);
    if (l != 1) {
        hiba("get_pcx_pal failed to read file: ", filename);
        qclose(h);
        return 0;
    }
    if (palette_header != 0x0c) {
        hiba("get_pcx_pal invalid palette data header:", filename);
        qclose(h);
        return 0;
    }
    if (fread(pal, 768, 1, h) != 1) {
        hiba("get_pcx_pal failed to read file: ", filename);
        qclose(h);
        return 0;
    }
    for (int i = 0; i < 768; i++) {
        pal[i] = (unsigned char)(pal[i] >> 2);
    }
    qclose(h);
    return 1;
}

int get_pcx_pal(const char* filename, ddpal** sdl_pal) {
    unsigned char pal[768];
    get_pcx_pal(filename, pal);
    if (strcmp(filename, "intro.pcx") == 0) {
        // Egy kicsit gany, teljes.cpp-bol hivodik ilyen parameterrel.
        pal[0] = pal[1] = pal[2] = 0;
    }
    *sdl_pal = new ddpal(pal);
    return 1;
}
void blit_scale8(pic8* dest, pic8* source, int x1, int y1, int x2, int y2) {
// Ellenorzesek:
#ifdef PIC8TEST
    if (!dest || !source) {
        hiba("blit_scale8 !dest || !source!");
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
    if (x1 < 0 || y1 < 0 || x2 >= dest->get_width() || y2 >= dest->get_height()) {
        hiba("pic8::blit_scale x1 < 0 || y1 < 0 || x2 >= width || y2 >= height!");
    }
#endif

    int xsd = x2 - x1 + 1;
    int ysd = y2 - y1 + 1;
    int xss = source->get_width();
    int yss = source->get_height();
    double s_per_d_y = (double)yss / ysd;
    double s_per_d_x = (double)xss / xsd;
    for (int y = 0; y < ysd; y++) {
        double sy = (y + 0.5) * s_per_d_y;
        for (int x = 0; x < xsd; x++) {
            double sx = (x + 0.5) * s_per_d_x;
            unsigned char c = source->gpixel(sx, sy);
            dest->ppixel(x1 + x, y1 + y, c);
        }
    }
}

void blit_scale8(pic8* dest, pic8* source) {
    blit_scale8(dest, source, 0, 0, dest->get_width() - 1, dest->get_height() - 1);
}

// Egyenlore csak vizszintes es fuggoleges vonalak:
void pic8::line(int x1, int y1, int x2, int y2, unsigned char index) {
    // Sorrend:
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
        // Fuggoleges:
        for (int y = y1; y <= y2; y++) {
            ppixel(x1, y, index);
        }
        return;
    }
    if (y1 == y2) {
        // Vizszintes:
        for (int x = x1; x <= x2; x++) {
            ppixel(x, y1, index);
        }
        return;
    }
    hiba("pic8::line diagonal lines not implemented!");
}

void pic8::subview(int x1, int y1, int x2, int y2, pic8* source) {
    width = x2 - x1 + 1;
    height = y2 - y1 + 1;
#ifdef TEST
    if (x1 < 0 || x2 >= SCREEN_WIDTH || y1 < 0 || y2 >= SCREEN_HEIGHT) {
        hiba("pic8::subview!");
    }
#endif
    for (int y = 0; y < height; y++) {
        rows[y] = source->get_row(y + y1) + x1;
    }
}
