#include "main.h"
#include "platform_utils.h"
#include <cstdio>
#include <cstring>

struct res_file {
    char filename[16];
    // In Shareware the order of these two fields is reversed.
    int length;
    int offset;
};

constexpr char RES_FILENAME[] = "elma.res";
constexpr int RES_MAGIC_NUMBER = 1347839;
constexpr int RES_MAX_FILES_LEGACY = 150;
constexpr int RES_MAX_FILES = 3000;
constexpr bool USE_RES_FILE = true;

static res_file* ResFiles = nullptr;
static int ResMaxFiles;
static int FileCount = 0;

static void decrypt() {
    short a = 23;
    // In Shareware, b is 9882
    short b = 9982;
    short c = 3391;

    if (!ResFiles) {
        internal_error("ResFiles is NULL!");
    }
    for (int i = 0; i < ResMaxFiles; i++) {
        unsigned char* pc = (unsigned char*)(&ResFiles[i]);
        for (int j = 0; j < sizeof(res_file); j++) {
            pc[j] ^= a;
            a %= c;
            b += a * c;
            a = 31 * b + c;
        }
    }
}

static bool QOpenInitialized = false;

static bool magic_number_at(FILE* h, int offset) {
    fseek(h, sizeof(FileCount) + sizeof(res_file) * offset, SEEK_SET);
    int magic_number = 0;
    if (fread(&magic_number, 1, sizeof(magic_number), h) != 4) {
        internal_error("init_qopen() cannot read magic_number!");
    }
    fseek(h, 0, SEEK_SET);

    return magic_number == RES_MAGIC_NUMBER;
}

void init_qopen() {
    if (QOpenInitialized) {
        internal_error("init_qopen() already called!");
    }
    QOpenInitialized = true;

    if (!USE_RES_FILE) {
        return;
    }

    FILE* h = fopen(RES_FILENAME, "rb");
    if (!h) {
        external_error("Missing file!: ", RES_FILENAME);
    }

    // There are two different .res formats, with no explicit versioning.
    // The only difference is the maximum amount of files allowed in the .res file.
    // The magic number marking the end of the header will therefore be at different offsets.
    if (magic_number_at(h, RES_MAX_FILES_LEGACY)) {
        ResMaxFiles = RES_MAX_FILES_LEGACY;
    } else if (magic_number_at(h, RES_MAX_FILES)) {
        ResMaxFiles = RES_MAX_FILES;
    } else {
        internal_error(".res file is corrupt!");
    }

    if (fread(&FileCount, 1, sizeof(FileCount), h) != 4) {
        internal_error("init_qopen() cannot read FileCount!");
    }
    if (FileCount <= 0 || FileCount > ResMaxFiles) {
        internal_error("FileCount <= 0 || FileCount > ResMaxFiles!");
    }

    ResFiles = new res_file[ResMaxFiles];
    if (!ResFiles) {
        external_error("init_qopen() out of memory!");
    }

    int size = sizeof(res_file) * ResMaxFiles;
    if (fread(ResFiles, 1, size, h) != size) {
        internal_error("init_qopen() cannot read files!");
    }
    decrypt();

    fclose(h);
}

constexpr int MAX_HANDLES = 3;
static FILE* Handles[MAX_HANDLES];
static int HandleOffset[MAX_HANDLES];

static int NumHandles = 0;

FILE* qopen(const char* filename, const char* mode) {
    if (!QOpenInitialized) {
        internal_error("qopen() called before init_qopen()!");
    }
    if (NumHandles == MAX_HANDLES) {
        internal_error("NumHandles == MAX_HANDLES!");
    }
    if (strcmp(mode, "rb") != 0 && strcmp(mode, "r") != 0) {
        internal_error("qopen() mode is not \"rb\" or \"r\"!: ", filename, mode);
    }

    if (USE_RES_FILE) {
        for (int i = 0; i < FileCount; i++) {
            if (strcmpi(filename, ResFiles[i].filename) == 0) {
                Handles[NumHandles] = fopen(RES_FILENAME, mode);
                HandleOffset[NumHandles] = i;
                if (!Handles[NumHandles]) {
                    internal_error("qopen() failed to open: ", RES_FILENAME);
                }
                if (fseek(Handles[NumHandles], ResFiles[i].offset, SEEK_SET) != 0) {
                    internal_error("qopen() failed to fseek!");
                }

                NumHandles++;
                return Handles[NumHandles - 1];
            }
        }
        internal_error("qopen() failed to find file: ", filename);
        return nullptr;
    } else {
        NumHandles++;
        char tmp[30] = "files/";
        if (strlen(filename) > 12 || filename[0] == '.') {
            internal_error("qopen() malformed filename!: ", filename);
        }
        strcat(tmp, filename);
        return fopen(tmp, mode);
    }
}

void qclose(FILE* h) {
    if (!QOpenInitialized) {
        internal_error("init_qopen() not yet called!");
    }
    if (!NumHandles) {
        internal_error("qclose() no handles open!");
    }
    if (USE_RES_FILE) {
        for (int i = 0; i < NumHandles; i++) {
            if (Handles[i] == h) {
                fclose(h);
                for (int j = i; j < NumHandles - 1; j++) {
                    Handles[j] = Handles[j + 1];
                    HandleOffset[j] = HandleOffset[j + 1];
                }
                NumHandles--;
                return;
            }
        }
        internal_error("qclose() cannot find handle!");
    } else {
        NumHandles--;
        fclose(h);
    }
}

int qseek(FILE* h, int offset, int whence) {
    if (whence != SEEK_SET && whence != SEEK_END && whence != SEEK_CUR) {
        internal_error("whence != SEEK_SET && whence != SEEK_END && whence != SEEK_CUR!");
    }
    if (USE_RES_FILE) {
        for (int i = 0; i < NumHandles; i++) {
            if (Handles[i] == h) {
                int index = HandleOffset[i];
                if (whence == SEEK_SET) {
                    return fseek(Handles[i], ResFiles[index].offset + offset, SEEK_SET);
                }
                if (whence == SEEK_END) {
                    return fseek(Handles[i],
                                 ResFiles[index].offset + ResFiles[index].length + offset,
                                 SEEK_SET);
                }
                return fseek(Handles[i], offset, SEEK_CUR);
            }
        }
        internal_error("qseek() can't find handle!");
        return 0;
    } else {
        return fseek(h, offset, whence);
    }
}
