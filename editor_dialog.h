#ifndef EDITOR_DIALOG_H
#define EDITOR_DIALOG_H

class pic8;

constexpr unsigned char DIALOG_PALETTE_ID = 4;
constexpr unsigned char DIALOG_BORDER_PALETTE_ID = 0;
constexpr unsigned char BUTTON_PALETTE_ID = 8;

extern bool InEditor;

#define DIALOG_BUTTONS "GOMBOK"
#define DIALOG_RETURN "VISSZATER"

int dialog(const char* text1, const char* text2 = nullptr, const char* text3 = nullptr,
           const char* text4 = nullptr, const char* text5 = nullptr, const char* text6 = nullptr,
           const char* text7 = nullptr, const char* text8 = nullptr, const char* text9 = nullptr,
           const char* text10 = nullptr, const char* text11 = nullptr, const char* text12 = nullptr,
           const char* text13 = nullptr, const char* text14 = nullptr, const char* text15 = nullptr,
           const char* text16 = nullptr, const char* text17 = nullptr, const char* text18 = nullptr,
           const char* text19 = nullptr, const char* text20 = nullptr, const char* text21 = nullptr,
           const char* text22 = nullptr, const char* text23 = nullptr,
           const char* text24 = nullptr);

struct box {
    int x1, y1, x2, y2;
};

bool is_in_box(int x, int y, box bx);

void render_box(pic8* pic, int x1, int y1, int x2, int y2, unsigned char fill_id,
                unsigned char border_id);
void render_box(pic8* pic, box bx, unsigned char fill_id, unsigned char border_id);

#endif
