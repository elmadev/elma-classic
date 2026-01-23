#include "WAV.H"
#include "main.h"
#include "platform_utils.h"
#include "qopen.h"
#include <cstring>

void wav::allocate() {
    if (size > 1000000) {
        internal_error("wav::alloc size > 1000000!");
    }
    samples = new signed short[size];
    if (!samples) {
        external_error("wav::alloc out of memory!");
    }
}

static void assert_filename_is_wav(const char* filename) {
    for (int i = strlen(filename) - 1; i > 0; i--) {
        if (filename[i] == '.') {
            if (strcmpi(".wav", &filename[i]) == 0) {
                return;
            }
            internal_error("assert_filename_is_wav not .wav!");
        }
    }
    internal_error("assert_filename_is_wav no extension!");
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
static_assert(sizeof(wav_header) == 44);

wav::wav(const char* filename, double max_volume, int start, int end) {
    samples = nullptr;
    assert_filename_is_wav(filename);
    FILE* h = qopen(filename, "rb");
    if (!h) {
        internal_error("Failed to open wav file: ", filename);
    }

    // Assume the header follows the specifications for the smallest possible header size
    wav_header header;
    if (fread(&header, 1, sizeof(wav_header), h) != sizeof(wav_header)) {
        internal_error("Failed to read wav file header!");
    }

    int source_size = header.data_size;
    bool bit16 = true;

    bool custom_range = true;
    if (end <= 0) {
        custom_range = false;
    }
    int target_length = end - start;
    if (!custom_range) {
        start = 0;
        if (bit16) {
            target_length = source_size / 2;
        } else {
            target_length = source_size;
        }
    }

    if (header.channels != 1) {
        internal_error("Wav file is not mono!: ", filename);
    }

    if (bit16) {
        if (source_size % 2) {
            internal_error("16-bit wav file must have an even number of data bytes");
        }
        size = target_length;
        if (start + target_length > source_size / 2) {
            internal_error("wav::wav end is out of range!");
        }
        allocate();
        qseek(h, start * 2, SEEK_CUR);
        if (fread(samples, 1, size * 2, h) != size * 2) {
            internal_error("Failed to read wav file: ", filename);
        }
    } else {
        if (custom_range) {
            internal_error("8-bit wav file cannot have a custom start or end!");
        }
        size = source_size;
        allocate();
        if (fread(samples, 1, source_size, h) != source_size) {
            internal_error("Failed to read wav file: ", filename);
        }
        // Convert the data from unsigned char (centered at 128) to short (centered at 0).
        unsigned char* samples_as_char = (unsigned char*)samples;
        for (int i = size - 1; i >= 0; i--) {
            samples[i] = short((((short)(samples_as_char[i])) - 128) << 8);
        }
    }

    qclose(h);

    // Calculate the max amplitude
    int max_amplitude = 1;
    for (int i = 0; i < size; i++) {
        if (samples[i] > 0) {
            if (samples[i] > max_amplitude) {
                max_amplitude = samples[i];
            }
        } else {
            if (-samples[i] > max_amplitude) {
                max_amplitude = -samples[i];
            }
        }
    }
    // Scale based on the max amplitude and max volume
    double scale = 32000.0 * max_volume / double(max_amplitude);
    for (int i = 0; i < size; i++) {
        samples[i] *= scale;
    }
}

void wav::loop(int fade_length) {
    if (fade_length >= size) {
        internal_error("fade_length >= size!");
    }
    for (int i = 0; i < fade_length; i++) {
        double fade = ((double)i) / fade_length;
        samples[i] = fade * samples[i] + (1 - fade) * samples[size - fade_length + i];
    }
    size -= fade_length;
}

void wav::fade(wav* next, int fade_length) {
    if (fade_length >= size) {
        internal_error("fade_length >= size!");
    }
    if (fade_length >= next->size) {
        internal_error("fade_length >= next->size!");
    }
    for (int i = 0; i < fade_length; i++) {
        double fade = ((double)i) / fade_length;
        samples[size - fade_length + i] = (1 - fade) * samples[i] + fade * next->samples[i];
    }
}

void wav::volume(double scale) {
    for (int i = 0; i < size; i++) {
        samples[i] *= scale;
    }
}

wav2::wav2(wav* source) {
    deltas = nullptr;
    samples = source->samples;
    if (source->size > 64000) {
        internal_error("wav2 size > 64000!");
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

short wav2::get_next_sample(double dt) {
    playback_index += dt;
    if (playback_index >= size) {
        playback_index -= size;
    }
    int whole = (int)(playback_index);
    double fraction = playback_index - whole;
    return samples[whole] + (short)(deltas[whole] * fraction);
}
