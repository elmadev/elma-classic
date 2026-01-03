#ifndef MENU_NAV_H
#define MENU_NAV_H

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

class menu_nav {
    friend void playextmenu();

    nav_entry* entries_left;
    nav_entry* entries_right;
    int length;
    bool two_columns;
    menu_pic* menu;

  public:
    int selected_index;
    int x_left;
    int y_entries;
    int dy;
    int x_right;
    int y_title;
    bool enable_esc;
    char title[100];

    menu_nav();
    ~menu_nav();
    void setup(int len, bool two_col = false);
    int navigate(text_line* extra_lines = nullptr, int extra_lines_length = 0,
                 bool render_only = false);

    void render();

  private:
    int calculate_visible_entries(int extra_lines_length);
};

extern bool CtrlAltPressed;

#endif
