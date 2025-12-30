#ifndef DIALOG_H
#define DIALOG_H

// Ez igaz, ha editorban van vezerles:
extern int InEditor;

// Visszaadja, hogy hanyadik gombot nyomtak:
int dialog(const char* text1, const char* text2 = NULL, const char* text3 = NULL,
           const char* text4 = NULL, const char* text5 = NULL, const char* text6 = NULL,
           const char* text7 = NULL, const char* text8 = NULL, const char* text9 = NULL,
           const char* text10 = NULL, const char* text11 = NULL, const char* text12 = NULL,
           const char* text13 = NULL, const char* text14 = NULL, const char* text15 = NULL,
           const char* text16 = NULL, const char* text17 = NULL, const char* text18 = NULL,
           const char* text19 = NULL, const char* text20 = NULL, const char* text21 = NULL,
           const char* text22 = NULL, const char* text23 = NULL, const char* text24 = NULL);

extern unsigned char DIALOG_PALETTE_ID;
extern unsigned char DIALOG_BORDER_PALETTE_ID;
extern unsigned char BUTTON_PALETTE_ID;

struct box {
    int x1, y1, x2, y2;
};

int is_in_box(int x, int y, box bx);

void render_box(pic8* pic, int x1, int y1, int x2, int y2, unsigned char fill_id,
                unsigned char border_id);
void render_box(pic8* pic, box bx, unsigned char fill_id, unsigned char border_id);

#endif
