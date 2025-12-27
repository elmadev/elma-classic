#include "ALL.H"

#define MENU_DIALOG_MAX_LENGTH (6)

// Visszaadja lenyomott billentyut:
int menu_dialog(const char* text1, const char* text2, const char* text3, const char* text4,
                const char* text5, const char* text6) {
    int text_count;
    const char* text_array[MENU_DIALOG_MAX_LENGTH + 1] = {text1, text2, text3, text4,
                                                          text5, text6, NULL};
    for (text_count = 0; text_count < 6; text_count++) {
        if (!text_array[text_count]) {
            break;
        }
    }

    int esc_only = 0;
    if (strcmp(text_array[0], "ESC") == 0) {
        esc_only = 1;
        for (int i = 0; i < text_count; i++) {
            text_array[i] = text_array[i + 1];
        }
        text_count--;
    }

    menu_pic menu;
    int dy = 40;
    int y0 = 240 - (text_count - 1) / 2 * dy;
    for (int i = 0; i < text_count; i++) {
        menu.add_line_centered(text_array[i], 320, y0 + i * dy);
    }
    mk_emptychar();
    while (1) {
        if (mk_kbhit()) {
            int c = mk_getextchar();
            if (esc_only) {
                if (c == MK_ESC) {
                    return c;
                }
            } else {
                return c;
            }
        }
        menu.render();
    }
}
