#ifndef PIC8_H
#define PIC8_H

// Ezt nyugodtan ki lehet venni, ha nem jonnek hibauzenetek:
#define PIC8TEST

typedef unsigned char* puchar;

// Ha igaz, akkor lefoglalt kepek minden masodik sora paratlan cimen
// kezdodik. LGR file beolvasas es betolthattereket alatt aktiv:
extern int Paratlankepsorok;

class pic8;

void blit8(pic8* dest, pic8* source, int x = 0, int y = 0, int x1 = -10000, int y1 = -10000,
           int x2 = -10000, int y2 = -10000);

class pic8 {
  public:
    // Baratok:
    friend void blit8(pic8* dest, pic8* source, int x, int y, int x1, int y1, int x2, int y2);
    friend void spriteosit(pic8* ppic, int index);
    friend void spriteosit(pic8* ppic);

  protected:
    // Sajat:
    int allocate(long w, long h);
    void spr_open(const char* filename, FILE* h);
    int spr_save(const char* filename, FILE* h);
    void pcx_open(const char* filename, FILE* h = NULL);
    int pcx_save(const char* filename, unsigned char* pal);

  public: // Csak gany miatt public!
    int width, height;
    puchar* rows;

  private:
    puchar* szegmuttomb;
    long szegnum;
    // Sprite reszek:
    unsigned char* transparency_data;
    unsigned short transparency_data_length;

  public:
    int success;
    pic8(int w, int h);
    pic8(const char* filename, FILE* h = NULL);
    ~pic8(void);
    // VOX, SPR es PCX-et ismeri:
    int save(const char* filename, unsigned char* pal = NULL, FILE* h = NULL);
    int get_width(void);
    int get_height(void);
    void ppixel(int x, int y, unsigned char index);
    unsigned char gpixel(int x, int y);
#ifdef TEST
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
int get_pcx_pal(const char* filename, unsigned char* pal);
int get_pcx_pal(const char* filename, ddpal** sdl_pal);

void blit_scale8(pic8* dest, pic8* source, int x1, int y1, int x2, int y2);
void blit_scale8(pic8* dest, pic8* source);

#endif
