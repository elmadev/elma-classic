#ifndef PIC8_H
#define PIC8_H

constexpr int BLIT_ALL = -10000;

class pic8;

void blit8(pic8* dest, pic8* source, int x = 0, int y = 0, int x1 = BLIT_ALL, int y1 = BLIT_ALL,
           int x2 = BLIT_ALL, int y2 = BLIT_ALL);

class pic8 {
  public:
    friend void blit8(pic8* dest, pic8* source, int x, int y, int x1, int y1, int x2, int y2);
    friend void spriteosit(pic8* ppic, int index);
    friend void spriteosit(pic8* ppic);

  protected:
    void allocate(int w, int h);
    void spr_open(const char* filename, FILE* h);
    bool spr_save(const char* filename, FILE* h);
    void pcx_open(const char* filename, FILE* h = nullptr);
    bool pcx_save(const char* filename, unsigned char* pal);

  public:
    int width;
    int height;
    unsigned char** rows;

  private:
    unsigned char* pixels;
    unsigned char* transparency_data;
    unsigned short transparency_data_length;

  public:
    pic8(int w, int h);
    pic8(const char* filename, FILE* h = nullptr);
    ~pic8();
    bool save(const char* filename, unsigned char* pal = nullptr, FILE* h = nullptr);
    int get_width();
    int get_height();
    void ppixel(int x, int y, unsigned char index);
    unsigned char gpixel(int x, int y);
#ifdef DEBUG
    unsigned char* get_row(int y);
#else
    unsigned char* get_row(int y) { return rows[y]; }
#endif
    void fill_box(int x1, int y1, int x2, int y2, unsigned char index);
    void fill_box(unsigned char index);
    void line(int x1, int y1, int x2, int y2, unsigned char index);
    void vertical_line(int x, int y, int size, unsigned char* lookup);
    void horizontal_line(int x, int y, int size, unsigned char* lookup);
    void subview(int x1, int y1, int x2, int y2, pic8* source);
};

class ddpal;

unsigned char* spriteadat8(pic8* pmask, unsigned char szin, unsigned short* pspritehossz);
bool get_pcx_pal(const char* filename, unsigned char* pal);
bool get_pcx_pal(const char* filename, ddpal** sdl_pal);

void blit_scale8(pic8* dest, pic8* source, int x1, int y1, int x2, int y2);
void blit_scale8(pic8* dest, pic8* source);

#endif
