#ifndef EDITUJ_H
#define EDITUJ_H

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
constexpr unsigned char MenuBorder = 0;
constexpr unsigned char WindowBorder = 0;
constexpr unsigned char Menu = 1;
constexpr unsigned char MenuSelected = 2;
constexpr unsigned char Background = 3;
constexpr unsigned char Window = 4;
constexpr unsigned char LevelNameWindow = 4;
constexpr unsigned char LevelNameWindowBorder = 5;
constexpr unsigned char WindowList = 6;
constexpr unsigned char WindowListSelected = 7;
constexpr unsigned char WindowButton = 8;
constexpr unsigned char WindowInput = 9;
} // namespace EditorPaletteId

extern int MouseX;
extern int MouseY;
void erase_cursor();
void draw_cursor();
void update_and_draw_cursor();

void invalidate_editor_level();
void invalidate_editor_gui();

void editor();

void draw_tooltip(const char* text = nullptr);
void draw_tooltip_help(); // Base tooltip for current tool when tool has not yet been used

#endif
