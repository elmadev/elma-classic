#include "ALL.H"

// Ez igaz, ha editorban van vezerles:
int InEditor = 0;

unsigned char DIALOG_PALETTE_ID = 4;
unsigned char DIALOG_BORDER_PALETTE_ID = 0;
unsigned char BUTTON_PALETTE_ID = 8;

void render_box(pic8* pic, int x1, int y1, int x2, int y2, unsigned char fill_id,
                unsigned char border_id) {
    pic->fill_box(x1, y1, x2, y2, fill_id);
    pic->line(x1, y1, x2, y1, border_id);
    pic->line(x1, y2, x2, y2, border_id);
    pic->line(x1, y1, x1, y2, border_id);
    pic->line(x2, y1, x2, y2, border_id);
}

void render_box(pic8* pic, box bx, unsigned char fill_id, unsigned char border_id) {
    render_box(pic, bx.x1, bx.y1, bx.x2, bx.y2, fill_id, border_id);
}

int is_in_box(int x, int y, box bx) {
    if (x >= bx.x1 && x <= bx.x2 && y >= bx.y1 && y <= bx.y2) {
        return 1;
    } else {
        return 0;
    }
}

int dialog(const char* text1, const char* text2, const char* text3, const char* text4,
           const char* text5, const char* text6, const char* text7, const char* text8,
           const char* text9, const char* text10, const char* text11, const char* text12,
           const char* text13, const char* text14, const char* text15, const char* text16,
           const char* text17, const char* text18, const char* text19, const char* text20,
           const char* text21, const char* text22, const char* text23, const char* text24) {
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
    const char* button_array[30];
    for (int i = 0; i < 24; i++) {
        button_array[i] = NULL;
    }
    int button_length = 0;
    for (int i = 0; i < text_length; i++) {
        if (strcmp(text_array[i], "GOMBOK") == 0) {
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
    if (button_length == 0) {
        button_length = 1;
        button_array[0] = "OK";
    }

    int immediately_return = 0;
    if (button_length == 1 && strcmpi(button_array[0], "VISSZATER") == 0) {
        button_length = 0;
        immediately_return = 1;
    }

    if (text_length < 1) {
        internal_error("dialog text_length < 1");
    }

    // Beallitottuk tomboket!!!

    if (InEditor) {
        push(); // Leszedjuk egeret keprol
    } else {
        Moux = 320;
        Mouy = 240;
    }

    invalidateegesz();

    int width = 0;
    for (int i = 0; i < text_length; i++) {
        if (width < Pabc2->len(text_array[i]) + 16) {
            width = Pabc2->len(text_array[i]) + 16;
        }
    }

    int dy = 20;
    int height = text_length * dy + 50;
    int x1 = 320 - width / 2;
    int y1 = 240 - height / 2;
    int x2 = 320 + width / 2;
    int y2 = 240 + height / 2;
    render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
    for (int i = 0; i < text_length; i++) {
        Pabc2->write_centered(BufferMain, 320, y1 + 22 + i * dy, text_array[i]);
    }

    // Kirajzoljuk gombokat is:
    int button_array_x1[15], button_array_x2[15], button_y1 = y2 - 30, button_y2 = y2 - 10;
    int button_dx = 80;
    if (button_length == 1) {
        button_array_x1[0] = 320;
    }
    if (button_length == 2) {
        button_array_x1[0] = 320 - button_dx / 2;
        button_array_x1[1] = 320 + button_dx / 2;
    }
    if (button_length == 3) {
        button_array_x1[0] = 320 - button_dx;
        button_array_x1[1] = 320;
        button_array_x1[2] = 320 + button_dx;
    }
    if (button_length > 3) {
        internal_error("dialog button_length > 3");
    }
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

    if (immediately_return) {
        if (!InEditor) {
            push(); // Leszedjuk egeret keprol
        }
        return 0;
    }

    // Most figyeljuk user mit csinal:
    // Azert adunk egy erteket, hogy ha lenyomassal jutott
    // dialogba, egyszer fel kell engednie mielott ujra
    // lenyomasnak erzekelne:
    left_mouse_clicked();
    right_mouse_clicked();
    while (1) {
        while (mk_kbhit()) {
            int c = mk_getextchar();
            if (button_length == 1) {
                if (c == MK_ESC || c == MK_ENTER) {
                    if (!InEditor) {
                        push(); // Leszedjuk egeret keprol
                    }
                    return 0;
                }
            }
            for (int i = 0; i < button_length; i++) {
                if (c == tolower(*(button_array[i]))) {
                    if (!InEditor) {
                        push(); // Leszedjuk egeret keprol
                    }
                    return i;
                }
            }
        }
        // Megnezzuk eger gombok lemenetelet:
        if (right_mouse_clicked()) {
            if (button_length == 1) {
                if (!InEditor) {
                    push(); // Leszedjuk egeret keprol
                }
                return 0;
            }
        }

        if (left_mouse_clicked()) {
            int mouse_x = 0, mouse_y = 0;
            get_mouse_position(&mouse_x, &mouse_y);
            push();
            Moux = mouse_x;
            Mouy = mouse_y;
            pop();
            // Most nezzuk meg, hogy hova kattintott:
            for (int i = 0; i < button_length; i++) {
                if (mouse_x >= button_array_x1[i] && mouse_x <= button_array_x2[i] &&
                    mouse_y >= button_y1 && mouse_y <= button_y2) {
                    if (!InEditor) {
                        push(); // Leszedjuk egeret keprol
                    }
                    return i;
                }
            }
            continue;
        }
        int mouse_x = 0, mouse_y = 0;
        get_mouse_position(&mouse_x, &mouse_y);
        if (mouse_x == Moux && mouse_y == Mouy) {
            continue;
        }
        push();
        Moux = mouse_x;
        Mouy = mouse_y;
        pop();
    }
}
