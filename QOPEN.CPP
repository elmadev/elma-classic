#include "ALL.H"

constexpr int USE_RES_FILE = 1;

static char RES_FILENAME[] = "elma.res";

struct res_file {
    char filename[16];
    // In Shareware the order of these two fields is reversed.
    int length;
    int offset;
};

static res_file* ResFiles = NULL;

#define RES_MAX_FILES (150)

#define RES_MAGIC_NUMBER (1347839l)

static long FileCount = 0;

static void decrypt(void) {
    short a = 23, b = 9982, c = 3391; // In Shareware, b is 9882

    if (!ResFiles) {
        internal_error("titkosit-ban !Leirotomb!");
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

static int QOpenInitialized = 0;

void init_qopen(void) {
    if (QOpenInitialized) {
        internal_error("initqopen tobbszor hivva!");
    }
    QOpenInitialized = 1;

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

    if (fread(&FileCount, 1, 4, h) != 4) {
        internal_error("initqopen-ben nem sikerult olvasni!");
    }
    if (FileCount <= 0 || FileCount > RES_MAX_FILES) {
        internal_error("FileCount <= 0 || FileCount > RES_MAX_FILES!");
    }

    long size = sizeof(res_file[RES_MAX_FILES]);
    if (fread(ResFiles, 1, unsigned(size), h) != size) {
        internal_error("initqopen-ben nem sikerult olvasni!");
    }
    decrypt();

    long magic_number = 0;
    if (fread(&magic_number, 1, 4, h) != 4) {
        internal_error("initqopen-ben nem sikerult olvasni!");
    }
    if (magic_number != RES_MAGIC_NUMBER) {
        internal_error("initqopen-ben nem stimmel magicnumber!");
    }

    fclose(h);
}

#define MAX_HANDLES (3)
static FILE* Handles[MAX_HANDLES];
static int HandleOffset[MAX_HANDLES];

static int NumHandles = 0;

FILE* qopen(const char* filename, const char* mode) {
    if (!QOpenInitialized) {
        internal_error("qopen initqopen nelkul!");
    }
    if (NumHandles == MAX_HANDLES) {
        internal_error("qopen-ben Nyitva == MAXNYITVA!");
    }
    if (strcmp(mode, "rb") != 0 && strcmp(mode, "rt")) {
        internal_error("qopen-ben sem rb, sem rt!: ", filename);
    }

    if (USE_RES_FILE) {
        for (int i = 0; i < FileCount; i++) {
            if (strcmpi(filename, ResFiles[i].filename) == 0) {
                Handles[NumHandles] = fopen(RES_FILENAME, mode);
                HandleOffset[NumHandles] = i;
                if (!Handles[NumHandles]) {
                    internal_error("Nem sikerult nagy tombot megnyitni!: ", RES_FILENAME);
                }
                if (fseek(Handles[NumHandles], ResFiles[i].offset, SEEK_SET) != 0) {
                    internal_error("Nem mukodik fseek qopen-ben!");
                }

                NumHandles++;
                return Handles[NumHandles - 1];
            }
        }
        internal_error("qopen nem talalta meg nagyfileban file-t!: ", filename);
        return NULL;
    } else {
        NumHandles++;
        char tmp[30] = "files/";
        if (strlen(filename) > 12 || filename[0] == '.') {
            internal_error("Hibas nev qopen-ben!: ", filename);
        }
        strcat(tmp, filename);
        return fopen(tmp, mode);
    }
}

void qclose(FILE* h) {
    if (!QOpenInitialized) {
        internal_error("qclose initqopen nelkul!");
    }
    if (!NumHandles) {
        internal_error("qclose-ban !Nyitva!");
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
        internal_error("qclose nem talalta meg h-t!");
    } else {
        NumHandles--;
        fclose(h);
    }
}

int qseek(FILE* h, long offset, int whence) {
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
        internal_error("qclose nem talalta meg h-t!");
        return 0;
    } else {
        return fseek(h, offset, whence);
    }
}
