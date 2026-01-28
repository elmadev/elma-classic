#include "piclist.h"
#include "main.h"
#include "platform_utils.h"
#include <cstdio>
#include <cstring>

constexpr int PICLIST_VERSION = 1002;

piclist::piclist(FILE* h) {
    int version = 0;
    if (fread(&version, 1, 4, h) != 4) {
        external_error("Cannot read pictures.lst information!");
    }
    if (version != PICLIST_VERSION) {
        external_error("In LGR file the pictures.lst information has incorrect version!");
    }
    if (fread(&length, 1, 4, h) != 4) {
        external_error("Cannot read pictures.lst information!");
    }
    if (fread(name, 1, length * 10, h) != length * 10) {
        external_error("Cannot read pictures.lst information!");
    }
    if (fread(type, 1, length * 4, h) != length * 4) {
        external_error("Cannot read pictures.lst information!");
    }
    if (fread(default_distance, 1, length * 4, h) != length * 4) {
        external_error("Cannot read pictures.lst information!");
    }
    if (fread(default_clipping, 1, length * 4, h) != length * 4) {
        external_error("Cannot read pictures.lst information!");
    }
    if (fread(transparency, 1, length * 4, h) != length * 4) {
        external_error("Cannot read pictures.lst information!");
    }
}

int piclist::get_index(const char* pic_name) {
    for (int i = 0; i < length; i++) {
        if (strcmpi(name[i], pic_name) == 0) {
            return i;
        }
    }
    return -1;
}
