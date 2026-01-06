#ifndef KISKEP_H
#define KISKEP_H

class pic8;

class affine_pic {
  public:
    unsigned char transparency;
    unsigned char* pixels; // Every row right-padded to a width of 256
    int width, height;
    long length;
    affine_pic(const char* filename, pic8* pic = NULL);
    ~affine_pic(void);
};

#endif
