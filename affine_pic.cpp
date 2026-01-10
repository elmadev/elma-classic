#include "affine_pic.h"
#include "main.h"
#include "pic8.h"

// Create a picture that may be rotated and stretched.
// Provide either a filename or a pic8.
affine_pic::affine_pic(const char* filename, pic8* pic) {
    if (!pic) {
        pic = new pic8(filename);
    }

    // Transparency is hard-coded to the topleft
    transparency = pic->gpixel(0, 0);

    // We pad each row to a length of 256 for faster rendering,
    // so the max width is 255
    width = pic->get_width();
    height = pic->get_height();
    if (width > 255 || height > 255) {
        internal_error("affine_pic size > 255!");
    }

    int length = height * 256;
    pixels = new unsigned char[length];
    if (!pixels) {
        external_error("affine_pic out of memory!");
    }
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixels[y * 256 + x] = pic->gpixel(x, y);
        }
    }

    delete pic;
}

affine_pic::~affine_pic() {
    if (pixels) {
        delete pixels;
    }
}
