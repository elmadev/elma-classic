#ifndef AFFINE_PIC_H
#define AFFINE_PIC_H

class pic8;

class affine_pic {
  public:
    unsigned char transparency;
    unsigned char* pixels; // Every row right-padded to a width of 256
    int width;
    int height;
    affine_pic(const char* filename, pic8* pic);
    ~affine_pic();
};

#endif
