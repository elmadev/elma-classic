#include "WAV.H"
#include "main.h"
#include "platform_utils.h"
#include "qopen.h"
#include <cstring>

void wav::lefoglal(void) {
    if (size > 1000000) {
        internal_error("wav::alloc-ban size > 1000000!");
    }
    tomb = new signed short[size];
    if (!tomb) {
        external_error("wav::alloc-ban memory!");
    }
}

static void nevwavtest(const char* nev) {
    int i = strlen(nev) - 1;
    while (i > 0) {
        if (nev[i] == '.') {
            // Most ponton all:
            if (strcmpi(".wav", &nev[i]) == 0) {
                return;
            } else {
                internal_error("nevrawtest-ben nincs .raw!");
            }
        }
        i--;
    }
    internal_error("nevwavtest-ben nincs .wav!");
}

// Wav Header specification
// https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
// We assume the smallest possible wav header, i.e. fmt size is 16,
// and all optional chunks are skipped.
struct wav_header {
    // "riff"
    char riff[4];
    int riff_size;
    // "WAVE"
    char wave[4];
    // "fmt "
    char fmt[4];
    int fmt_size;
    short format;
    short channels;
    int sample_rate;
    int byte_rate;
    short block_align;
    short bits_per_sample;
    // "data"
    char data[4];
    int data_size;
};

wav::wav(const char* nev, double maxamplitudo, int start, int ucsoutani) {
    tomb = NULL;
    nevwavtest(nev);
    // Lemeri file hosszat:
    FILE* h = qopen(nev, "rb");
    if (!h) {
        internal_error("Nem tudta megnyitni file-t: ", nev);
    }

    wav_header header;
    int negyvennegy = 44;
    if (sizeof(wav_header) != negyvennegy) {
        internal_error("sizeof( wav_header ) != 44!");
    }

    if (fread(&header, 1, 44, h) != 44) {
        internal_error("Nem sikerult beolvasni wav-ot!");
    }

    long hossz = header.data_size;
    int bit16 = 1;

    // Reszlet eldontese:
    int reszlet = 1;
    if (ucsoutani <= 0) {
        reszlet = 0;
    }
    int reszhossz = ucsoutani - start;
    if (!reszlet) {
        start = 0;
        if (bit16) {
            reszhossz = hossz / 2;
        } else {
            reszhossz = hossz;
        }
    }

    if (header.channels != 1) {
        internal_error("Wav file nem mono!: ", nev);
    }

    if (bit16) {
        // 16 bites uncompressed:
        if (hossz % 2) {
            internal_error("16 bites wav-nal nem paros byteszam! (abcd)");
        }
        size = reszhossz;
        if (start + reszhossz > hossz / 2) {
            internal_error("Reszlet kilog wav-bol!");
        }
        lefoglal();
        qseek(h, start * 2, SEEK_CUR);
        if (fread(tomb, 1, size * 2, h) != size * 2) {
            internal_error("Nem sikerult beolvasni wav-ot!: ", nev);
        }
    } else {
        // 8 bites uncompressed:
        if (reszlet) {
            internal_error("8 bitre meg nincs megirva reszlet!");
        }
        size = hossz;
        lefoglal();
        if (fread(tomb, 1, hossz, h) != hossz) {
            internal_error("Nem sikerult beolvasni wav-ot!: ", nev);
        }
        unsigned char* bytetomb = (unsigned char*)tomb;
        for (int i = size - 1; i >= 0; i--) {
            tomb[i] = short((((short)(bytetomb[i])) - 128) << 8);
        }
    }

    qclose(h);

    // Megkeresi maximalis amplitudot:
    int maxamp = 1;
    for (int i = 0; i < size; i++) {
        if (tomb[i] > 0) {
            if (tomb[i] > maxamp) {
                maxamp = tomb[i];
            }
        } else {
            if (-tomb[i] > maxamp) {
                maxamp = -tomb[i];
            }
        }
    }
    // Kiszamolja szorzot:
    double szorzo = 32000.0 * maxamplitudo / double(maxamp);
    for (int i = 0; i < size; i++) {
        tomb[i] *= szorzo;
    }
}

// Elejere tesz egy loopot a vegebol, amit aztan le is vag:
void wav::loopol(int n) {
    if (n >= size) {
        internal_error("n >= size!");
    }
    for (int i = 0; i < n; i++) {
        double arany = ((double)i) / n;
        tomb[i] = arany * tomb[i] + (1 - arany) * tomb[size - n + i];
    }
    size -= n;
}

// pwav elejet rateszi vegere:
void wav::vegereilleszt(wav* pwav, int n) {
    if (n >= size) {
        internal_error("n >= size!");
    }
    if (n >= pwav->size) {
        internal_error("n >= pwav->size!");
    }
    for (int i = 0; i < n; i++) {
        double arany = ((double)i) / n;
        tomb[size - n + i] = (1 - arany) * tomb[i] + arany * pwav->tomb[i];
    }
}

void wav::hangero(double szorzo) {
    for (int i = 0; i < size; i++) {
        tomb[i] *= szorzo;
    }
}

// WAV2 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// short* tomb, mtomb;
wav2::wav2(wav* pwav) {
    mtomb = NULL;
    tomb = pwav->tomb;
    if (pwav->size > 64000) {
        internal_error("wav2-ben size > 64000!");
    }
    mtomb = new short[pwav->size];
    if (!mtomb) {
        external_error("memory");
    }
    for (int i = 0; i < pwav->size - 1; i++) {
        mtomb[i] = tomb[i + 1] - tomb[i];
    }
    mtomb[pwav->size - 1] = tomb[0] - tomb[pwav->size - 1];
    ulsize = pwav->size;
    ulfuto = 0.0;
}

void wav2::reset(int fazis) {
    if (fazis < 0) {
        ulfuto = 0.0;
    } else {
        ulfuto = fazis;
    }
}

// dt 65536 szoros:
short wav2::getnextsample(double dt) {
    ulfuto += dt;
    if (ulfuto >= ulsize) {
        ulfuto -= ulsize;
    }
    int whole = (int)(ulfuto);
    double fraction = ulfuto - whole;
    return tomb[whole] + (short)(mtomb[whole] * fraction);
}
