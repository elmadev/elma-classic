#include "editor_dialog.h"
#include "abc8.h"
#include "EDITUJ.H"
#include "keys.h"
#include "M_PIC.H"
#include "main.h"
#include "menu_pic.h"
#include "pic8.h"
#include "platform_impl.h"
#include "platform_utils.h"
#include <cctype>
#include <cstring>

bool InEditor = false;

// Draw a box with a 1-pixel border
void render_box(pic8* pic, int x1, int y1, int x2, int y2, unsigned char fill_id,
                unsigned char border_id) {
    pic->fill_box(x1, y1, x2, y2, fill_id);
    pic->line(x1, y1, x2, y1, border_id);
    pic->line(x1, y2, x2, y2, border_id);
    pic->line(x1, y1, x1, y2, border_id);
    pic->line(x2, y1, x2, y2, border_id);
}

// Draw a box with a 1-pixel border
void render_box(pic8* pic, box bx, unsigned char fill_id, unsigned char border_id) {
    render_box(pic, bx.x1, bx.y1, bx.x2, bx.y2, fill_id, border_id);
}

// Return true if the coordinates are within the specified box
bool is_in_box(int x, int y, box bx) {
    return x >= bx.x1 && x <= bx.x2 && y >= bx.y1 && y <= bx.y2;
}

// Show a dialog (editor-style).
// DIALOG_BUTTONS is a special code for the next entries to be buttons
// DIALOG_RETURN is a special code in the first button to immediately return and not wait for input
// e.g. dialog("Hello World", DIALOG_BUTTONS, "Yes", "No");
// Undocumented feature: you can press the first letter of any button to choose it as well as use
// the mouse.
// Returns the index of the selected button
int dialog(const char* text1, const char* text2, const char* text3, const char* text4,
           const char* text5, const char* text6, const char* text7, const char* text8,
           const char* text9, const char* text10, const char* text11, const char* text12,
           const char* text13, const char* text14, const char* text15, const char* text16,
           const char* text17, const char* text18, const char* text19, const char* text20,
           const char* text21, const char* text22, const char* text23, const char* text24) {
    // Count text
    const char* text_array[30] = {text1,  text2,  text3,  text4,  text5,  text6,  text7,  text8,
                                  text9,  text10, text11, text12, text13, text14, text15, text16,
                                  text17, text18, text19, text20, text21, text22, text23, text24};
    int text_length = 24;
    for (int i = 0; i < 24; i++) {
        if (!text_array[i]) {
            text_length = i;
            break;
        }
    }

    // Count buttons
    const char* button_array[30];
    for (int i = 0; i < 24; i++) {
        button_array[i] = nullptr;
    }
    int button_length = 0;
    for (int i = 0; i < text_length; i++) {
        if (strcmp(text_array[i], DIALOG_BUTTONS) == 0) {
            button_length = text_length - i - 1;
            if (button_length <= 0) {
                internal_error("dialog button_length <= 0!");
            }
            text_length = i;
            for (int j = 0; j < button_length; j++) {
                button_array[j] = text_array[text_length + j + 1];
            }
            break;
        }
    }
    // Default to "OK" if no buttons are provided
    if (button_length == 0) {
        button_length = 1;
        button_array[0] = "OK";
    }
    // No buttons option
    bool immediately_return = false;
    if (button_length == 1 && strcmpi(button_array[0], DIALOG_RETURN) == 0) {
        button_length = 0;
        immediately_return = true;
    }
    if (text_length < 1) {
        internal_error("dialog text_length < 1");
    }

    // If we are in editor, keep the current mouse position, or else center the mouse
    if (InEditor) {
        push();
    } else {
        hide_cursor();
        if (is_fullscreen()) {
            Moux = SCREEN_WIDTH / 2;
            Mouy = SCREEN_HEIGHT / 2;
            set_mouse_position(Moux, Mouy);
        }
    }
    invalidateegesz();

    // Determine max width of the dialog box
    int width = 0;
    for (int i = 0; i < text_length; i++) {
        int text_width = Pabc2->len(text_array[i]) + 16;
        if (width < text_width) {
            width = text_width;
        }
    }
    // Draw dialog box
    int dy = 20;
    int height = text_length * dy + 50;
    int x1 = SCREEN_WIDTH / 2 - width / 2;
    int y1 = SCREEN_HEIGHT / 2 - height / 2;
    int x2 = SCREEN_WIDTH / 2 + width / 2;
    int y2 = SCREEN_HEIGHT / 2 + height / 2;
    render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);

    // Draw text
    for (int i = 0; i < text_length; i++) {
        Pabc2->write_centered(BufferMain, SCREEN_WIDTH / 2, y1 + 22 + i * dy, text_array[i]);
    }

    // Determine button positions
    int button_array_x1[15];
    int button_array_x2[15];
    int button_y1 = y2 - 30;
    int button_y2 = y2 - 10;
    int button_dx = 80;
    if (button_length == 1) {
        button_array_x1[0] = SCREEN_WIDTH / 2;
    }
    if (button_length == 2) {
        button_array_x1[0] = SCREEN_WIDTH / 2 - button_dx / 2;
        button_array_x1[1] = SCREEN_WIDTH / 2 + button_dx / 2;
    }
    if (button_length == 3) {
        button_array_x1[0] = SCREEN_WIDTH / 2 - button_dx;
        button_array_x1[1] = SCREEN_WIDTH / 2;
        button_array_x1[2] = SCREEN_WIDTH / 2 + button_dx;
    }
    if (button_length > 3) {
        internal_error("dialog button_length > 3");
    }
    // Draw buttons
    for (int i = 0; i < button_length; i++) {
        int button_width = Pabc2->len(button_array[i]) + 10;
        button_array_x2[i] = button_array_x1[i] + button_width / 2;
        button_array_x1[i] = button_array_x1[i] - button_width / 2;
        render_box(BufferMain, button_array_x1[i], button_y1, button_array_x2[i], button_y2,
                   BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
        Pabc2->write_centered(BufferMain, (button_array_x2[i] + button_array_x1[i]) / 2,
                              button_y1 + 14, button_array[i]);
    }

    bltfront(BufferMain);
    pop();

    // Await input
    int choice = 0;
    left_mouse_clicked();
    right_mouse_clicked();
    // Immediate exit if DIALOG_RETURN
    while (!immediately_return) {
        while (has_keypress()) {
            Keycode c = get_keypress();
            // ESC/ENTER only valid if 1 button exists
            if (button_length == 1) {
                if (c == KEY_ESC || c == KEY_ENTER) {
                    goto exit;
                }
            }
            // Allow the first letter of each button as a shortcut (undocumented)
            for (int i = 0; i < button_length; i++) {
                if (c == tolower(*(button_array[i]))) {
                    choice = i;
                    goto exit;
                }
            }
        }
        // Right mouse click only valid if 1 button exists
        if (right_mouse_clicked()) {
            if (button_length == 1) {
                goto exit;
            }
        }
        // Left mouse click a button
        if (left_mouse_clicked()) {
            update_and_draw_cursor();
            for (int i = 0; i < button_length; i++) {
                if (Moux >= button_array_x1[i] && Moux <= button_array_x2[i] && Mouy >= button_y1 &&
                    Mouy <= button_y2) {
                    choice = i;
                    goto exit;
                }
            }
            continue;
        }
        update_and_draw_cursor();
    }
exit:
    if (!InEditor) {
        push();
        show_cursor();
    }
    return choice;
}
