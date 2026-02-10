#ifndef MENU_NAV_H
#define MENU_NAV_H

#include "keys.h"
#include "menu_pic.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class menu_pic;
struct text_line;

#define MENU_CENTER_TEXT "*$$^&|@"

constexpr int NAV_ENTRY_TEXT_MAX_LENGTH = 40;
extern int NavEntriesLeftMaxLength;
constexpr int NAV_ENTRIES_RIGHT_MAX_LENGTH = 110;

typedef char nav_entry[NAV_ENTRY_TEXT_MAX_LENGTH + 2];

extern nav_entry* NavEntriesLeft;
extern nav_entry NavEntriesRight[NAV_ENTRIES_RIGHT_MAX_LENGTH + 1];

void menu_nav_entries_init();

enum class SearchPattern { None, Sorted, Internals };

class menu_nav_old {
    nav_entry* entries_left;
    nav_entry* entries_right;
    int length;
    bool two_columns;
    menu_pic* menu;
    std::string search_input;

  public:
    int selected_index;
    int x_left;
    int y_entries;
    int dy;
    int x_right;
    int y_title;
    bool enable_esc;
    char title[100];
    SearchPattern search_pattern;
    bool search_skip_one;

    menu_nav_old();
    ~menu_nav_old();
    void setup(int len, bool two_col = false);
    int navigate(text_line* extra_lines = nullptr, int extra_lines_length = 0,
                 bool render_only = false);

    void render();
    nav_entry* entry_left(int index);

  private:
    int calculate_visible_entries(int extra_lines_length);
    bool search_handler(int code);
};

extern bool CtrlAltPressed;
extern bool F1Pressed;

#define NAV_FUNC(...) [__VA_ARGS__](int choice, std::string& left, std::string& right)
using nav_func = std::function<void(int, std::string&, std::string&)>;

struct nav_row {
    std::string text_left;
    std::string text_right;
    nav_func handler;
};

enum class OverlayAlignment {
    Left,
    Centered,
};

struct nav_overlay {
    std::string text;
    int x;
    int y;
    OverlayAlignment alignment;
};

class menu_nav {
    std::vector<nav_row> entries;
    std::vector<nav_overlay> overlays;
    std::unique_ptr<menu_pic> menu;
    std::string search_input;

  public:
    int selected_index;
    int x_left;
    int y_entries;
    int dy;
    int x_right;
    int y_title;
    bool enable_esc;
    std::string title;
    SearchPattern search_pattern;
    bool search_skip_one;
    int search_skip;

    menu_nav(std::string title);

    void add_row(std::string left, std::string right, nav_func handler = nullptr);
    void add_overlay(std::string text, int x, int y,
                     OverlayAlignment alignment = OverlayAlignment::Left);

    int navigate(bool render_only = false);
    void render();
    size_t row_count() { return entries.size(); }
    std::string& entry_left(int index);

  private:
    int prompt_choice(bool render_only);
    int calculate_visible_entries();
    bool search_handler(Keycode code);
};

#endif
