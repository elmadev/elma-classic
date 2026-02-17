#include "menu_dialog.h"
#include "keys.h"
#include "main.h"
#include "menu_pic.h"
#include <cstring>

// Display the menu with the provided text, then return the key pressed by the user.
int menu_dialog(const char* text1, const char* text2, const char* text3, const char* text4,
                const char* text5, const char* text6) {
    // Count the number of entries
    constexpr int MENU_DIALOG_MAX_LENGTH = 6;
    const char* text_array[MENU_DIALOG_MAX_LENGTH + 1] = {text1, text2, text3,  text4,
                                                          text5, text6, nullptr};
    int text_count = 0;
    while (text_array[text_count] && text_count < MENU_DIALOG_MAX_LENGTH) {
        text_count += 1;
    }
    if (text_count <= 0) {
        internal_error("menu_dialog text_count <= 0!");
    }

    // Render and prompt for key input
    menu_pic menu;
    int dy = 40;
    int y0 = 240 - (text_count - 1) / 2 * dy;
    for (int i = 0; i < text_count; i++) {
        menu.add_line_centered(text_array[i], 320, y0 + i * dy);
    }
    empty_keypress_buffer();
    while (true) {
        if (has_keypress()) {
            Keycode c = get_keypress();
            return c;
        }
        menu.render();
    }
}
