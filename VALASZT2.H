#ifndef VALASZT2_H
#define VALASZT2_H

extern const int LISTegykepen;
extern const int LISTx0;
extern const int LISTy0;
extern const int LISTdy;

constexpr int NAV_ENTRY_TEXT_MAX_LENGTH = 40;
extern int NavEntriesLeftMaxLength;
constexpr int NAV_ENTRIES_RIGHT_MAX_LENGTH = 110;

typedef char nav_entry[NAV_ENTRY_TEXT_MAX_LENGTH + 2];

extern nav_entry* NavEntriesLeft;
extern nav_entry NavEntriesRight[NAV_ENTRIES_RIGHT_MAX_LENGTH + 1];

void menu_nav_entries_init();

// Felhasznalja, Korny rubrikak-at es picbuffer-t!
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
    int max_visible_entries;
    int x_right; // Csak akkor erdekes, ha two_columns igaz
    int y_title; // Ezt o maganak beallitja, de nehanyan atdefinialjak
    bool enable_esc;
    char title[100];

    menu_nav();
    ~menu_nav();
    void setup(int len, bool two_col = false); // Rubrikakat beveszi
    // Visszaadja valasztott szamat, ESC eseten -1-et:
    // tobbrajz eseten tobbszor is kirajzolja kezdetben (Win95):
    int navigate(text_line* extra_lines = nullptr, int extra_lines_length = 0,
                 bool render_only = false);

    void render();
};

// RECidomenteshez kell (mainmenu.cpp-ben is):
// Ha CtrlAltEnabled-et beallitjuk, akkor CtrlAltPressed-t beallitja
// Ctrl+Alt+Enter eseten (elotte CtrlAltPressed-t nullazni kell):
extern int CtrlAltEnabled;
extern int CtrlAltPressed;

#endif
