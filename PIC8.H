
// Ezt nyugodtan ki lehet venni, ha nem jonnek hibauzenetek:
#define PIC8TEST

typedef unsigned char* puchar;

// Ha igaz, akkor lefoglalt kepek minden masodik sora paratlan cimen
// kezdodik. LGR file beolvasas es betolthattereket alatt aktiv:
extern int Paratlankepsorok;

class pic8;

void blt8(pic8* pd, pic8* ps, int x = 0, int y = 0, int x1 = -10000, int y1 = -10000,
          int x2 = -10000, int y2 = -10000);

class pic8 {
  public:
    // Baratok:
    friend void blt8(pic8* pd, pic8* ps, int x, int y, int x1, int y1, int x2, int y2);
    // friend void blt8fizrefejjelle( pic8* ps );
    friend void spriteosit(pic8* ppic, int szin);
    friend void spriteosit(pic8* ppic);
    friend void setmode8_l(int res);

  protected:
    // Sajat:
    int fizkep;
    int nemdestrukt;
    int lefoglal(long xsize, long ysize);
    void spritebeolvas(char* nev, FILE* h);
    int spritesave(char* nev, FILE* h);
    void pcxbeolvas(char* nev, FILE* h = NULL);
    int pcxsave(char* nev, unsigned char* pal);

  public: // Csak gany miatt public!
    int xsize, ysize;
    puchar* sormuttomb;

  private:
    puchar* szegmuttomb;
    long szegnum;
    // Sprite reszek:
    unsigned char* sprite;
    unsigned short spritehossz;

  public:
    int success;
    pic8(int xsize, int ysize);
    // Mar meglevo tombre epiti ra pic8-at, destruktor nem torli:
    pic8(int xsize, int ysize, unsigned char** ppc);
    pic8(char* nev, FILE* h = NULL);
    ~pic8(void);
    // VOX, SPR es PCX-et ismeri:
    int save(char* nev, unsigned char* pal = NULL, FILE* h = NULL);
    int getxsize(void);
    int getysize(void);
    void ppixel(int x, int y, unsigned char szin);
    unsigned char gpixel(int x, int y);
#ifdef TEST
    unsigned char* getptr(int y);
#else
    unsigned char* getptr(int y) { return sormuttomb[y]; }
#endif
    void fillbox(int x1, int y1, int x2, int y2, unsigned char szin);
    void fillbox(unsigned char szin);
    void line(int x1, int y1, int x2, int y2, unsigned char szin);
    void fuggszegmens_look(int x, int y, int size, unsigned char* lookup);
    void vizszegmens_look(int x, int y, int size, unsigned char* lookup);

    void keszitbelsot(int x1, int y1, int x2, int y2, pic8* ppic);
};

// Fizikai kepernyore mutat:
// extern pic8* Pscr8;

class ddpal;

unsigned char* spriteadat8(pic8* pmask, unsigned char szin, unsigned short* pspritehossz);
int pcxtopal(char* nev, unsigned char* pal);
int pcxtopal(char* nev, ddpal** ppddpal);

void mintavetel8(pic8* pdest, pic8* psour, int x1, int y1, int x2, int y2);
void mintavetel8(pic8* pdest, pic8* psour);

// void lassufizre( pic8* ppic, ddpal* ppal );
