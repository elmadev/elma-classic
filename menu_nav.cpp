#include "menu_nav.h"
#include "fs_utils.h"
#include "keys.h"
#include "main.h"
#include "M_PIC.H"
#include "menu_pic.h"
#include <cstring>

int NavEntriesLeftMaxLength = 1;

nav_entry* NavEntriesLeft = nullptr;
nav_entry NavEntriesRight[NAV_ENTRIES_RIGHT_MAX_LENGTH + 1];

// Initialize the left column by counting the total number of .lev and .rec files to use as the
// maximum column length
void menu_nav_entries_init() {
    if (NavEntriesLeft) {
        internal_error("menu_nav_entries_init already called!");
    }

    // Count level files
    int level_count = 0;
    finame filename;
    bool done = find_first("lev/*.lev", filename);
    while (!done) {
        done = find_next(filename);
        level_count++;
    }
    find_close();

    // Count replay files
    int rec_count = 0;
    done = find_first("rec/*.rec", filename);
    while (!done) {
        done = find_next(filename);
        rec_count++;
    }
    find_close();

    // Get the max + 220
    int max_count = level_count;
    if (max_count < rec_count) {
        max_count = rec_count;
    }
    max_count += 220;
    if (max_count > 40000) {
        max_count = 40000;
    }

    NavEntriesLeft = new nav_entry[max_count + 10];
    if (!NavEntriesLeft) {
        internal_error("menu_nav_entries_init out of memory!");
    }
    NavEntriesLeftMaxLength = max_count;
    if (NavEntriesLeftMaxLength < 200) {
        internal_error("menu_nav_entries_init max count invalid!");
    }
}

menu_nav::menu_nav() {
    entries_left = nullptr;
    entries_right = nullptr;
    length = 0;
    selected_index = 0;
    x_left = 200;
    y_entries = 90;
    dy = 33;
    enable_esc = true;
    title[0] = 0;
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
    enable_esc = false;
}

// Load the menu with data from NavEntriesLeft and, if two columns, NavEntriesRight
void menu_nav::setup(int len, bool two_col) {
    if (entries_left) {
        internal_error("menu_nav::setup called twice!");
    }
    length = len;
    two_columns = two_col;
    if (length < 1 || length > NavEntriesLeftMaxLength) {
        internal_error("menu_nav::setup length too long!");
    }
    if (two_columns && length > NAV_ENTRIES_RIGHT_MAX_LENGTH) {
        internal_error("menu_nav::setup length too long (two_columns)!");
    }
    entries_left = new nav_entry[length];
    if (!entries_left) {
        internal_error("menu_nav::setup out of memory!");
    }
    entries_right = nullptr;
    if (two_columns) {
        entries_right = new nav_entry[length];
        if (!entries_right) {
            internal_error("menu_nav::setup out of memory!");
        }
    }
    for (int i = 0; i < length; i++) {
        if (strlen(NavEntriesLeft[i]) > NAV_ENTRY_TEXT_MAX_LENGTH) {
            internal_error("menu_nav::setup text length too long!: ", NavEntriesLeft[i]);
        }
        strcpy(entries_left[i], NavEntriesLeft[i]);
        if (two_columns) {
            if (strlen(NavEntriesRight[i]) > NAV_ENTRY_TEXT_MAX_LENGTH) {
                internal_error("menu_nav::setup text length too long (two_columns)!:",
                               NavEntriesRight[i]);
            }
            strcpy(entries_right[i], NavEntriesRight[i]);
        }
    }
}

bool CtrlAltPressed = false;

int menu_nav::calculate_visible_entries(int extra_lines_length) {
    int max_visible_entries = (SCREEN_HEIGHT - y_entries) / dy;
    if (max_visible_entries < 2) {
        max_visible_entries = 2;
    }
    // Account for extra lines as well as title line
    int max_value = MENU_MAX_LINES - extra_lines_length - 1;
    if (two_columns) {
        max_value /= 2;
    }
    if (max_visible_entries > max_value) {
        max_visible_entries = max_value;
    }
    return max_visible_entries;
}

// Render menu and return selected index (or -1 if Esc)
int menu_nav::navigate(text_line* extra_lines, int extra_lines_length, bool render_only) {
    if (length < 1) {
        internal_error("menu_nav::navigate invalid setup!");
    }

    // Bound current selection
    if (selected_index > length - 1) {
        selected_index = length - 1;
    }

    int max_visible_entries = calculate_visible_entries(extra_lines_length);

    // Center current selection on the screen
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
    menu = new menu_pic(false);

    empty_keypress_buffer();
    bool rerender = true;
    while (true) {
        while (!render_only && has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC && enable_esc) {
                CtrlAltPressed = false;
                return -1;
            }
            if (c == KEY_ENTER) {
                CtrlAltPressed = is_ctrl_alt_down();
                return selected_index;
            }
            if (c == KEY_UP) {
                if (selected_index > 0) {
                    selected_index--;
                    if (selected_index < view_index) {
                        view_index--;
                        rerender = true;
                    }
                }
            }
            if (c == KEY_PGUP) {
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
            if (c == KEY_DOWN) {
                if (selected_index < length - 1) {
                    selected_index++;
                    if (selected_index > view_index + max_visible_entries - 1) {
                        view_index++;
                        rerender = true;
                    }
                }
            }
            if (c == KEY_PGDOWN) {
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
        }
        // Rerender screen only if updated menu position
        if (rerender) {
            rerender = false;
            menu->clear();

            // Fixed-position extra text lines
            for (int i = 0; i < extra_lines_length; i++) {
                if (strncmp(extra_lines[i].text, MENU_CENTER_TEXT, sizeof(MENU_CENTER_TEXT) - 1) ==
                    0) {
                    menu->add_line_centered(&extra_lines[i].text[sizeof(MENU_CENTER_TEXT) - 1],
                                            extra_lines[i].x, extra_lines[i].y);
                } else {
                    menu->add_line(extra_lines[i].text, extra_lines[i].x, extra_lines[i].y);
                }
            }

            // Title
            menu->add_line_centered(title, 320, y_title);

            // Only the visible menu entries
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
        internal_error("menu_nav::render !menu");
    }
    menu->render();
}
