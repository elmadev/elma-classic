#ifndef EDITOR_H
#define EDITOR_H

#include "pic/pic8.h"

class abc8;
class object;
class palette;
class polygon;
class sprite;
class level;

extern level* Level;

extern palette* EditorPalette;
void create_editor_palette();
extern abc8* EditorWhiteFont;
extern abc8* EditorBlackFont;

extern polygon* SelectedPolygon;
extern int SelectedVertexIndex;
extern bool CreateVertexDirection;

extern object* SelectedObject;
extern sprite* SelectedSprite;

extern bool LevelChanged;

namespace EditorPaletteId {
constexpr unsigned char MENU_BORDER = 0;
constexpr unsigned char WINDOW_BORDER = 0;
constexpr unsigned char MENU = 1;
constexpr unsigned char MENU_SELECTED = 2;
constexpr unsigned char BACKGROUND = 3;
constexpr unsigned char WINDOW = 4;
constexpr unsigned char LEVEL_NAME_WINDOW = 4;
constexpr unsigned char LEVEL_NAME_WINDOW_BORDER = 5;
constexpr unsigned char WINDOW_LIST = 6;
constexpr unsigned char WINDOW_LIST_SELECTED = 7;
constexpr unsigned char WINDOW_BUTTON = 8;
constexpr unsigned char WINDOW_INPUT = 9;
constexpr unsigned char BLACK_FONT = 59;
constexpr unsigned char WHITE_FONT = 62;
} // namespace EditorPaletteId

extern int MouseX;
extern int MouseY;
void draw_cursor(pic8& dest, bool cursor_shape_is_x);
void erase_cursor();
void draw_cursor();
void update_and_draw_cursor();

void invalidate_editor_level();
void invalidate_editor_gui();

void editor();

void draw_tooltip(const char* text = nullptr);
void draw_tooltip_help(); // Base tooltip for current tool when tool has not yet been used

#endif
