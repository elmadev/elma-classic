#include "ALL.H"

// Binaris formatumbol:
piclist::piclist(FILE* h) {
    // Elso verzio 1002
    int version = 0;
    if (fread(&version, 1, 4, h) != 4) {
        uzenet("Cannot read pictures.lst information!");
    }
    if (version != 1002) {
        uzenet("In LGR file the pictures.lst information has incorrect version!");
    }
    if (fread(&length, 1, 4, h) != 4) {
        uzenet("Cannot read pictures.lst information!");
    }
    if (fread(name, 1, length * 10, h) != length * 10) {
        uzenet("Cannot read pictures.lst information!");
    }
    if (fread(type, 1, length * 4, h) != length * 4) {
        uzenet("Cannot read pictures.lst information!");
    }
    if (fread(default_distance, 1, length * 4, h) != length * 4) {
        uzenet("Cannot read pictures.lst information!");
    }
    if (fread(default_clipping, 1, length * 4, h) != length * 4) {
        uzenet("Cannot read pictures.lst information!");
    }
    if (fread(transparency, 1, length * 4, h) != length * 4) {
        uzenet("Cannot read pictures.lst information!");
    }
}

int piclist::get_index(const char* pic_name) {
    for (int i = 0; i < length; i++) {
        if (strcmpi(&name[i * 10], pic_name) == 0) {
            return i;
        }
    }
    return -1;
}
