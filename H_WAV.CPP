#include "WAV.H"
#include "main.h"
#include "platform_utils.h"
#include "qopen.h"
#include <cstring>

void wav::allocate(void) {
    if (size > 1000000) {
        internal_error("wav::alloc-ban size > 1000000!");
    }
    samples = new signed short[size];
    if (!samples) {
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

wav::wav(const char* filename, double max_volume, int start, int end) {
    samples = NULL;
    nevwavtest(filename);
    // Lemeri file hosszat:
    FILE* h = qopen(filename, "rb");
    if (!h) {
        internal_error("Nem tudta megnyitni file-t: ", filename);
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
    if (end <= 0) {
        reszlet = 0;
    }
    int reszhossz = end - start;
    if (!reszlet) {
        start = 0;
        if (bit16) {
            reszhossz = hossz / 2;
        } else {
            reszhossz = hossz;
        }
    }

    if (header.channels != 1) {
        internal_error("Wav file nem mono!: ", filename);
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
        allocate();
        qseek(h, start * 2, SEEK_CUR);
        if (fread(samples, 1, size * 2, h) != size * 2) {
            internal_error("Nem sikerult beolvasni wav-ot!: ", filename);
        }
    } else {
        // 8 bites uncompressed:
        if (reszlet) {
            internal_error("8 bitre meg nincs megirva reszlet!");
        }
        size = hossz;
        allocate();
        if (fread(samples, 1, hossz, h) != hossz) {
            internal_error("Nem sikerult beolvasni wav-ot!: ", filename);
        }
        unsigned char* bytetomb = (unsigned char*)samples;
        for (int i = size - 1; i >= 0; i--) {
            samples[i] = short((((short)(bytetomb[i])) - 128) << 8);
        }
    }

    qclose(h);

    // Megkeresi maximalis amplitudot:
    int maxamp = 1;
    for (int i = 0; i < size; i++) {
        if (samples[i] > 0) {
            if (samples[i] > maxamp) {
                maxamp = samples[i];
            }
        } else {
            if (-samples[i] > maxamp) {
                maxamp = -samples[i];
            }
        }
    }
    // Kiszamolja szorzot:
    double szorzo = 32000.0 * max_volume / double(maxamp);
    for (int i = 0; i < size; i++) {
        samples[i] *= szorzo;
    }
}

// Elejere tesz egy loopot a vegebol, amit aztan le is vag:
void wav::loop(int fade_length) {
    if (fade_length >= size) {
        internal_error("fade_length >= size!");
    }
    for (int i = 0; i < fade_length; i++) {
        double arany = ((double)i) / fade_length;
        samples[i] = arany * samples[i] + (1 - arany) * samples[size - fade_length + i];
    }
    size -= fade_length;
}

// pwav elejet rateszi vegere:
void wav::fade(wav* next, int fade_length) {
    if (fade_length >= size) {
        internal_error("fade_length >= size!");
    }
    if (fade_length >= next->size) {
        internal_error("fade_length >= next->size!");
    }
    for (int i = 0; i < fade_length; i++) {
        double arany = ((double)i) / fade_length;
        samples[size - fade_length + i] = (1 - arany) * samples[i] + arany * next->samples[i];
    }
}

void wav::volume(double scale) {
    for (int i = 0; i < size; i++) {
        samples[i] *= scale;
    }
}

// WAV2 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// short* tomb, mtomb;
wav2::wav2(wav* source) {
    deltas = NULL;
    samples = source->samples;
    if (source->size > 64000) {
        internal_error("wav2-ben size > 64000!");
    }
    deltas = new short[source->size];
    if (!deltas) {
        external_error("memory");
    }
    for (int i = 0; i < source->size - 1; i++) {
        deltas[i] = samples[i + 1] - samples[i];
    }
    deltas[source->size - 1] = samples[0] - samples[source->size - 1];
    size = source->size;
    playback_index = 0.0;
}

void wav2::reset(int index) {
    if (index < 0) {
        playback_index = 0.0;
    } else {
        playback_index = index;
    }
}

// dt 65536 szoros:
short wav2::get_next_sample(double dt) {
    playback_index += dt;
    if (playback_index >= size) {
        playback_index -= size;
    }
    int whole = (int)(playback_index);
    double fraction = playback_index - whole;
    return samples[whole] + (short)(deltas[whole] * fraction);
}
