#ifndef EDITUJ_H
#define EDITUJ_H

#include "sprite.h"

class abc8;
class object;
class palette;
class polygon;
class sprite;
class level;

// Uj integralt editor h-ja:

extern level* Ptop;
// extern pic8* Editbuffer; Ezentul ez BufferMain
// extern pic8* Editbuffer2;  Ezentul ez BufferBall

extern palette* Pal_editor;
void seteditorpal(void); // Pal_editor-t hozza letre es allitja be

extern abc8 *Pabc1, *Pabc2;

extern polygon* Pgy;
extern int K;
extern bool Fel;

extern object* Pker;

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
void update_and_draw_cursor();

void invalidate(void);
void invalidateegesz(void);

void editor(void);

void toolhelp(const char* text = nullptr);

#define SZ_ZOOMVONAL (90)

void kispritenev(const char* kepnev, const char* texturanev, const char* maszknev, int tav,
                 Clipping hatarol);

#endif
