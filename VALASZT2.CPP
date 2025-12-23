#include "ALL.H"

const int LISTegykepen = 11;
const int LISTx0 = 200;
const int LISTy0 = 90;
const int LISTdy = 33;

int NavEntriesLeftMaxLength = 1;

nav_entry* NavEntriesLeft = NULL;
nav_entry NavEntriesRight[NAV_ENTRIES_RIGHT_MAX_LENGTH + 1];

void menu_nav_entries_init(void) {
    if (NavEntriesLeft) {
        hiba("98y5t4");
    }
    // Megszamolja maximalis lev es rec file-okat:
    int levszam = 0;
    finame fname;
    int done = fifirst("lev/*.lev", fname);
    while (!done) {
        done = finext(fname);
        levszam++;
    }
    ficlose();

    int recszam = 0;
    done = fifirst("rec/*.rec", fname);
    while (!done) {
        done = finext(fname);
        recszam++;
    }
    ficlose();

    int szam = levszam;
    if (szam < recszam) {
        szam = recszam;
    }

    // if( szam < MAXKEPSZAM )
    //	szam = MAXKEPSZAM;

    // Megvan max szam:
    szam += 220;

    if (szam > 40000) {
        szam = 40000;
    }

    NavEntriesLeft = new nav_entry[szam + 10];
    if (!NavEntriesLeft) {
        hiba("Nincs eleg memoria NavEntriesLeft-at lefoglalni!");
    }
    NavEntriesLeftMaxLength = szam;
    if (NavEntriesLeftMaxLength < 200) {
        hiba("87iuo");
    }
}

menu_nav::menu_nav(void) {
    entries_left = NULL;
    entries_right = NULL;
    length = 0;
    selected_index = x_left = y_entries = dy = max_visible_entries = enable_esc = 0;
    title[0] = 0;
    // Ezt o maganak beallitja, de nehanyan atdefinialjak:
    y_title = 30;
    menu = NULL;
}

menu_nav::~menu_nav(void) {
    if (entries_left) {
        delete[] entries_left;
    }
    if (entries_right) {
        delete[] entries_right;
    }
    if (menu) {
        delete menu;
    }

    entries_left = NULL;
    entries_right = NULL;
    menu = NULL;
    length = 0;
    selected_index = x_left = y_entries = dy = max_visible_entries = enable_esc = 0;
}

// Rubrikakat beveszi:
void menu_nav::setup(int len, int two_col) {
    if (entries_left) {
        hiba("menu_nav::setup entries_left!");
    }
    length = len;
    two_columns = two_col;
    if (length < 1 || length > NavEntriesLeftMaxLength) {
        hiba("menu_nav::setup nem jo length!");
    }
    if (two_columns && length > NAV_ENTRIES_RIGHT_MAX_LENGTH) {
        hiba("menu_nav::setup nem jo length (two_columns)!");
    }
    entries_left = new nav_entry[length];
    if (!entries_left) {
        hiba("memoria");
    }
    entries_right = NULL;
    if (two_columns) {
        entries_right = new nav_entry[length];
        if (!entries_right) {
            hiba("memoria");
        }
    }
    for (int i = 0; i < length; i++) {
        if (strlen(NavEntriesLeft[i]) > NAV_ENTRY_TEXT_MAX_LENGTH) {
            hiba("menu_nav::setup tul hosszu string!:", NavEntriesLeft[i]);
        }
        strcpy(entries_left[i], NavEntriesLeft[i]);
        if (two_columns) {
            if (strlen(NavEntriesRight[i]) > NAV_ENTRY_TEXT_MAX_LENGTH) {
                hiba("menu_nav::setup tul hosszu string (tab)!:", NavEntriesRight[i]);
            }
            strcpy(entries_right[i], NavEntriesRight[i]);
        }
    }
}

int CtrlAltEnabled = 0;
int CtrlAltPressed = 0;

// Visszaadja valasztott szamat, ESC eseten -1-t:
// tobbrajz eseten tobbszor is kirajzolja kezdetben (Win95):
int menu_nav::navigate(text_line* extra_lines, int extra_lines_length, int render_only) {
    if (length < 1 || max_visible_entries < 2) {
        hiba("menu_nav::navigate nem jo elejen!");
    }

    if (selected_index > length - 1) {
        selected_index = length - 1;
    }

    int felso = selected_index - max_visible_entries / 2;
    int maxfelso = length - max_visible_entries;
    if (felso > maxfelso) {
        felso = maxfelso;
    }
    if (felso < 0) {
        felso = 0;
    }

    if (menu) {
        delete menu;
    }
    menu = new menu_pic;

    mk_emptychar();
    int valtozott = 1;
    while (1) {
        while (!render_only && mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC && enable_esc) {
                return -1;
            }
            if (c == MK_ENTER) {
                // RECidomenteshez kell (mainmenu.cpp-ben is):
                int controlaltnyomva(void);
                if (CtrlAltEnabled && controlaltnyomva()) {
                    CtrlAltPressed = 1;
                }
                return selected_index;
            }
            if (c == MK_UP) {
                if (selected_index > 0) {
                    selected_index--;
                    if (selected_index < felso) {
                        felso--;
                        valtozott = 1;
                    }
                }
            }
            if (c == MK_PGUP) {
                if (selected_index > 0) {
                    selected_index -= max_visible_entries;
                    if (selected_index < 0) {
                        selected_index = 0;
                    }
                    if (selected_index < felso) {
                        felso = selected_index;
                        valtozott = 1;
                    }
                }
            }
            if (c == MK_DOWN) {
                if (selected_index < length - 1) {
                    selected_index++;
                    if (selected_index > felso + max_visible_entries - 1) {
                        felso++;
                        valtozott = 1;
                    }
                }
            }
            if (c == MK_PGDOWN) {
                if (selected_index < length - 1) {
                    selected_index += max_visible_entries;
                    if (selected_index >= length) {
                        selected_index = length - 1;
                    }
                    if (selected_index > felso + max_visible_entries - 1) {
                        felso = selected_index - max_visible_entries + 1;
                        valtozott = 1;
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
        if (valtozott) {
            valtozott = 0;
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
            for (int i = 0; i < max_visible_entries && i < length - felso; i++) {
                menu->add_line(entries_left[felso + i], x_left, y_entries + i * dy);
            }
            if (two_columns) {
                for (int i = 0; i < max_visible_entries && i < length - felso; i++) {
                    menu->add_line(entries_right[felso + i], x_right, y_entries + i * dy);
                }
            }
        }
        menu->set_helmet(x_left - 30, y_entries + (selected_index - felso) * dy);
        menu->render();
        if (render_only) {
            return 0;
        }
    }
}

void menu_nav::render(void) {
    if (!menu) {
        hiba("tegcdy");
    }
    menu->render();
}
