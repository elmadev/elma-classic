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
void seteditorpal(); // Pal_editor-t hozza letre es allitja be

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

constexpr unsigned char Menulineszin = 0;
constexpr unsigned char DIALOG_BORDER_PALETTE_ID = 0;
constexpr unsigned char Menuszin = 1;
constexpr unsigned char Menu_nyomottszin = 2;
constexpr unsigned char Hatterindex = 3;
constexpr unsigned char DIALOG_PALETTE_ID = 4;
constexpr unsigned char EditorPalette_LevelNameDialog = 4;
constexpr unsigned char EditorPalette_LevelNameDialogBorder = 5;
constexpr unsigned char EditorPalette_List = 6;
constexpr unsigned char EditorPalette_ListSelected = 7;
constexpr unsigned char BUTTON_PALETTE_ID = 8;
constexpr unsigned char Feherszin = 9;

extern int Moux, Mouy;
void push(); // levesz
void pop();  // feltesz
void update_and_draw_cursor();

void invalidate();
void invalidateegesz();

void editor();

void toolhelp(const char* text = nullptr);

#define SZ_ZOOMVONAL (90)

void kispritenev(const char* kepnev, const char* texturanev, const char* maszknev, int tav,
                 Clipping hatarol);

#endif
