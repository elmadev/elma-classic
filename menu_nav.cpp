#include "menu_nav.h"
#include "abc8.h"
#include "eol_settings.h"
#include "fs_utils.h"
#include "keys.h"
#include "main.h"
#include "M_PIC.H"
#include "platform_impl.h"
#include "platform_utils.h"
#include <algorithm>
#include <cstring>
#include <directinput/scancodes.h>
#include <numeric>

menu_nav::menu_nav(std::string title)
    : menu(std::make_unique<menu_pic>(false)),
      title(std::move(title)) {
    entries.reserve(8);
    selected_index = 0;
    x_left = 200;
    y_entries = 90;
    dy = 33;
    x_right = 240; // dummy value
    enable_esc = true;
    y_title = 30;
    search_pattern = SearchPattern::None;
    search_skip = 0;
    max_search_len = MAX_FILENAME_LEN;
}

void menu_nav::add_row(std::string left, std::string right, nav_func handler) {
    entries.emplace_back(std::move(left), std::move(right), std::move(handler));
}

void menu_nav::add_overlay(std::string text, int x, int y, OverlayAlignment alignment) {
    overlays.emplace_back(std::move(text), x, y, alignment);
}

void menu_nav::sort_rows() {
    std::sort(entries.begin() + search_skip, entries.end(), [](nav_row& a, nav_row& b) {
        return strcmpi(a.text_left.c_str(), b.text_left.c_str()) < 0;
    });
}

void menu_nav::select_row(const std::string& left) {
    std::vector<nav_row>::iterator it = find_if(
        entries.begin(), entries.end(), [left](const nav_row& c) { return left == c.text_left; });
    selected_index = it - entries.begin();
    if (selected_index == row_count()) {
        // Not found
        selected_index = 0;
    }
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

    if (search_pattern != SearchPattern::Filter) {
        search_input.clear();
    }

    // Initialize filter_indices to identity mapping
    filter_indices.resize(entries.size());
    std::iota(filter_indices.begin(), filter_indices.end(), 0);

    if (!search_input.empty()) {
        update_filter();
    }

    // Bound current selection
    selected_index = std::min(selected_index, (int)filter_indices.size() - 1);

    int max_visible_entries = calculate_visible_entries();

    // Center current selection on the screen
    int view_index = selected_index - max_visible_entries / 2;
    int view_max = (int)filter_indices.size() - max_visible_entries;

    empty_keypress_buffer();
    bool rerender = true;
    while (true) {
        handle_events();
        if (!render_only) {
            while (has_text_input()) {
                char c = pop_text_input();
                if (search_handler_text(c)) {
                    view_index = selected_index - max_visible_entries / 2;
                    rerender = true;
                }
            }
            if (was_key_down(DIK_BACK) && search_handler_backspace()) {
                view_index = selected_index - max_visible_entries / 2;
                rerender = true;
            }
            if (was_key_just_pressed(DIK_ESCAPE)) {
                if (search_pattern != SearchPattern::None && !search_input.empty()) {
                    search_input.clear();
                    update_search();
                    view_index = selected_index - max_visible_entries / 2;
                    rerender = true;
                } else if (enable_esc) {
                    return -1;
                }
            }
            if (was_key_just_pressed(DIK_RETURN) && !filter_indices.empty()) {
                return filter_indices[selected_index];
            }
            if (!filter_indices.empty()) {
                if (was_key_down(DIK_UP)) {
                    selected_index--;
                }
                if (was_key_down(DIK_DOWN)) {
                    selected_index++;
                }
                if (was_key_down(DIK_PRIOR)) {
                    selected_index -= max_visible_entries;
                }
                if (was_key_down(DIK_NEXT)) {
                    selected_index += max_visible_entries;
                }
                int wheel = get_mouse_wheel_delta();
                if (wheel != 0) {
                    selected_index -= wheel;
                }
            }
        }

        // Recalculate view_max (filter may have changed the count)
        view_max = (int)filter_indices.size() - max_visible_entries;

        // Limit selected index to valid values
        if (!filter_indices.empty()) {
            selected_index = std::max(selected_index, 0);
            selected_index = std::min(selected_index, (int)filter_indices.size() - 1);
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
            int visible = (int)filter_indices.size() - view_index;
            for (int i = 0; i < max_visible_entries && i < visible; i++) {
                menu->add_line(entries[filter_indices[view_index + i]].text_left, x_left,
                               y_entries + i * dy);
                menu->add_line(entries[filter_indices[view_index + i]].text_right, x_right,
                               y_entries + i * dy);
            }
        }
        if (filter_indices.empty()) {
            menu->set_helmet(x_left - 30, -100);
        } else {
            menu->set_helmet(x_left - 30, y_entries + (selected_index - view_index) * dy);
        }
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

static bool accept_search_input() {
    if (EolSettings->lctrl_search()) {
        return is_key_down(DIK_LCONTROL);
    }

    return true;
}

static bool contains_ci(const std::string& haystack, const std::string& needle) {
    return std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(),
                       [](char a, char b) {
                           return std::tolower((unsigned char)a) == std::tolower((unsigned char)b);
                       }) != haystack.end();
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

bool menu_nav::search_handler_text(char c) {
    if (search_pattern == SearchPattern::None) {
        return false;
    }
    if (search_pattern != SearchPattern::Filter && !accept_search_input()) {
        return false;
    }
    if (!MenuFont->has_char(c)) {
        return false;
    }
    if (search_input.size() < max_search_len) {
        search_input.push_back(c);
    }
    update_search();
    return true;
}

bool menu_nav::search_handler_backspace() {
    if (search_pattern == SearchPattern::None) {
        return false;
    }
    if (search_input.empty()) {
        return false;
    }
    search_input.pop_back();
    update_search();
    return true;
}

void menu_nav::update_search() {
    if (search_pattern == SearchPattern::Filter) {
        update_filter();
        return;
    }
    if (search_input.empty()) {
        return;
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
    case SearchPattern::Filter:
    case SearchPattern::None:
        internal_error("update_search() SearchPattern::None reached!");
        break;
    }
}

void menu_nav::update_filter() {
    // Remember which real entry was selected so we can restore position
    int prev_real = -1;
    if (selected_index >= 0 && selected_index < (int)filter_indices.size()) {
        prev_real = filter_indices[selected_index];
    }

    filter_indices.clear();
    if (search_input.empty()) {
        filter_indices.resize(entries.size());
        std::iota(filter_indices.begin(), filter_indices.end(), 0);
    } else {
        for (int i = 0; i < (int)entries.size(); ++i) {
            if (contains_ci(entries[i].text_left, search_input) ||
                contains_ci(entries[i].text_right, search_input)) {
                filter_indices.push_back(i);
            }
        }
    }

    // Restore selection to the same entry, or clamp if it's no longer visible
    if (prev_real >= 0) {
        auto it = std::find(filter_indices.begin(), filter_indices.end(), prev_real);
        if (it != filter_indices.end()) {
            selected_index = (int)(it - filter_indices.begin());
            return;
        }
    }
    selected_index = std::min(selected_index, std::max(0, (int)filter_indices.size() - 1));
}
