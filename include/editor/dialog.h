#ifndef EDITOR_DIALOG_H
#define EDITOR_DIALOG_H

class pic8;

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

// Use dialog from menu
void dialog_switch_palette();
// Restore dialog from menu
void dialog_restore_palette();

#endif
