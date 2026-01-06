#ifndef KISKEP_H
#define KISKEP_H

class pic8;

// Egy szegmensben tarolja kepeket, maximalis meretei 256-nal kissebbek, es
// sorok pont 256 byte-onkent kovetkeznek!

class affine_pic {
  public:
    unsigned char transparency;
    unsigned char* pixels;
    int width, height;
    long length;
    // Ha megadjuk ppic-t, akkor azt o deleteli:
    affine_pic(const char* filename, pic8* pic = NULL);
    ~affine_pic(void);
};

#endif
