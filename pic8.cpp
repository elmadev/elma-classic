#include "M_PIC.H"
#include "main.h"
#include "pic8.h"
#include "platform_impl.h"
#include "platform_utils.h"
#include "qopen.h"
#include <algorithm>
#include <cstring>

void pic8::allocate(int w, int h) {
    if (rows || pixels) {
        internal_error("pic8 already allocated!");
        return;
    }
    width = short(w);
    height = short(h);
    if (w % 4) {
        w += 4 - w % 4;
    }
    if (w <= 0 || h <= 0) {
        internal_error("pic8 invalid width/height!");
        return;
    }
    // Allocate pixels and rows
    pixels = new unsigned char[w * h];
    rows = new unsigned char*[(unsigned int)(h)];
    if (!rows || !pixels) {
        external_error("pic8::alloc memory!");
        return;
    }
    memset(pixels, 0, sizeof(unsigned char) * w * h);
    // Map the pixel array to the array of rows
    for (int i = 0; i < h; i++) {
        rows[i] = pixels + i * w;
    }
}

pic8::~pic8() {
    if (rows) {
        delete rows;
    }
    if (pixels) {
        delete pixels;
    }
    if (transparency_data) {
        delete transparency_data;
    }
}

// Initialize a blank picture
pic8::pic8(int w, int h) {
    rows = nullptr;
    pixels = nullptr;
    transparency_data = nullptr;
    transparency_data_length = 0;
    allocate(w, h);
}

// Initialize a picture from filename or file
pic8::pic8(const char* filename, FILE* h) {
    rows = nullptr;
    pixels = nullptr;
    transparency_data = nullptr;
    transparency_data_length = 0;
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
            internal_error("pic8 unknown file extension: ", filename);
            return;
        }
        i--;
    }
    internal_error("pic8 could not find file extension: ", filename);
}

pic8* pic8::scale(pic8* src, double scale) {
    if (scale == 1.0) {
        return src;
    }

    const int scaled_width = std::max((int)(src->get_width() * scale), 1);
    const int scaled_height = std::max((int)(src->get_height() * scale), 1);

    pic8* scaled = new pic8(scaled_width, scaled_height);
    blit_scale8(scaled, src);
    delete src;
    return scaled;
}

bool pic8::save(const char* filename, unsigned char* pal, FILE* h) {
    int i = 0;
    while (filename[i]) {
        if (filename[i] == '.') {
            if (strcmpi(filename + i, ".spr") == 0) {
                return spr_save(filename, h);
            }
            if (strcmpi(filename + i, ".pcx") == 0) {
                return pcx_save(filename, pal);
            }
            internal_error("pic8::save unknown file extension: ", filename);
            return false;
        }
        i++;
    }
    internal_error("pic8::save could not find file extension: ", filename);
    return false;
}

void pic8::ppixel(int x, int y, unsigned char index) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }
    rows[y][x] = index;
    return;
}

unsigned char pic8::gpixel(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return 0;
    }
    return rows[y][x];
}

#ifdef DEBUG
unsigned char* pic8::get_row(int y) {
    if (y < 0 || y >= height) {
        internal_error("pic8::get_row y out of bounds!");
        return 0;
    }
    return rows[y];
}
#endif

void pic8::fill_box(unsigned char index) {
    fill_box(0, 0, get_width() - 1, get_height() - 1, index);
}

