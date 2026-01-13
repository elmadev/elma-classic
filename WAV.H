#ifndef WAV_H
#define WAV_H

// enum { W_MONO = 1, W_LEFT, W_RIGHT };

class wav {
    void lefoglal(void); // size alapjan lefoglal!
  public:
    signed short* tomb;
    unsigned size;

    // maxamplitudo 0 es 1 kozott valtozik!:
    wav(const char* nev, double maxamplitudo, int start = -1, int ucsoutani = -1);
    // Manipulalas:

    // Elejere tesz egy loopot a vegebol:
    void loopol(int n);
    // pwav elejet rateszi vegere:
    void vegereilleszt(wav* pwav, int n);
    void hangero(double szorzo);
};

class wav2 {
    short *tomb, *mtomb;
    double ulsize, ulfuto;

  public:
    wav2(wav* pwav);
    void reset(int fazis = -1);
    // dt 65536 szoros:
    short getnextsample(double dt);
};

#endif
