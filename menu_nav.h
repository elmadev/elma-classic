#ifndef MENU_NAV_H
#define MENU_NAV_H

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

class menu_nav {
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

    menu_nav();
    ~menu_nav();
    void setup(int len, bool two_col = false);
    int navigate(text_line* extra_lines = nullptr, int extra_lines_length = 0,
                 bool render_only = false);

    void render();
    nav_entry* entry_left(int index);

  private:
    int calculate_visible_entries(int extra_lines_length);
    bool search_handler(int code);
};

#define NAV_LAMBDA [](int choice, const char* left, const char* right)
using nav_func = void(int choice, const char* left, const char* right);

struct nav_row {
    nav_entry text_left;
    nav_entry text_right;
    nav_func* handler;
    nav_func* ctrl_alt_handler;
    nav_func* f1_handler;
};

class menu_nav2 {
    std::vector<nav_row> entries;
    std::vector<text_line> extra_lines;
    bool two_columns;
    std::unique_ptr<menu_pic> menu;
    std::string search_input;

    int total_entries();
    int calculate_visible_entries();
    bool search_handler(int code);
    int prompt_choice(bool render_only);

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
    int search_skip;

    menu_nav2(std::string titl);
    void add_row(std::string left, std::string right, nav_func* handle, nav_func* handle_ctrl_alt,
                 nav_func* handle_f1);
    // Function overloads:
    void add_row(std::string left, std::string right, nav_func* handle, nav_func* handle_ctrl_alt) {
        add_row(left, right, handle, handle_ctrl_alt, handle);
    }
    void add_row(std::string left, std::string right, nav_func* handle) {
        add_row(left, right, handle, handle, handle);
    }
    void add_row(std::string left, std::string right) {
        add_row(left, right, nullptr, nullptr, nullptr);
    }
    int navigate(bool render_only = false);
    void render_pic();
};

extern bool CtrlAltPressed;
extern bool F1Pressed;

#endif