void pic8::fill_box(int x1, int y1, int x2, int y2, unsigned char index) {
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

// Read a .spr file
// char Header[1] == 0x2D
// short width
// short height
// unsigned char raw_palette_data[xsize*ysize]
// char spriteheader[7] == "SPRITE"
// short transparency_data_length
// unsigned char transparency_data[transparency_data_length] (sprite transparency data)
// Transparency data:
//       ['K', length] -> the next length pixels are solid
//       ['N', length] -> the next length pixels are transparent
void pic8::spr_open(const char* filename, FILE* h) {
    bool h_provided = true;
    if (!h) {
        h_provided = false;
        h = qopen(filename, "rb");
        if (!h) {
            internal_error("Failed to open sprite file!: ", filename);
            return;
        }
    }
    // Header
    unsigned char c = 0;
    if (fread(&c, 1, 1, h) != 1) {
        internal_error("Error reading sprite file: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }
    if (c != 0x2d) {
        internal_error("Sprite file header invalid:", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }
    // Pixel data
    unsigned short sprite_width = -1;
    unsigned short sprite_height = -1;
    if (fread(&sprite_width, 2, 1, h) != 1 || fread(&sprite_height, 2, 1, h) != 1) {
        internal_error("Error reading sprite file: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }
    width = sprite_width;
    height = sprite_height;
    if (width < 1 || height < 1) {
        internal_error("Sprite file width/height invalid: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }
    allocate(width, height);
    for (int y = 0; y < height; y++) {
        if (fread(rows[y], width, 1, h) != 1) {
            internal_error("Error reading sprite file: ", filename);
            if (!h_provided) {
                qclose(h);
            }
            return;
        }
    }
    // Transparency data header
    char tmp[10] = "";
    if (fread(tmp, 7, 1, h) != 1) {
        internal_error("Error reading sprite file: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }
    if (strcmp(tmp, "SPRITE") != 0) {
        internal_error("Sprite file data header invalid: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }
    // Transparency data
    transparency_data_length = -1;
    if (fread(&transparency_data_length, 2, 1, h) != 1) {
        internal_error("Error reading sprite file: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }
    if (transparency_data_length < 1) {
        internal_error("Sprite file transparency data length invalid: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }
    transparency_data = new unsigned char[transparency_data_length];
    if (!transparency_data) {
        internal_error("Could not allocate memory for sprite file: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }
    if (fread(transparency_data, transparency_data_length, 1, h) != 1) {
        internal_error("Error reading sprite file transparency data: ", filename);
        if (!h_provided) {
            qclose(h);
        }
        return;
    }
    if (!h_provided) {
        qclose(h);
    }
}

bool pic8::spr_save(const char* filename, FILE* h) {
    bool h_provided = true;
    if (!h) {
        h_provided = false;
        h = fopen(filename, "wb");
        if (!h) {
            internal_error("pic8::spr_save failed to open file: ", filename);
            return false;
        }
    }
    unsigned char c = 0x2d;
    if (fwrite(&c, 1, 1, h) != 1 || fwrite(&width, 2, 1, h) != 1 || fwrite(&height, 2, 1, h) != 1) {
        internal_error("pic8::spr_save failed to write to file: ", filename);
        if (!h_provided) {
            fclose(h);
        }
        return false;
    }
    for (int y = 0; y < height; y++) {
        if (fwrite(rows[y], width, 1, h) != 1) {
            internal_error("pic8::spr_save failed to write to file: ", filename);
            if (!h_provided) {
                fclose(h);
            }
            return false;
        }
    }
    if (fwrite("SPRITE", 7, 1, h) != 1 || fwrite(&transparency_data_length, 2, 1, h) != 1 ||
        fwrite(transparency_data, transparency_data_length, 1, h) != 1) {
        internal_error("pic8::spr_save failed to write to file: ", filename);
        if (!h_provided) {
            fclose(h);
        }
        return false;
    }
    if (!h_provided) {
        fclose(h);
    }
    return true;
}

struct pcxdescriptor {
    unsigned char ManufactId, VersionNum, EncodingTech, BitsPerPlane;
    short Xmin, Ymin, Xmax, Ymax;
    short HorRes, VertRes;
    unsigned char ColorMap[48], Reserved, NumberOfBitPlanes;
    short BytesPerScanLine, PaletteInf;
    unsigned char Padding[127 - 70 + 1];
};

void pic8::pcx_open(const char* filename, FILE* h) {
    bool h_not_provided = false;
    if (!h) {
        h_not_provided = true;
        h = qopen(filename, "rb");
        if (!h) {
            internal_error("Failed to open PCX file!: ", filename);
        }
    }
    // Header
    pcxdescriptor desc;
    if (fread(&desc, sizeof(desc), 1, h) != 1) {
        internal_error("Failed to read PCX file: ", filename);
    }
    if ((desc.VersionNum != 5) || (desc.ManufactId != 10) || (desc.EncodingTech != 1) ||
        (desc.BitsPerPlane != 8) || (desc.NumberOfBitPlanes != 1)) {
        internal_error("PCX file header invalid or not supported: ", filename);
    }
    allocate(desc.Xmax - desc.Xmin + 1, desc.Ymax - desc.Ymin + 1);
    // Pixel data
    for (int y = 0; y < height; y++) {
        short nnn = 0, ccc, iii;

        do {
            unsigned char index;
            int l = fread(&index, 1, 1, h);
            ccc = index;
            if (l != 1) {
                internal_error("Failed to read PCX file: ", filename);
            }

            if ((ccc & 0xc0) == 0xc0) {
                iii = ccc & (short)0x3f;
                l = fread(&index, 1, 1, h);
                ccc = index;
                if (l != 1) {
                    internal_error("Failed to read PCX file: ", filename);
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
    if (h_not_provided) {
        qclose(h);
    }
}

// Get the number of times the same pixel palette ID repeats, stopping at end of scanline
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

// Strictly respects pcx convention: Only compresses palette IDs 0-63 instead of 0-191
// Does not enforce the width to be an even number however
bool pic8::pcx_save(const char* filename, unsigned char* pal) {
    FILE* h = fopen(filename, "wb");
    if (!h) {
        internal_error("pcx_save failed to open file: ", filename);
        return false;
    }
    // Header
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
        internal_error("pcx_save failed to write header to file: ", filename);
        fclose(h);
        return false;
    }
    // Pixel data
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
                    internal_error("pcx_save failed to write to file: ", filename);
                    fclose(h);
                    return false;
                }
                unsigned char index = gpixel(x, y);
                if (fwrite(&index, 1, 1, h) != 1) {
                    internal_error("pcx_save failed to write to file: ", filename);
                    fclose(h);
                    return false;
                }
                x += i;
            } else {
                unsigned char index = gpixel(x, y);
                if (index < 64) {
                    if (fwrite(&index, 1, 1, h) != 1) {
                        internal_error("pcx_save failed to write to file: ", filename);
                        fclose(h);
                        return false;
                    }
                } else {
                    unsigned char controll = 193;
                    if (fwrite(&controll, 1, 1, h) != 1) {
                        internal_error("pcx_save failed to write to file: ", filename);
                        fclose(h);
                        return false;
                    }
                    index = gpixel(x, y);
                    if (fwrite(&index, 1, 1, h) != 1) {
                        internal_error("pcx_save failed to write to file: ", filename);
                        fclose(h);
                        return false;
                    }
                }
                x++;
            }
        }
    }
    // Palette
    unsigned char palette_header = 0x0c;
    if (fwrite(&palette_header, 1, 1, h) != 1) {
        internal_error("pcx_save failed to write to file: ", filename);
        fclose(h);
        return false;
    }
    if (pal) {
        for (int i = 0; i < 768; i++) {
            if (fwrite(&pal[i], 1, 1, h) != 1) {
                internal_error("pcx_save failed to write to file: ", filename);
                fclose(h);
                return false;
            }
        }
    } else {
        // Use a dummy greyscale palette if none provided
        for (int i = 0; i < 256; i++) {
            unsigned char c = (unsigned char)i;
            for (int j = 0; j < 3; j++) {
                if (fwrite(&c, 1, 1, h) != 1) {
                    internal_error("pcx_save failed to write to file: ", filename);
                    fclose(h);
                    return false;
                }
            }
        }
    }
    fclose(h);
    return true;
}

constexpr short BMP_MAGIC = 0x4D42;
constexpr unsigned int BMP_HEADER_SIZE = 52;

// This is a combination of BITMAPFILEHEADER and BITMAPINFOHEADER.
// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader
// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
struct bmp_header {
    unsigned int size;
    unsigned int reserved;
    unsigned int offset;
    unsigned int dib_size;
    signed int width;
    signed int height;
    unsigned short planes;
    unsigned short bpp;
    unsigned int compression;
    unsigned int image_size;
    signed int x_pixels_per_meter;
    signed int y_pixels_per_meter;
    unsigned int num_colors;
    unsigned int num_important_colors;
};

static_assert(sizeof(bmp_header) == BMP_HEADER_SIZE);

pic8* pic8::from_bmp(const char* filename) {
    FILE* h = fopen(filename, "rb");
    if (!h) {
        return nullptr;
    }

    short magic;
    if (fread(&magic, 1, sizeof(magic), h) != 2 || magic != BMP_MAGIC) {
        return nullptr;
    }

    bmp_header header;
    if (fread(&header, 1, sizeof(header), h) != BMP_HEADER_SIZE) {
        return nullptr;
    }

    // Unsupported headers: BITMAPCOREHEADER, OS22XBITMAPHEADER, OS22XBITMAPHEADER
    if (header.dib_size == 12 || header.dib_size == 64 || header.dib_size == 16) {
        return nullptr;
    }

    // Only support:
    //   - 8 bits-per-pixel (indexed bitmap)
    //   - positive height (if height is negative pixels are stored top down)
    if (header.bpp != 8 || header.height < 1) {
        return nullptr;
    }

    int width = header.width;
    int height = header.height;

    pic8* pic = new pic8(width, height);

    // BMP rows are padded.
    int padded_width = width;
    if (width % 4) {
        padded_width += 4 - width % 4;
    }

    unsigned char* pixels = new unsigned char[padded_width * height];
    fseek(h, header.offset, SEEK_SET);
    if (fread(pixels, 1, padded_width * height, h) != padded_width * height) {
        return nullptr;
    }

    for (int h = 0; h < height; h++) {
        // BMP rows are bottom up.
        int src_row = height - 1 - h;
        memcpy(pic->get_row(h), &pixels[src_row * padded_width], width);
    }
    delete[] pixels;

    return pic;
}

// Paste source (x1, y1, x2, y2) into dest at (x, y)
void blit8(pic8* dest, pic8* source, int x, int y, int x1, int y1, int x2, int y2) {
#ifdef DEBUG
    if (!dest || !source) {
        internal_error("blit8 missing dest or source!");
        return;
    }
#endif
    if (dest->transparency_data) {
        internal_error("blit8 destination has transparency_data!");
        return;
    }
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
    // x - Make sure we don't grab pixels that are out of range of source picture
    if (x1 < 0) {
        x += -x1;
        x1 = 0;
    }
    if (x2 >= source->width) {
        x -= x2 - (source->width - 1);
        x2 = source->width - 1;
    }
    // x - Make sure we don't grab pixels that are out of range of destination picture
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
    // y - Make sure we don't grab pixels that are out of range of source picture
    if (y1 < 0) {
        y += -y1;
        y1 = 0;
    }
    if (y2 >= source->height) {
        y -= y2 - (source->height - 1);
        y2 = source->height - 1;
    }
    // y - Make sure we don't grab pixels that are out of range of destination picture
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
        // Skip transparent pixels of source image
        unsigned buf = 0;
        unsigned char* buffer = source->transparency_data;
        int desty = y - y1;
        for (int sy = 0; sy < source->height; sy++) {
            bool y_in_range = true;
            if (sy < y1 || sy > y2) {
                y_in_range = false;
            }
            int sx = 0;
            while (sx < source->width) {
                switch (buffer[buf++]) {
                case 'K':
                    if (y_in_range) {
                        if (sx < x1 || sx + buffer[buf] - 1 > x2) {
                            if (!(sx > x2 || sx + buffer[buf] - 1 < x1)) {
                                int xstart = sx;
                                int xend = sx + buffer[buf] - 1;
                                if (xstart < x1) {
                                    xstart = x1;
                                }
                                if (xend > x2) {
                                    xend = x2;
                                }
                                memcpy(&dest->rows[desty][x + xstart - x1],
                                       &source->rows[sy][xstart], xend - xstart + 1);
                            }
                        } else {
                            memcpy(&dest->rows[desty][x + sx - x1], &source->rows[sy][sx],
                                   buffer[buf]);
                        }
                    }
                    sx += buffer[buf++];
                    break;
                case 'N':
                    sx += buffer[buf++];
                    break;
                default:
                    internal_error("Sprite blit8 unknown data block!");
                    return;
                }
            }
            desty++;
        }
        return;
    }

    // No transparent pixels, just copy everything
    int length = x2 - x1 + 1;
    int dfy = y;
    for (int fy = y1; fy <= y2; fy++) {
        memcpy(&dest->rows[dfy++][x], &source->rows[fy][x1], length);
    }
}

// Paste source (x1, y1, x2, y2) into dest at (x, y)
void blit8(pic8* dest, pic8* source, int x, int y) {
#ifdef DEBUG
    if (!dest || !source) {
        internal_error("blit8 missing dest or source!");
        return;
    }
#endif
    int x1 = 0;
    int y1 = 0;
    int x2 = source->width - 1;
    int y2 = source->height - 1;
    blit8(dest, source, x, y, x1, y1, x2, y2);
}

bool get_pcx_pal(const char* filename, unsigned char* pal) {
    FILE* h = qopen(filename, "rb");
    if (!h) {
        internal_error("get_pcx_pal failed to open file: ", filename);
        return false;
    }
    int l = -769;
    if (qseek(h, l, SEEK_END) != 0) {
        internal_error("get_pcx_pal failed to seek to palette data: ", filename);
        qclose(h);
        return false;
    }
    char palette_header;
    l = fread(&palette_header, 1, 1, h);
    if (l != 1) {
        internal_error("get_pcx_pal failed to read file: ", filename);
        qclose(h);
        return false;
    }
    if (palette_header != 0x0c) {
        internal_error("get_pcx_pal invalid palette data header:", filename);
        qclose(h);
        return false;
    }
    if (fread(pal, 768, 1, h) != 1) {
        internal_error("get_pcx_pal failed to read file: ", filename);
        qclose(h);
        return false;
    }
    qclose(h);
    return true;
}

bool get_pcx_pal(const char* filename, palette** pal) {
    unsigned char pal_data[768];
    get_pcx_pal(filename, pal_data);
    *pal = new palette(pal_data);
    return true;
}

// Copy entire source into the (x1, y1, x2, y2) coordinates of dest
void blit_scale8(pic8* dest, pic8* source, int x1, int y1, int x2, int y2) {
#ifdef DEBUG
    if (!dest || !source) {
        internal_error("blit_scale8 !dest || !source!");
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
#ifdef DEBUG
    if (x1 < 0 || y1 < 0 || x2 >= dest->get_width() || y2 >= dest->get_height()) {
        internal_error("pic8::blit_scale x1 < 0 || y1 < 0 || x2 >= width || y2 >= height!");
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
            unsigned char c = source->gpixel((int)(sx), (int)(sy));
            dest->ppixel(x1 + x, y1 + y, c);
        }
    }
}

// Copy entire source and resize and paste into entire dest
void blit_scale8(pic8* dest, pic8* source) {
    blit_scale8(dest, source, 0, 0, dest->get_width() - 1, dest->get_height() - 1);
}

// Only horizontal and vertical lines can be drawn
void pic8::line(int x1, int y1, int x2, int y2, unsigned char index) {
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
        for (int y = y1; y <= y2; y++) {
            ppixel(x1, y, index);
        }
        return;
    }
    if (y1 == y2) {
        for (int x = x1; x <= x2; x++) {
            ppixel(x, y1, index);
        }
        return;
    }
    internal_error("pic8::line diagonal lines not implemented!");
}

// Make this picture point by reference of a subview of the source picture
void pic8::subview(int x1, int y1, int x2, int y2, pic8* source) {
    width = x2 - x1 + 1;
    height = y2 - y1 + 1;
#ifdef DEBUG
    if (x1 < 0 || x2 >= SCREEN_WIDTH || y1 < 0 || y2 >= SCREEN_HEIGHT) {
        internal_error("pic8::subview!");
    }
#endif
    for (int y = 0; y < height; y++) {
        rows[y] = source->get_row(y + y1) + x1;
    }
}
