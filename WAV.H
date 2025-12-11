
// enum { W_MONO = 1, W_LEFT, W_RIGHT };

class wav {
    void lefoglal(void); // allocate based on size
  public:
    signed short* tomb;
    unsigned size;

    wav(unsigned sizepar);
    // maxamplitudo varies between 0 and 1:
    wav(char* nev, double maxamplitudo, int start = -1, int ucsoutani = -1);
    void save(char* nev);
    // manipulation:

    // insert loop from back to front:
    void loopol(int n);
    // put front of pwav to back:
    void vegereilleszt(wav* pwav, int n);
    void hangero(double szorzo);
};

class wav2 {
    short *tomb, *mtomb;
    unsigned long ulsize, ulfuto;
    unsigned short *plow, *phigh;

  public:
    wav2(wav* pwav);
    void reset(int fazis = -1);
    // dt 65536 tight: (szoros = tight dont undd this)
    short getnextsample(long dt);
};
