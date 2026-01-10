#include "ALL.H"
#include "main.h"
#include "platform_utils.h"
#include <stdio.h>
#include <string.h>

struct res_file {
    char filename[16];
    // In Shareware the order of these two fields is reversed.
    int length;
    int offset;
};

constexpr char RES_FILENAME[] = "elma.res";
constexpr int RES_MAGIC_NUMBER = 1347839;
constexpr int RES_MAX_FILES = 150;
constexpr bool USE_RES_FILE = true;

static res_file* ResFiles = nullptr;
static int FileCount = 0;

static void decrypt() {
    short a = 23;
    // In Shareware, b is 9882
    short b = 9982;
    short c = 3391;

    if (!ResFiles) {
        internal_error("ResFiles is NULL!");
    }
    for (int i = 0; i < RES_MAX_FILES; i++) {
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

void init_qopen() {
    if (QOpenInitialized) {
        internal_error("init_qopen() already called!");
    }
    QOpenInitialized = true;

    if (!USE_RES_FILE) {
        return;
    }

    ResFiles = new res_file[RES_MAX_FILES];
    if (!ResFiles) {
        external_error("initqopen-ben memory!");
    }
    FILE* h = fopen(RES_FILENAME, "rb");
    if (!h) {
        external_error("Missing file!: ", RES_FILENAME);
    }

    if (fread(&FileCount, 1, sizeof(FileCount), h) != 4) {
        internal_error("init_qopen() cannot read FileCount!");
    }
    if (FileCount <= 0 || FileCount > RES_MAX_FILES) {
        internal_error("FileCount <= 0 || FileCount > RES_MAX_FILES!");
    }

    int size = sizeof(res_file[RES_MAX_FILES]);
    if (fread(ResFiles, 1, size, h) != size) {
        internal_error("init_qopen() cannot read files!");
    }
    decrypt();

    int magic_number = 0;
    if (fread(&magic_number, 1, sizeof(magic_number), h) != 4) {
        internal_error("init_qopen() cannot read magic_number!");
    }
    if (magic_number != RES_MAGIC_NUMBER) {
        internal_error("init_qopen() invalid magic_number!");
    }

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
