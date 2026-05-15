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

enum {
    Tool_Move = 0,
    Tool_ZoomIn,
    Tool_CreateVertex,
    Tool_DeleteVertex,
    Tool_DeletePolygon,
    Tool_CreateFood,
    Tool_CreateKiller,
    Tool_DeleteObject,
    Tool_CreateSprite,
    Tool_DeleteSprite
};

extern int SelectedTool;

constexpr unsigned char EditorPaletteId_MenuBorder = 0;
constexpr unsigned char EditorPaletteId_WindowBorder = 0;
constexpr unsigned char EditorPaletteId_Menu = 1;
constexpr unsigned char EditorPaletteId_MenuSelected = 2;
constexpr unsigned char EditorPaletteId_Background = 3;
constexpr unsigned char EditorPaletteId_Window = 4;
constexpr unsigned char EditorPaletteId_LevelNameWindow = 4;
constexpr unsigned char EditorPaletteId_LevelNameWindowBorder = 5;
constexpr unsigned char EditorPaletteId_WindowList = 6;
constexpr unsigned char EditorPaletteId_WindowListSelected = 7;
constexpr unsigned char EditorPaletteId_WindowButton = 8;
constexpr unsigned char EditorPaletteId_WindowInput = 9;

extern int MouseX;
extern int MouseY;
void erase_cursor();
void draw_cursor();
void update_and_draw_cursor();

void invalidate_editor_level();
void invalidate_editor_gui();

void editor();

void draw_tooltip(const char* text = nullptr);

#endif
