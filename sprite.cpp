#include "sprite.h"
#include "editor_canvas.h"
#include "fs_utils.h"
#include "lgr.h"
#include "main.h"
#include "physics_init.h"
#include <cstring>

sprite::sprite(double x, double y, const char* pic_name, const char* text_name,
               const char* mask_nam) {
    if (!Plgr) {
        internal_error("sprite::sprite !Plgr");
    }
    r = vect2(x, y);
    if (strlen(pic_name) > MAX_FILENAME_LEN || strlen(mask_nam) > MAX_FILENAME_LEN ||
        strlen(text_name) > MAX_FILENAME_LEN) {
        internal_error("sprite::sprite name too long!");
    }
    strcpy(picture_name, pic_name);
    strcpy(texture_name, text_name);
    strcpy(mask_name, mask_nam);

    // Default values - they should all be overwritten below (or else crash)
    distance = 100;
    clipping = Clipping::Unclipped;
    wireframe_width = PixelsToMeters * DEFAULT_SPRITE_WIREFRAME;
    wireframe_height = PixelsToMeters * DEFAULT_SPRITE_WIREFRAME;

    if (picture_name[0] && (mask_name[0] || texture_name[0])) {
        internal_error("sprite::sprite invalid pic/mask/text combination!");
    }
    if (picture_name[0]) {
        int index = Plgr->getkepindex(picture_name);
        if (index < 0) {
            picture_name[0] = 0;
        } else {
            wireframe_width = Plgr->kepek[index].xsize * PixelsToMeters;
            wireframe_height = Plgr->kepek[index].ysize * PixelsToMeters;
            distance = Plgr->kepek[index].tavolsag;
            clipping = Plgr->kepek[index].hatarol;
        }
    } else {
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

void sprite::render() {
    render_line(r, r + vect2(wireframe_width, 0.0), false);
    render_line(r, r + vect2(0.0, wireframe_height), false);
    vect2 r2(wireframe_width + r.x, wireframe_height + r.y);
    render_line(r2, r2 - vect2(wireframe_width, 0.0), false);
    render_line(r2, r2 - vect2(0.0, wireframe_height), false);
}

sprite::sprite(FILE* h) {
    if (fread(picture_name, 1, 10, h) != 10) {
        internal_error("Failed to read sprite from file!");
    }
    picture_name[9] = 0;
    if (fread(texture_name, 1, 10, h) != 10) {
        internal_error("Failed to read sprite from file!");
    }
    texture_name[9] = 0;
    if (fread(mask_name, 1, 10, h) != 10) {
        internal_error("Failed to read sprite from file!");
    }
    mask_name[9] = 0;

    if (fread(&r.x, 1, sizeof(r.x), h) != 8) {
        internal_error("Failed to read sprite from file!");
    }
    if (fread(&r.y, 1, sizeof(r.y), h) != 8) {
        internal_error("Failed to read sprite from file!");
    }
    if (fread(&distance, 1, sizeof(distance), h) != 4) {
        internal_error("Failed to read sprite from file!");
    }
    if (fread(&clipping, 1, sizeof(clipping), h) != 4) {
        internal_error("Failed to read sprite from file!");
    }
}

void sprite::save(FILE* h) {
    if (fwrite(picture_name, 1, 10, h) != 10) {
        internal_error("Failed to write sprite to file!");
    }
    if (fwrite(texture_name, 1, 10, h) != 10) {
        internal_error("Failed to write sprite to file!");
    }
    if (fwrite(mask_name, 1, 10, h) != 10) {
        internal_error("Failed to write sprite to file!");
    }
    if (fwrite(&r.x, 1, sizeof(r.x), h) != 8) {
        internal_error("Failed to write sprite to file!");
    }
    if (fwrite(&r.y, 1, sizeof(r.y), h) != 8) {
        internal_error("Failed to write sprite to file!");
    }
    if (fwrite(&distance, 1, sizeof(distance), h) != 4) {
        internal_error("Failed to write sprite to file!");
    }
    if (fwrite(&clipping, 1, sizeof(clipping), h) != 4) {
        internal_error("Failed to write sprite to file!");
    }
}

double sprite::checksum() {
    double sum = 0.0;
    sum += r.x;
    sum += r.y;
    return sum;
}
