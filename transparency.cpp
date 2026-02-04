#include "main.h"
#include "pic8.h"
#include <cstring>

constexpr int SPRITE_MAX_RUN_LENGTH = 255;

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

// Generate transparency data with a specified transparency palette index
void pic8::add_transparency(int transparency) {
    constexpr int SPRITE_MAX_BUFFER = 20000;
    transparency_data_length = 0;
    unsigned char* buffer = nullptr;
    buffer = new unsigned char[SPRITE_MAX_BUFFER];
    if (!buffer) {
        external_error("add_transparency memory!");
        return;
    }

    // Transparency data format:
    //       ['K', length] -> the next length pixels are solid
    //       ['N', length] -> the next length pixels are transparent
    int xsize = get_width();
    int ysize = get_height();
    unsigned int buffer_length = 0;
    for (int y = 0; y < ysize; y++) {
        int x = 0;
        while (x < xsize) {
            int solid_count = consecutive_solid_pixels(this, x, y, transparency);
            if (solid_count) {
                x += solid_count;
                buffer[buffer_length++] = 'K';
                buffer[buffer_length++] = solid_count;
            } else {
                int transparent_count = consecutive_transparent_pixels(this, x, y, transparency);
                x += transparent_count;
                buffer[buffer_length++] = 'N';
                buffer[buffer_length++] = transparent_count;
            }
            if (buffer_length >= SPRITE_MAX_BUFFER) {
                internal_error("add_transparency buffer too small!");
                delete buffer;
                return;
            }
        }
    }

    // Copy data to new buffer of appropriate size
    transparency_data = new unsigned char[buffer_length];
    if (!transparency_data) {
        external_error("add_transparency memory!");
        delete buffer;
        return;
    }
    memcpy(transparency_data, buffer, buffer_length);
    transparency_data_length = buffer_length;
    delete buffer;
}

// Generate transparency data using the top-left pixel as the transparency palette index
void pic8::add_transparency() { add_transparency(gpixel(0, 0)); }
