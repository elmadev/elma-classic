#ifndef WAV_H
#define WAV_H

// enum { W_MONO = 1, W_LEFT, W_RIGHT };

class wav {
    void allocate(void); // size alapjan lefoglal!
  public:
    signed short* samples;
    unsigned size;

    // maxamplitudo 0 es 1 kozott valtozik!:
    wav(const char* filename, double max_volume, int start = -1, int end = -1);
    // Manipulalas:

    // Elejere tesz egy loopot a vegebol:
    void loop(int fade_length);
    // pwav elejet rateszi vegere:
    void fade(wav* next, int fade_length);
    void volume(double scale);
};

class wav2 {
    short *samples, *deltas;
    double size, playback_index;

  public:
    wav2(wav* source);
    void reset(int index = -1);
    // dt 65536 szoros:
    short get_next_sample(double dt);
};

#endif
