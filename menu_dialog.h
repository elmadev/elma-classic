#ifndef MENU_DIALOG_H
#define MENU_DIALOG_H

#define MENU_DIALOG_ONLY_ESC "ESC"

int menu_dialog(const char* text1, const char* text2 = nullptr, const char* text3 = nullptr,
                const char* text4 = nullptr, const char* text5 = nullptr,
                const char* text6 = nullptr);

#endif
