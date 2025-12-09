// Uj integralt editor h-ja:

extern topol* Ptop;
// extern pic8* Editbuffer; Ezentul ez Buffsima
// extern pic8* Editbuffer2;  Ezentul ez Buffgolyo

extern ddpal* Pal_editor;
void seteditorpal(void); // Pal_editor-t hozza letre es allitja be

extern abc8 *Pabc1, *Pabc2;

extern gyuru* Pgy;
extern int K;
extern int Fel;

extern kerek* Pker;

extern sprite* Psp;

extern int Valtozott;

enum {
    T_MOVE = 0,
    T_ZOOMIN,
    T_CREATE_VERT,
    T_DELETE_VERT,
    T_DELETE_POLY,
    T_CREATE_FOOD,
    T_CREATE_KILLER,
    T_DELETE_KEREK,
    T_CREATE_SPRITE,
    T_DELETE_SPRITE
};

extern int Tool;

// extern char Editfilenev[20]; // "nincsnev.lev" eseten meg nincs neve

// extern unsigned char Feketeindex;
extern unsigned char Feherszin;

extern unsigned char Hatterindex;

extern int Moux, Mouy;
void push(void); // levesz
void pop(void);  // feltesz

void invalidate(void);
void invalidateegesz(void);

void editor(void);

void toolhelp(char* text = NULL);

#define SZ_ZOOMVONAL (90)

void kispritenev(char* kepnev, char* texturanev, char* maszknev, int tav, int hatarol);
