#include "transparency.h"
#include "main.h"
#include "pic8.h"
#include <cstring>

#define SPRITE_MAX_BUFFER (20000u)
static int SPRITE_MAX_RUN_LENGTH = 255;

// Count number of solid pixels starting from x, y, up to a length of 255 or end of row
static int consecutive_solid_pixels(pic8* pic, int x, int y, unsigned char transparency) {
    int count = 0;
    while (x < pic->get_width() && pic->gpixel(x, y) != transparency &&
           count < SPRITE_MAX_RUN_LENGTH) {
        x++;
        count++;
    }
    return count;
}

// Count number of transparent pixels starting from x, y, up to a length of 255 or end of row
static int consecutive_transparent_pixels(pic8* pic, int x, int y, unsigned char transparency) {
    int count = 0;
    while (x < pic->get_width() && pic->gpixel(x, y) == transparency &&
           count < SPRITE_MAX_RUN_LENGTH) {
        x++;
        count++;
    }
    return count;
}

// Transparency data format:
//       ['K', length] -> the next length pixels are solid
//       ['N', length] -> the next length pixels are transparent
unsigned char* create_transparency_buffer(pic8* pic, unsigned char transparency,
                                          unsigned short* transparency_length) {
    *transparency_length = 0;
    unsigned char* buffer = nullptr;
    buffer = new unsigned char[SPRITE_MAX_BUFFER];
    if (!buffer) {
        external_error("create_transparency_buffer memory!");
        return nullptr;
    }
    int xsize = pic->get_width();
    int ysize = pic->get_height();
    unsigned buffer_length = 0;
    for (int y = 0; y < ysize; y++) {
        int x = 0;
        while (x < xsize) {
            int solid_count = consecutive_solid_pixels(pic, x, y, transparency);
            if (solid_count) {
                x += solid_count;
                buffer[buffer_length++] = 'K';
                buffer[buffer_length++] = solid_count;
            } else {
                int transparent_count = consecutive_transparent_pixels(pic, x, y, transparency);
                x += transparent_count;
                buffer[buffer_length++] = 'N';
                buffer[buffer_length++] = transparent_count;
            }
            if (buffer_length >= SPRITE_MAX_BUFFER) {
                internal_error("create_transparency_buffer buffer too small!");
                delete buffer;
                return nullptr;
            }
        }
    }

    // Copy data to new buffer of appropriate size
    unsigned char* new_buffer = nullptr;
    new_buffer = new unsigned char[buffer_length];
    if (!new_buffer) {
        external_error("create_transparency_buffer memory!");
        delete buffer;
        return nullptr;
    }
    memcpy(new_buffer, buffer, buffer_length);

    *transparency_length = buffer_length;

    delete buffer;
    return new_buffer;
}

// Generate transparency data with a specified transparency palette index
void add_transparency(pic8* pic, int transparency) {
    pic->transparency_data =
        create_transparency_buffer(pic, transparency, &pic->transparency_data_length);
}

// Generate transparency data using the top-left pixel as the transparency palette index
void add_transparency(pic8* pic) { add_transparency(pic, pic->gpixel(0, 0)); }
