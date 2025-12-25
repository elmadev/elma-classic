#include "ALL.H"

const int LISTegykepen = 11;
const int LISTx0 = 200;
const int LISTy0 = 90;
const int LISTdy = 33;

int NavEntriesLeftMaxLength = 1;

nav_entry* NavEntriesLeft = nullptr;
nav_entry NavEntriesRight[NAV_ENTRIES_RIGHT_MAX_LENGTH + 1];

void menu_nav_entries_init() {
    if (NavEntriesLeft) {
        hiba("menu_nav_entries_init already called!");
    }
    // Megszamolja maximalis lev es rec file-okat:
    int level_count = 0;
    finame filename;
    bool done = fifirst("lev/*.lev", filename);
    while (!done) {
        done = finext(filename);
        level_count++;
    }
    ficlose();

    int rec_count = 0;
    done = fifirst("rec/*.rec", filename);
    while (!done) {
        done = finext(filename);
        rec_count++;
    }
    ficlose();

    int max_count = level_count;
    if (max_count < rec_count) {
        max_count = rec_count;
    }

    // if( max_count < MAXKEPSZAM )
    //	max_count = MAXKEPSZAM;

    // Megvan max szam:
    max_count += 220;

    if (max_count > 40000) {
        max_count = 40000;
    }

    NavEntriesLeft = new nav_entry[max_count + 10];
    if (!NavEntriesLeft) {
        hiba("menu_nav_entries_init out of memory!");
    }
    NavEntriesLeftMaxLength = max_count;
    if (NavEntriesLeftMaxLength < 200) {
        hiba("menu_nav_entries_init max count invalid!");
    }
}

menu_nav::menu_nav() {
    entries_left = nullptr;
    entries_right = nullptr;
    length = 0;
    selected_index = 0;
    x_left = 0;
    y_entries = 0;
    dy = 0;
    max_visible_entries = 0;
    enable_esc = false;
    title[0] = 0;
    // Ezt o maganak beallitja, de nehanyan atdefinialjak:
    y_title = 30;
    menu = nullptr;
}

menu_nav::~menu_nav() {
    if (entries_left) {
        delete[] entries_left;
    }
    if (entries_right) {
        delete[] entries_right;
    }
    if (menu) {
        delete menu;
    }

    entries_left = nullptr;
    entries_right = nullptr;
    menu = nullptr;
    length = 0;
    selected_index = 0;
    x_left = 0;
    y_entries = 0;
    dy = 0;
    max_visible_entries = 0;
    enable_esc = false;
}

// Rubrikakat beveszi:
void menu_nav::setup(int len, bool two_col) {
    if (entries_left) {
        hiba("menu_nav::setup called twice!");
    }
    length = len;
    two_columns = two_col;
    if (length < 1 || length > NavEntriesLeftMaxLength) {
        hiba("menu_nav::setup length too long!");
    }
    if (two_columns && length > NAV_ENTRIES_RIGHT_MAX_LENGTH) {
        hiba("menu_nav::setup length too long (two_columns)!");
    }
    entries_left = new nav_entry[length];
    if (!entries_left) {
        hiba("menu_nav::setup out of memory!");
    }
    entries_right = nullptr;
    if (two_columns) {
        entries_right = new nav_entry[length];
        if (!entries_right) {
            hiba("menu_nav::setup out of memory!");
        }
    }
    for (int i = 0; i < length; i++) {
        if (strlen(NavEntriesLeft[i]) > NAV_ENTRY_TEXT_MAX_LENGTH) {
            hiba("menu_nav::setup text length too long!: ", NavEntriesLeft[i]);
        }
        strcpy(entries_left[i], NavEntriesLeft[i]);
        if (two_columns) {
            if (strlen(NavEntriesRight[i]) > NAV_ENTRY_TEXT_MAX_LENGTH) {
                hiba("menu_nav::setup text length too long (two_columns)!:", NavEntriesRight[i]);
            }
            strcpy(entries_right[i], NavEntriesRight[i]);
        }
    }
}

int CtrlAltEnabled = 0;
int CtrlAltPressed = 0;

