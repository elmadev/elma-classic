#ifndef KISKEP_H
#define KISKEP_H

// Egy szegmensben tarolja kepeket, maximalis meretei 256-nal kissebbek, es
// sorok pont 256 byte-onkent kovetkeznek!

class kiskep {
  public:
    unsigned char lyuk;
    unsigned char* tomb;
    int xsize, ysize;
    long hossz;
    // Ha megadjuk ppic-t, akkor azt o deleteli:
    kiskep(const char* pic8nev, pic8* ppic = NULL);
    kiskep(int xsize, int ysize);
    static kiskep* from_bmp(const char* filename);
    ~kiskep(void);
};

#endif
