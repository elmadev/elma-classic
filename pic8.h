#ifndef PIC8_H
#define PIC8_H

#include <cstdio>

class palette;

class pic8 {
  private:
    friend void blit8(pic8* dest, pic8* source, int x, int y, int x1, int y1, int x2, int y2);
    friend void blit8(pic8* dest, pic8* source, int x, int y);
    friend pic8* lockbackbuffer_pic(bool flipped);
    friend void lockfrontbuffer_pic(bool flipped);
    friend void unlockbackbuffer_pic();

    void allocate(int w, int h);
    void spr_open(const char* filename, FILE* h);
    bool spr_save(const char* filename, FILE* h);
    void pcx_open(const char* filename, FILE* h = nullptr);
    bool pcx_save(const char* filename, unsigned char* pal);

    int width;
    int height;
    unsigned char** rows;
    unsigned char* pixels;
    unsigned char* transparency_data;
    unsigned short transparency_data_length;

  public:
    pic8(int w, int h);
    pic8(const char* filename, FILE* h = nullptr);
    static pic8* from_bmp(const char* filename);
    static pic8* resize(pic8* src, int height);
    ~pic8();
    void vertical_flip();
    bool save(const char* filename, unsigned char* pal = nullptr, FILE* h = nullptr);
    void ppixel(int x, int y, unsigned char index);
    unsigned char gpixel(int x, int y);
    int get_width() { return width; }
    int get_height() { return height; }
#ifdef DEBUG
    unsigned char* get_row(int y);
#else
    unsigned char* get_row(int y) { return rows[y]; }
#endif
    void fill_box(int x1, int y1, int x2, int y2, unsigned char index);
    void fill_box(unsigned char index);
    void line(int x1, int y1, int x2, int y2, unsigned char index);
    void subview(int x1, int y1, int x2, int y2, pic8* source);

    // Generate transparency data with a specified transparency palette index.
    void add_transparency(int transparency);
    // Generate transparency data using the top-left pixel as the transparency palette index.
    void add_transparency();
};

void blit8(pic8* dest, pic8* source, int x = 0, int y = 0);

bool get_pcx_pal(const char* filename, unsigned char* pal);
bool get_pcx_pal(const char* filename, palette** pal);

void blit_scale8(pic8* dest, pic8* source, int x1, int y1, int x2, int y2);
void blit_scale8(pic8* dest, pic8* source);

#endif
