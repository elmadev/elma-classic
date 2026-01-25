#ifndef WAV_H
#define WAV_H

// 11,025 Hz 16-bit mono audio
class wav {
    void allocate();

  public:
    short* samples;
    unsigned size;

    // Open a wav file. The volume will be scaled based on max_volume (0.0-1.0)
    // You can skip part of the audio file by setting start and end.
    wav(const char* filename, double max_volume, int start = 0, int end = -1);

    // Loop an audio file.
    // Cut out the last n samples from the file and fade them into the first n samples.
    void loop(int fade_length);
    // Fade between two audio files.
    // The first n samples of "next" are faded into the end of "this".
    // After, you need to manually cut out the first n samples of next.
    void fade(wav* next, int fade_length);
    // Multiply the volume
    void volume(double scale);
};

// Contains 16-bit looping audio played back at variable speed (used for throttle engine sound)
class wav2 {
    short *samples, *deltas;
    double size, playback_index;

  public:
    // Create a wav2 by copying the data from a wav
    wav2(wav* source);
    // Select the playback index
    void reset(double index = 0.0);
    // Advance time by "dt" samples and return the audio sample at this point in time
    short get_next_sample(double dt);
};

#endif