// Visszaadja valasztott szamat, ESC eseten -1-t:
// tobbrajz eseten tobbszor is kirajzolja kezdetben (Win95):
int menu_nav::navigate(text_line* extra_lines, int extra_lines_length, bool render_only) {
    if (length < 1 || max_visible_entries < 2) {
        hiba("menu_nav::navigate invalid setup!");
    }

    if (selected_index > length - 1) {
        selected_index = length - 1;
    }

    int view_index = selected_index - max_visible_entries / 2;
    int view_max = length - max_visible_entries;
    if (view_index > view_max) {
        view_index = view_max;
    }
    if (view_index < 0) {
        view_index = 0;
    }

    if (menu) {
        delete menu;
    }
    menu = new menu_pic;

    mk_emptychar();
    bool rerender = true;
    while (true) {
        while (!render_only && mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC && enable_esc) {
                return -1;
            }
            if (c == MK_ENTER) {
                // RECidomenteshez kell (mainmenu.cpp-ben is):
                if (CtrlAltEnabled && controlaltnyomva()) {
                    CtrlAltPressed = 1;
                }
                return selected_index;
            }
            if (c == MK_UP) {
                if (selected_index > 0) {
                    selected_index--;
                    if (selected_index < view_index) {
                        view_index--;
                        rerender = true;
                    }
                }
            }
            if (c == MK_PGUP) {
                if (selected_index > 0) {
                    selected_index -= max_visible_entries;
                    if (selected_index < 0) {
                        selected_index = 0;
                    }
                    if (selected_index < view_index) {
                        view_index = selected_index;
                        rerender = true;
                    }
                }
            }
            if (c == MK_DOWN) {
                if (selected_index < length - 1) {
                    selected_index++;
                    if (selected_index > view_index + max_visible_entries - 1) {
                        view_index++;
                        rerender = true;
                    }
                }
            }
            if (c == MK_PGDOWN) {
                if (selected_index < length - 1) {
                    selected_index += max_visible_entries;
                    if (selected_index >= length) {
                        selected_index = length - 1;
                    }
                    if (selected_index > view_index + max_visible_entries - 1) {
                        view_index = selected_index - max_visible_entries + 1;
                        rerender = true;
                    }
                }
            }
            // Volume allitas itt specialisan van elintezve:
            /*if( (c == MK_LEFT || c == MK_RIGHT) &&
                strcmp( entries_left[selected_index], "Sound Volume:" ) == 0 &&
                (selected_index == 3 || selected_index == 4) &&
                strcmp( entries_left[selected_index-1], "Player B:" ) == 0 ) {
                if( c == MK_LEFT ) {
                    State->soundvolume -= 0.05;
                    if( State->soundvolume < 0.0 )
                        State->soundvolume = 0.0;
                }
                if( c == MK_RIGHT ) {
                    State->soundvolume += 0.05;
                    if( State->soundvolume > 1.0 )
                        State->soundvolume = 1.0;
                }
                return selected_index;
            } */
        }
        if (rerender) {
            rerender = false;
            // Betaplalja kirajzba uj kepszoveget:
            menu->clear();
            for (int i = 0; i < extra_lines_length; i++) {
                if (strncmp(extra_lines[i].text, "*$$^&|@", 7) == 0) {
                    // Kozepre kell tenni:
                    menu->add_line_centered(&extra_lines[i].text[7], extra_lines[i].x,
                                            extra_lines[i].y);
                } else {
                    // Nem kell kozepre tenni:
                    menu->add_line(extra_lines[i].text, extra_lines[i].x, extra_lines[i].y);
                }
            }

            menu->add_line_centered(title, 320, y_title);
            for (int i = 0; i < max_visible_entries && i < length - view_index; i++) {
                menu->add_line(entries_left[view_index + i], x_left, y_entries + i * dy);
            }
            if (two_columns) {
                for (int i = 0; i < max_visible_entries && i < length - view_index; i++) {
                    menu->add_line(entries_right[view_index + i], x_right, y_entries + i * dy);
                }
            }
        }
        menu->set_helmet(x_left - 30, y_entries + (selected_index - view_index) * dy);
        menu->render();
        if (render_only) {
            return 0;
        }
    }
}

void menu_nav::render() {
    if (!menu) {
        hiba("menu_nav::render !menu");
    }
    menu->render();
}
