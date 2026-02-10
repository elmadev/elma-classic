#include "menu_nav.h"
#include "abc8.h"
#include "eol_settings.h"
#include "fs_utils.h"
#include "main.h"
#include "M_PIC.H"
#include "platform_impl.h"
#include "platform_utils.h"
#include <algorithm>
#include <cstring>
#include <directinput/scancodes.h>

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

menu_nav_old::menu_nav_old() {
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
    search_pattern = SearchPattern::None;
    search_skip_one = false;
}

menu_nav_old::~menu_nav_old() {
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
void menu_nav_old::setup(int len, bool two_col) {
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
bool F1Pressed = false;

int menu_nav_old::calculate_visible_entries(int extra_lines_length) {
    int max_visible_entries = (SCREEN_HEIGHT - y_entries) / dy;
    if (max_visible_entries < 2) {
        max_visible_entries = 2;
    }
    return max_visible_entries;
}

// Render menu and return selected index (or -1 if Esc)
int menu_nav_old::navigate(text_line* extra_lines, int extra_lines_length, bool render_only) {
    if (length < 1) {
        internal_error("menu_nav::navigate invalid setup!");
    }

    search_input.clear();

    // Bound current selection
    if (selected_index > length - 1) {
        selected_index = length - 1;
    }

    int max_visible_entries = calculate_visible_entries(extra_lines_length);

    // Center current selection on the screen
    int view_index = selected_index - max_visible_entries / 2;
    int view_max = length - max_visible_entries;

    if (menu) {
        delete menu;
    }
    menu = new menu_pic(false);

    empty_keypress_buffer();
    bool rerender = true;
    while (true) {
        while (!render_only && has_keypress()) {
            Keycode c = get_keypress();
            if (search_handler(c)) {
                view_index = selected_index - max_visible_entries / 2;
                rerender = true;
                break;
            }
            if (c == KEY_ESC && enable_esc) {
                CtrlAltPressed = false;
                return -1;
            }
            if (c == KEY_ENTER) {
                CtrlAltPressed = is_key_down(DIK_LCONTROL) && is_key_down(DIK_LMENU);
                F1Pressed = is_key_down(DIK_F1);
                return selected_index;
            }
            if (c == KEY_UP) {
                selected_index--;
            }
            if (c == KEY_PGUP) {
                selected_index -= max_visible_entries;
            }
            if (c == KEY_DOWN) {
                selected_index++;
            }
            if (c == KEY_PGDOWN) {
                selected_index += max_visible_entries;
            }
        }

        // Limit selected index to valid values
        if (selected_index < 0) {
            selected_index = 0;
        }
        if (selected_index >= length) {
            selected_index = length - 1;
        }
        // Update view_index and limit to valid values
        if (selected_index < view_index) {
            view_index = selected_index;
            rerender = true;
        }
        if (selected_index > view_index + max_visible_entries - 1) {
            view_index = selected_index - (max_visible_entries - 1);
            rerender = true;
        }
        if (view_index > view_max) {
            view_index = view_max;
            rerender = true;
        }
        if (view_index < 0) {
            view_index = 0;
            rerender = true;
        }

        // Rerender screen only if updated menu position
        if (rerender) {
            rerender = false;
            menu->clear();

            // Fixed-position extra text lines
            for (int i = 0; i < extra_lines_length; i++) {
                if (extra_lines[i].text.starts_with(MENU_CENTER_TEXT)) {
                    menu->add_line_centered(
                        extra_lines[i].text.substr(sizeof(MENU_CENTER_TEXT) - 1), extra_lines[i].x,
                        extra_lines[i].y);
                } else {
                    menu->add_line(extra_lines[i].text, extra_lines[i].x, extra_lines[i].y);
                }
            }

            // Title
            if (!search_input.empty()) {
                std::string search_title = title;
                search_title.append(": ");
                search_title.append(search_input);
                menu->add_line_centered(search_title.c_str(), 320, y_title);
            } else {
                menu->add_line_centered(title, 320, y_title);
            }

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

void menu_nav_old::render() {
    if (!menu) {
        internal_error("menu_nav::render !menu");
    }
    menu->render();
}

nav_entry* menu_nav_old::entry_left(int index) { return &entries_left[index]; }

static bool accept_search_input() {
    if (EolSettings->lctrl_search()) {
        return is_key_down(DIK_LCONTROL);
    }

    return true;
}

static size_t common_prefix_len(const char* a, const char* b) {
    size_t n = 0;
    for (;; ++a, ++b, ++n) {
        unsigned char ca = std::tolower((unsigned char)*a);
        unsigned char cb = std::tolower((unsigned char)*b);

        if (ca != cb || ca == 0) {
            return n;
        }
    }
}

bool menu_nav_old::search_handler(int code) {
    if (search_pattern == SearchPattern::None) {
        return false;
    }

    if (code == KEY_BACKSPACE) {
        if (!search_input.empty()) {
            search_input.pop_back();
        }
    } else if (code == KEY_ESC) {
        if (!search_input.empty()) {
            search_input.clear();
        } else {
            return false;
        }
    } else if (accept_search_input() && MenuFont->has_char(code)) {
        if (search_input.size() < MAX_FILENAME_LEN) {
            search_input.push_back(code);
        }
    } else {
        return false;
    }

    if (search_input.empty()) {
        return true;
    }

    nav_entry* begin = entries_left;
    nav_entry* end = entries_left + length;
    if (search_skip_one) {
        ++begin;
    }

    switch (search_pattern) {
    case SearchPattern::Sorted: {
        // Find the entry
        nav_entry* match = std::lower_bound(
            begin, end, search_input.c_str(),
            [](const nav_entry& entry, const char* k) { return strcmpi(entry, k) < 0; });
        selected_index = match - entries_left;

        if (selected_index != length && selected_index > 0 &&
            strnicmp(*match, search_input.c_str(), search_input.length()) != 0) {
            size_t a = common_prefix_len(search_input.c_str(), entries_left[selected_index]);
            size_t b = common_prefix_len(search_input.c_str(), entries_left[selected_index - 1]);
            // Use the previous entry if it has a longer common prefix
            if (b >= a) {
                selected_index -= 1;
            }
        }

        break;
    }
    case SearchPattern::Internals: {
        // Try to jump via number input
        int index_search = -1;
        try {
            index_search = std::stoi(search_input);
        } catch (...) {
        }

        if (index_search >= 0) {
            selected_index = index_search;
            break;
        }

        // Try to find exact match
        for (int i = 0; i < length; ++i) {
            char* text = entries_left[i];
            // Skip the number prefix
            if (i >= 1) {
                text += 2;
            }
            if (i >= 10) {
                text++;
            }

            if (strnicmp(text, search_input.c_str(), search_input.size()) == 0) {
                selected_index = i;
                break;
            }
        }
        break;
    }
    case SearchPattern::None:
        internal_error("search_handler() SearchPattern::None reached!");
        break;
    }

    return true;
}

menu_nav::menu_nav(std::string title)
    : menu(std::make_unique<menu_pic>(false)),
      title(std::move(title)) {
    entries.reserve(8);
    selected_index = 0;
    x_left = 200;
    y_entries = 90;
    dy = 33;
    enable_esc = true;
    y_title = 30;
    search_pattern = SearchPattern::None;
    search_skip = 0;
}

void menu_nav::add_row(std::string left, std::string right, nav_func handler) {
    entries.emplace_back(std::move(left), std::move(right), std::move(handler));
}

void menu_nav::add_overlay(std::string text, int x, int y, OverlayAlignment alignment) {
    overlays.emplace_back(std::move(text), x, y, alignment);
}

int menu_nav::calculate_visible_entries() {
    int max_visible_entries = (SCREEN_HEIGHT - y_entries) / dy;
    if (max_visible_entries < 2) {
        max_visible_entries = 2;
    }
    return max_visible_entries;
}

// Render menu and return selected index (or -1 if Esc)
int menu_nav::prompt_choice(bool render_only) {
    if (row_count() < 1) {
        internal_error("menu_nav::prompt_choice no rows!");
    }

    search_input.clear();

    // Bound current selection
    selected_index = std::min(selected_index, (int)row_count() - 1);

    int max_visible_entries = calculate_visible_entries();

    // Center current selection on the screen
    int view_index = selected_index - max_visible_entries / 2;
    int view_max = row_count() - max_visible_entries;

    empty_keypress_buffer();
    bool rerender = true;
    while (true) {
        while (!render_only && has_keypress()) {
            Keycode c = get_keypress();
            if (search_handler(c)) {
                view_index = selected_index - max_visible_entries / 2;
                rerender = true;
                break;
            }
            if (c == KEY_ESC && enable_esc) {
                CtrlAltPressed = false;
                return -1;
            }
            if (c == KEY_ENTER) {
                CtrlAltPressed = is_key_down(DIK_LCONTROL) && is_key_down(DIK_LMENU);
                F1Pressed = is_key_down(DIK_F1);
                return selected_index;
            }
            if (c == KEY_UP) {
                selected_index--;
            }
            if (c == KEY_PGUP) {
                selected_index -= max_visible_entries;
            }
            if (c == KEY_DOWN) {
                selected_index++;
            }
            if (c == KEY_PGDOWN) {
                selected_index += max_visible_entries;
            }
        }

        // Limit selected index to valid values
        selected_index = std::max(selected_index, 0);
        selected_index = std::min(selected_index, (int)row_count() - 1);
        // Update view_index and limit to valid values
        if (selected_index < view_index) {
            view_index = selected_index;
            rerender = true;
        }
        if (selected_index > view_index + max_visible_entries - 1) {
            view_index = selected_index - (max_visible_entries - 1);
            rerender = true;
        }
        if (view_index > view_max) {
            view_index = view_max;
            rerender = true;
        }
        if (view_index < 0) {
            view_index = 0;
            rerender = true;
        }

        // Rerender screen only if updated menu position
        if (rerender) {
            rerender = false;
            menu->clear();

            // Overlays
            for (nav_overlay overlay : overlays) {
                switch (overlay.alignment) {
                case OverlayAlignment::Centered:
                    menu->add_line_centered(overlay.text, overlay.x, overlay.y);
                    break;
                case OverlayAlignment::Left:
                    menu->add_line(overlay.text, overlay.x, overlay.y);
                    break;
                }
            }

            // Title
            if (!search_input.empty()) {
                std::string search_title = title + ": " + search_input;
                menu->add_line_centered(search_title, 320, y_title);
            } else {
                menu->add_line_centered(title, 320, y_title);
            }

            // Only the visible menu entries
            for (int i = 0; i < max_visible_entries && i < row_count() - view_index; i++) {
                menu->add_line(entries[view_index + i].text_left, x_left, y_entries + i * dy);
            }
            for (int i = 0; i < max_visible_entries && i < row_count() - view_index; i++) {
                menu->add_line(entries[view_index + i].text_right, x_right, y_entries + i * dy);
            }
        }
        menu->set_helmet(x_left - 30, y_entries + (selected_index - view_index) * dy);
        menu->render();
        if (render_only) {
            return -1;
        }
    }
}

int menu_nav::navigate(bool render_only) {
    // Get choice from menu
    int choice = prompt_choice(render_only);
    if (choice == -1) {
        return choice;
    }

    // Run the handler
    nav_row& entry = entries[choice];
    nav_func& f = entry.handler;
    if (f) {
        f(choice, entry.text_left, entry.text_right);
    }
    return choice;
}

void menu_nav::render() { menu->render(); }

std::string& menu_nav::entry_left(int index) { return entries[index].text_left; }

bool menu_nav::search_handler(Keycode code) {
    if (search_pattern == SearchPattern::None) {
        return false;
    }

    if (code == KEY_BACKSPACE) {
        if (!search_input.empty()) {
            search_input.pop_back();
        }
    } else if (code == KEY_ESC) {
        if (!search_input.empty()) {
            search_input.clear();
        } else {
            return false;
        }
    } else if (accept_search_input() && MenuFont->has_char(code)) {
        const size_t max_len = search_pattern == SearchPattern::Internals ? 20 : MAX_FILENAME_LEN;
        if (search_input.size() < max_len) {
            search_input.push_back(code);
        }
    } else {
        return false;
    }

    if (search_input.empty()) {
        return true;
    }

    using iter = std::vector<nav_row>::iterator;
    iter begin = entries.begin() + search_skip;
    iter end = entries.end();

    switch (search_pattern) {
    case SearchPattern::Sorted: {
        // Find the entry
        iter match = std::lower_bound(begin, end, search_input.c_str(),
                                      [](const nav_row& entry, const char* k) {
                                          return strcmpi(entry.text_left.c_str(), k) < 0;
                                      });
        selected_index = match - entries.begin();

        if (selected_index != row_count() && selected_index > 0 &&
            strnicmp(match->text_left.c_str(), search_input.c_str(), search_input.length()) != 0) {
            size_t a =
                common_prefix_len(search_input.c_str(), entries[selected_index].text_left.c_str());
            size_t b = common_prefix_len(search_input.c_str(),
                                         entries[selected_index - 1].text_left.c_str());
            // Use the previous entry if it has a longer common prefix
            if (b >= a) {
                selected_index -= 1;
            }
        }

        break;
    }
    case SearchPattern::Internals: {
        // Try to jump via number input
        int index_search = -1;
        try {
            index_search = std::stoi(search_input);
        } catch (...) {
        }

        if (index_search >= 0) {
            selected_index = index_search;
            break;
        }

        // Try to find exact match
        for (int i = 0; i < row_count(); ++i) {
            const char* text = entries[i].text_left.c_str();
            // Skip the number prefix
            if (i >= 1) {
                text += 2;
            }
            if (i >= 10) {
                text++;
            }

            if (strnicmp(text, search_input.c_str(), search_input.size()) == 0) {
                selected_index = i;
                break;
            }
        }
        break;
    }
    case SearchPattern::None:
        internal_error("search_handler() SearchPattern::None reached!");
        break;
    }

    return true;
}
