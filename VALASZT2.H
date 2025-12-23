#ifndef VALASZT2_H
#define VALASZT2_H

extern const int LISTegykepen;
extern const int LISTx0;
extern const int LISTy0;
extern const int LISTdy;

#define NAV_ENTRY_TEXT_MAX_LENGTH (40)
extern int NavEntriesLeftMaxLength;
#define NAV_ENTRIES_RIGHT_MAX_LENGTH (110)

typedef char nav_entry[NAV_ENTRY_TEXT_MAX_LENGTH + 2];

extern nav_entry* NavEntriesLeft;
extern nav_entry NavEntriesRight[NAV_ENTRIES_RIGHT_MAX_LENGTH + 1];

void menu_nav_entries_init(void);

// Felhasznalja, Korny rubrikak-at es picbuffer-t!
class menu_nav {
    friend void playextmenu(void);

    nav_entry* entries_left;
    nav_entry* entries_right;
    int length;
    int two_columns;
    menu_pic* menu;

  public:
    int selected_index;
    int x_left, y_entries, dy, max_visible_entries;
    int x_right; // Csak akkor erdekes, ha two_columns igaz
    int y_title; // Ezt o maganak beallitja, de nehanyan atdefinialjak
    int enable_esc;
    char title[100];

    menu_nav(void);
    ~menu_nav(void);
    void setup(int len, int two_col = 0); // Rubrikakat beveszi
    // Visszaadja valasztott szamat, ESC eseten -1-et:
    // tobbrajz eseten tobbszor is kirajzolja kezdetben (Win95):
    int navigate(text_line* extra_lines = NULL, int extra_lines_length = 0, int render_only = 0);

    void render(void);
};

// RECidomenteshez kell (mainmenu.cpp-ben is):
// Ha CtrlAltEnabled-et beallitjuk, akkor CtrlAltPressed-t beallitja
// Ctrl+Alt+Enter eseten (elotte CtrlAltPressed-t nullazni kell):
extern int CtrlAltEnabled;
extern int CtrlAltPressed;

#endif
