#include "EDITUJ.H"
#include "abc8.h"
#include "editor/help.h"
#include "editor/canvas.h"
#include "editor/dialog.h"
#include "editor/window.h"
#include "editor/tool.h"
#include "editor/topology.h"
#include "keys.h"
#include "LEJATSZO.H"
#include "level.h"
#include "lgr.h"
#include "level_load.h"
#include "M_PIC.H"
#include "main.h"
#include "menu/pic.h"
#include "object.h"
#include "pic8.h"
#include "platform/implementation.h"
#include "platform/utils.h"
#include "polygon.h"
#include "sprite.h"
#include <cstring>
#include <directinput/scancodes.h>

level* Level = nullptr;

abc8* EditorWhiteFont = nullptr;
abc8* EditorBlackFont = nullptr;

polygon* SelectedPolygon = nullptr;
int SelectedVertexIndex = 0;
bool CreateVertexDirection = false;

object* SelectedObject = nullptr;
sprite* SelectedSprite = nullptr;

bool LevelChanged = false;

palette* EditorPalette = nullptr;

constexpr int COMMANDS_LENGTH = 13;
constexpr int TOOLS_LENGTH = 10;
constexpr int MENU_LENGTH = COMMANDS_LENGTH + TOOLS_LENGTH;
constexpr int MENU_ENTRY_HEIGHT = 19;

constexpr char MENU_TEXT[23][15] = {"Exit",
                                    "New",
                                    "Open",
                                    "Save As",
                                    "Save",
                                    "Save and Play",
                                    "Check Topology",
                                    "Properties",
                                    "Zoom Out",
                                    "Zoom Fill",
                                    "View Options",
                                    "Help",
                                    "",
                                    "Move",
                                    "Zoom In",
                                    "Create Vertex",
                                    "Delete Vertex",
                                    "Delete Polygon",
                                    "Create Food",
                                    "Create Killer",
                                    "Delete Object",
                                    "Create Picture",
                                    "Delete Picture"};

enum class Tool {
    Move = 0,
    ZoomIn,
    CreateVertex,
    DeleteVertex,
    DeletePolygon,
    CreateFood,
    CreateKiller,
    DeleteObject,
    CreateSprite,
    DeleteSprite
};

static Tool SelectedTool = Tool::Move;

void create_editor_palette() {
    unsigned char pal[768];
    std::memset(pal, 128, sizeof(pal));

    // 0-63: Hardcoded colours
    pal[EditorPaletteId::MENU_BORDER * 3 + 0] = 0;
    pal[EditorPaletteId::MENU_BORDER * 3 + 1] = 0;
    pal[EditorPaletteId::MENU_BORDER * 3 + 2] = 0;
    pal[EditorPaletteId::MENU * 3 + 0] = 122;
    pal[EditorPaletteId::MENU * 3 + 1] = 131;
    pal[EditorPaletteId::MENU * 3 + 2] = 181;
    pal[EditorPaletteId::MENU_SELECTED * 3 + 0] = 48;
    pal[EditorPaletteId::MENU_SELECTED * 3 + 1] = 56;
    pal[EditorPaletteId::MENU_SELECTED * 3 + 2] = 107;
    pal[EditorPaletteId::BACKGROUND * 3 + 0] = 171;
    pal[EditorPaletteId::BACKGROUND * 3 + 1] = 146;
    pal[EditorPaletteId::BACKGROUND * 3 + 2] = 142;
    pal[EditorPaletteId::WINDOW * 3 + 0] = 252;
    pal[EditorPaletteId::WINDOW * 3 + 1] = 218;
    pal[EditorPaletteId::WINDOW * 3 + 2] = 213;
    pal[EditorPaletteId::LEVEL_NAME_WINDOW_BORDER * 3 + 0] = 0;
    pal[EditorPaletteId::LEVEL_NAME_WINDOW_BORDER * 3 + 1] = 0;
    pal[EditorPaletteId::LEVEL_NAME_WINDOW_BORDER * 3 + 2] = 0;
    pal[EditorPaletteId::WINDOW_LIST * 3 + 0] = 213;
    pal[EditorPaletteId::WINDOW_LIST * 3 + 1] = 148;
    pal[EditorPaletteId::WINDOW_LIST * 3 + 2] = 139;
    pal[EditorPaletteId::WINDOW_LIST_SELECTED * 3 + 0] = 223;
    pal[EditorPaletteId::WINDOW_LIST_SELECTED * 3 + 1] = 209;
    pal[EditorPaletteId::WINDOW_LIST_SELECTED * 3 + 2] = 206;
    pal[EditorPaletteId::WINDOW_BUTTON * 3 + 0] = 198;
    pal[EditorPaletteId::WINDOW_BUTTON * 3 + 1] = 202;
    pal[EditorPaletteId::WINDOW_BUTTON * 3 + 2] = 231;
    pal[EditorPaletteId::WINDOW_INPUT * 3 + 0] = 255;
    pal[EditorPaletteId::WINDOW_INPUT * 3 + 1] = 255;
    pal[EditorPaletteId::WINDOW_INPUT * 3 + 2] = 255;
    // Unused
    pal[10 * 3 + 0] = 180;
    pal[10 * 3 + 1] = 120;
    pal[10 * 3 + 2] = 110;
    // Unused
    pal[39 * 3 + 0] = 0;
    pal[39 * 3 + 1] = 0;
    pal[39 * 3 + 2] = 0;
    pal[EditorPaletteId::BLACK_FONT * 3 + 0] = 0;
    pal[EditorPaletteId::BLACK_FONT * 3 + 1] = 0;
    pal[EditorPaletteId::BLACK_FONT * 3 + 2] = 0;
    pal[EditorPaletteId::WHITE_FONT * 3 + 0] = 255;
    pal[EditorPaletteId::WHITE_FONT * 3 + 1] = 255;
    pal[EditorPaletteId::WHITE_FONT * 3 + 2] = 255;

    // 64-127: True colour RGB222 (i -> 0x00RRGGBB)
    for (int i = 0; i < 64; i++) {
        int r = (i >> 4) & 0b00000011;
        int g = (i >> 2) & 0b00000011;
        int b = (i >> 0) & 0b00000011;
        pal[(64 + i) * 3 + 0] = (unsigned char)(r * 64 + 8);
        pal[(64 + i) * 3 + 1] = (unsigned char)(g * 64 + 8);
        pal[(64 + i) * 3 + 2] = (unsigned char)(b * 64 + 8);
    }

    // 128-255: inverse colours of 0-127, when mousing over or drawing over
    for (int i = 0; i < 128; i++) {
        int source = i * 3;
        int inverted = (i + 128) * 3;
        int sum = pal[source + 0] + pal[source + 1] + pal[source + 2];
        if (sum >= 128 * 3) {
            pal[inverted + 0] = 0;
            pal[inverted + 1] = 0;
            pal[inverted + 2] = 0;
        } else {
            pal[inverted + 0] = 255;
            pal[inverted + 1] = 255;
            pal[inverted + 2] = 255;
        }
    }

    if (EditorPalette) {
        internal_error("create_editor_palette EditorPalette already exists!");
    }
    EditorPalette = new palette(pal);
}

int MouseX = 0;
int MouseY = 0;
static bool CursorShapeIsX = false;

static void draw_cursor_pixel(int x, int y) {
    if (x < 0 || y < 0 || x > (SCREEN_WIDTH - 1) || y > (SCREEN_HEIGHT - 1)) {
        return;
    }
    unsigned char palette_id = BufferMain->gpixel(x, y);
    palette_id += 128;
    BufferMain->ppixel(x, y, palette_id);
    ppixelfront(x, y, palette_id);
}

static void draw_cursor_shape() {
    lockfrontbuffer_pic();
    int radius = 4;
    for (int i = -radius; i <= radius; i++) {
        if (CursorShapeIsX) {
            // Draw "x"
            draw_cursor_pixel(MouseX + i, MouseY + i);
            if (i != 0) {
                draw_cursor_pixel(MouseX + i, MouseY - i);
            }
        } else {
            // Draw "+"
            draw_cursor_pixel(MouseX + i, MouseY);
            if (i != 0) {
                draw_cursor_pixel(MouseX, MouseY + i);
            }
        }
    }
    unlockfrontbuffer_pic();
}

static bool CursorDrawn = false;

void erase_cursor() {
    if (!CursorDrawn) {
        internal_error("erase_cursor-ban !CursorDrawn!");
    }
    CursorDrawn = false;
    draw_cursor_shape();
}

void draw_cursor() {
    if (CursorDrawn) {
        internal_error("draw_cursor-ban CursorDrawn!");
    }
    CursorDrawn = true;
    draw_cursor_shape();
}

void update_and_draw_cursor() {
    int mouse_x = 0;
    int mouse_y = 0;
    get_mouse_position(&mouse_x, &mouse_y);
    if (mouse_x == MouseX && mouse_y == MouseY) {
        return;
    }
    erase_cursor();
    MouseX = mouse_x;
    MouseY = mouse_y;
    draw_cursor();
}

void draw_tooltip(const char* text) {
    static char TooltipText[110] = "";
    if (text) {
        if (strlen(text) > 100) {
            internal_error(std::string("draw_tooltip strlen( text ) > 100!\n") + text);
        }
        strcpy(TooltipText, text);
    }
    int x1 = 1;
    int y1 = 18;
    int x2 = SCREEN_WIDTH - 2;
    int y2 = EDITOR_MENU_Y - 2;
    BufferMain->fill_box(x1, y1, x2, y2, EditorPaletteId::BACKGROUND);
    EditorWhiteFont->write(BufferMain, 10, y1 + 12, TooltipText);
    bltfront(BufferMain, x1, y1, x2, y2);
}

void draw_tooltip_help() {
    switch (SelectedTool) {
    case Tool::Move:
        draw_tooltip(
            "Move the cursor near a vertex or an object center you want to move, and left click.");
        break;
    case Tool::ZoomIn:
        draw_tooltip("Left click to place the first corner of the zoom in window.");
        break;
    case Tool::CreateVertex:
        draw_tooltip(
            "If you left click near a vertex you will add to a polygon, otherwise you will "
            "create a new polygon.");
        break;
    case Tool::DeleteVertex:
        draw_tooltip("Left click near the vertex you want to delete.");
        break;
    case Tool::DeletePolygon:
        draw_tooltip("Left click near any vertex of the polygon you want to delete.");
        break;
    case Tool::CreateFood:
        draw_tooltip("Left click to place a new Food object.");
        break;
    case Tool::CreateKiller:
        draw_tooltip("Left click to place a new Killer object.");
        break;
    case Tool::DeleteObject:
        draw_tooltip("Left click near the center of the object you want to delete.");
        break;
    case Tool::CreateSprite:
        draw_tooltip("Left click to place a new Picture. Right click chooses the picture.");
        break;
    case Tool::DeleteSprite:
        draw_tooltip("Left click near the top-left corner of the picture you want to delete.");
        break;
    default:
        internal_error("draw_tooltip_help unknown tool!");
    }
}

static bool RedrawEditorCanvas = false;
void invalidate_editor_level() { RedrawEditorCanvas = true; }

static bool RedrawEditorGui = false;
void invalidate_editor_gui() { RedrawEditorGui = true; }

static void draw_editor_gui() {
    // Entire screen blue
    BufferMain->fill_box(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, EditorPaletteId::MENU);
    // Entire screen border
    BufferMain->line(0, 0, SCREEN_WIDTH - 1, 0, EditorPaletteId::MENU_BORDER);
    BufferMain->line(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1,
                     EditorPaletteId::MENU_BORDER);
    BufferMain->line(0, 0, 0, SCREEN_HEIGHT - 1, EditorPaletteId::MENU_BORDER);
    BufferMain->line(SCREEN_WIDTH - 1, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1,
                     EditorPaletteId::MENU_BORDER);
    // Tooltip section line (top)
    BufferMain->line(0, EDITOR_MENU_Y - 1, SCREEN_WIDTH - 1, EDITOR_MENU_Y - 1,
                     EditorPaletteId::MENU_BORDER);
    // Menu section line (left)
    BufferMain->line(EDITOR_MENU_X - 1, EDITOR_MENU_Y, EDITOR_MENU_X - 1, SCREEN_HEIGHT - 1,
                     EditorPaletteId::MENU_BORDER);
    // Menu entries
    int tool_index = static_cast<int>(SelectedTool);
    for (int i = 0; i < MENU_LENGTH; i++) {
        int x1 = 1;
        int y1 = EDITOR_MENU_Y + i * MENU_ENTRY_HEIGHT + 1;
        int x2 = EDITOR_MENU_X - 2;
        int y2 = EDITOR_MENU_Y + (i + 1) * MENU_ENTRY_HEIGHT - 1;
        if (i == COMMANDS_LENGTH + tool_index) {
            BufferMain->fill_box(x1, y1, x2, y2, EditorPaletteId::MENU_SELECTED);
        }
        BufferMain->line(x1, y2 + 1, x2, y2 + 1, EditorPaletteId::MENU_BORDER);
        EditorWhiteFont->write(BufferMain, 5, EDITOR_MENU_Y + i * MENU_ENTRY_HEIGHT + 14,
                               MENU_TEXT[i]);
    }
    // Underline Save and *P*lay
    int x = 66;
    int y = EDITOR_MENU_Y + 5 * MENU_ENTRY_HEIGHT + 15;
    BufferMain->line(x, y, x + 6, y, 247);
    // Underline *E*xit
    x = 13;
    y = EDITOR_MENU_Y + 0 * MENU_ENTRY_HEIGHT + 15;
    BufferMain->line(x, y, x + 7, y, 247);
    // Underline *O*pen
    x = 5;
    y = EDITOR_MENU_Y + 2 * MENU_ENTRY_HEIGHT + 15;
    BufferMain->line(x, y, x + 6, y, 247);
    // Underline *Z*oom Out
    x = 5;
    y = EDITOR_MENU_Y + 8 * MENU_ENTRY_HEIGHT + 15;
    BufferMain->line(x, y, x + 7, y, 247);
    // Underline *S*ave
    x = 4;
    y = EDITOR_MENU_Y + 4 * MENU_ENTRY_HEIGHT + 15;
    BufferMain->line(x, y, x + 7, y, 247);
}

static pic8 TooltipBuffer(400, 17);
static char TooltipPictureName[10] = "";
static char TooltipTextureName[10] = "";
static char TooltipMaskName[10] = "";

void draw_sprite_tooltip(const char* picture_name, const char* texture_name, const char* mask_name,
                         int distance, Clipping clipping) {
    // Skip if no change
    if (strcmp(picture_name, TooltipPictureName) == 0 &&
        strcmp(texture_name, TooltipTextureName) == 0 && strcmp(mask_name, TooltipMaskName) == 0) {
        return;
    }

    strcpy(TooltipPictureName, picture_name);
    strcpy(TooltipTextureName, texture_name);
    strcpy(TooltipMaskName, mask_name);

    TooltipBuffer.fill_box(EditorPaletteId::BACKGROUND);

    if (picture_name[0] && (texture_name[0] || mask_name[0])) {
        internal_error("draw_sprite_tooltip too many params!");
    }

    if (picture_name[0]) {
        EditorWhiteFont->write(&TooltipBuffer, 0, 14, "PICTURE:");
        char tmp[30];
        sprintf(tmp, "%s (%d)  ", picture_name, distance);
        if (clipping == Clipping::Unclipped) {
            strcat(tmp, "U");
        }
        if (clipping == Clipping::Ground) {
            strcat(tmp, "G");
        }
        if (clipping == Clipping::Sky) {
            strcat(tmp, "S");
        }
        EditorBlackFont->write(&TooltipBuffer, 60, 14, tmp);
    }
    if (texture_name[0] || mask_name[0]) {
        EditorWhiteFont->write(&TooltipBuffer, 0, 14, "TEXTURE:");
        EditorBlackFont->write(&TooltipBuffer, 60, 14, texture_name);
        EditorWhiteFont->write(&TooltipBuffer, 126, 14, "MASK:");
        char tmp[30];
        sprintf(tmp, "%s (%d)  ", mask_name, distance);
        if (clipping == Clipping::Unclipped) {
            strcat(tmp, "U");
        }
        if (clipping == Clipping::Ground) {
            strcat(tmp, "G");
        }
        if (clipping == Clipping::Sky) {
            strcat(tmp, "S");
        }
        EditorBlackFont->write(&TooltipBuffer, 164, 14, tmp);
    }

    blit8(BufferMain, &TooltipBuffer, 226, 2);
    bltfront(BufferMain, 226, 2, 540, 30);
}

void draw_object_tooltip(object::Type type, object::Property property, int animation) {
    // Impossible name (9 characters long) to invalidate cache
    strcpy(TooltipPictureName, "aaccbbdde");

    TooltipBuffer.fill_box(EditorPaletteId::BACKGROUND);
    EditorWhiteFont->write(&TooltipBuffer, 0, 14, "Object:");
    char tmp[50];
    if (type == object::Type::Exit) {
        sprintf(tmp, "Exit");
    }
    if (type == object::Type::Start) {
        sprintf(tmp, "Start");
    }
    if (type == object::Type::Food) {
        if (property == object::Property::None) {
            sprintf(tmp, "Food");
        }
        if (property == object::Property::GravityUp) {
            sprintf(tmp, "Grav. Up");
        }
        if (property == object::Property::GravityDown) {
            sprintf(tmp, "Grav. Down");
        }
        if (property == object::Property::GravityLeft) {
            sprintf(tmp, "Grav. Left");
        }
        if (property == object::Property::GravityRight) {
            sprintf(tmp, "Grav. Right");
        }
        EditorWhiteFont->write(&TooltipBuffer, 130, 14, "Anim num:");
        char tmp2[10];
        sprintf(tmp2, "%d", (int)(animation + 1));
        EditorBlackFont->write(&TooltipBuffer, 193, 14, tmp2);
    }
    if (type == object::Type::Killer) {
        sprintf(tmp, "Killer");
    }
    EditorBlackFont->write(&TooltipBuffer, 50, 14, tmp);

    blit8(BufferMain, &TooltipBuffer, 226, 2);
    bltfront(BufferMain, 226, 2, 540, 30);
}

static void draw_editor() {
    erase_cursor();

    // Make sure the sprite tooltip will be immediately updated
    TooltipPictureName[0] = 0;
    TooltipTextureName[0] = 0;
    TooltipMaskName[0] = 0;
    if (RedrawEditorGui) {
        draw_editor_gui();
    }

    // Background of level canvas
    BufferMain->fill_box(EDITOR_MENU_X, EDITOR_MENU_Y, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2,
                         EditorPaletteId::BACKGROUND);
    // Background of tooltip section (top)
    BufferMain->fill_box(1, 1, SCREEN_WIDTH - 2, EDITOR_MENU_Y - 2, EditorPaletteId::BACKGROUND);

    // Tooltip section
    EditorWhiteFont->write(BufferMain, 6, 15, "File:");
    strupr(State->editor_filename);
    int filename_x = 41;
    if (State->editor_filename[0] == 0) {
        EditorBlackFont->write(BufferMain, filename_x, 15, "UNNAMED");
        filename_x += EditorBlackFont->len("UNNAMED");
    } else {
        EditorBlackFont->write(BufferMain, filename_x, 15, State->editor_filename);
        filename_x += EditorBlackFont->len(State->editor_filename);
    }
    if (LevelChanged) {
        EditorWhiteFont->write(BufferMain, filename_x + 3, 15, "(Changed)");
    } else {
        EditorWhiteFont->write(BufferMain, filename_x + 3, 15, "(Unchanged)");
    }

    draw_tooltip();

    EditorWhiteFont->write(BufferMain, 543, 15, "Zoom:");
    char text[30];
    sprintf(text, "%lf", get_zoom());
    if (*strstr(text, ".")) {
        // Four digits after decimal point
        *(strstr(text, ".") + 5) = 0;
    }
    EditorBlackFont->write(BufferMain, 580, 15, text);

    RedrawingEditor = true;
    Level->render();
    if (SelectedPolygon) {
        SelectedPolygon->render_one_line(SelectedVertexIndex, CreateVertexDirection, false);
        SelectedPolygon->render_one_line(SelectedVertexIndex, CreateVertexDirection, true);
    }
    RedrawingEditor = false;

    bltfront(BufferMain);

    RedrawEditorCanvas = false;
    RedrawEditorGui = false;

    draw_cursor();

    if (SelectedTool == Tool::CreateSprite) {
        if (!Lgr) {
            internal_error("draw_editor !Lgr");
        }
        draw_sprite_tooltip(Lgr->editor_picture_name, Lgr->editor_texture_name,
                            Lgr->editor_mask_name, 0, Clipping::Unclipped);
    }
}

// Update the mouse shape from + to x if hovering over something
// Change the tooltip to provide info about what is being hovered
static void draw_mouse_tooltip(int mouse_x, int mouse_y) {
    // Food tooltip
    if (SelectedTool == Tool::CreateFood) {
        draw_object_tooltip(object::Type::Food, DefaultFoodProperty, DefaultFoodAnimation);
        return;
    }

    // Sprite tooltip
    if (SelectedTool == Tool::CreateSprite) {
        if (!Lgr) {
            internal_error("draw_mouse_tooltip !Lgr");
        }
        draw_sprite_tooltip(Lgr->editor_picture_name, Lgr->editor_texture_name,
                            Lgr->editor_mask_name, 0, Clipping::Unclipped);
        return;
    }

    // Hovered tooltip
    polygon* poly = nullptr;
    object* obj = nullptr;
    sprite* spr = nullptr;
    bool hover_poly = SelectedTool == Tool::Move || SelectedTool == Tool::CreateVertex ||
                      SelectedTool == Tool::DeleteVertex || SelectedTool == Tool::DeletePolygon;
    bool hover_obj = SelectedTool == Tool::Move || SelectedTool == Tool::DeleteObject;
    bool hover_spr = SelectedTool == Tool::Move || SelectedTool == Tool::DeleteSprite;
    Level->get_closest_entity(mouse_x, mouse_y, hover_poly ? &poly : nullptr, &SelectedVertexIndex,
                              hover_obj ? &obj : nullptr, hover_spr ? &spr : nullptr);

    CursorShapeIsX = poly || obj || spr;
    if (spr) {
        draw_sprite_tooltip(spr->picture_name, spr->texture_name, spr->mask_name, spr->distance,
                            spr->clipping);
    } else if (obj) {
        draw_object_tooltip(obj->type, obj->property, obj->animation);
    } else {
        // Vertex or no tooltip
        draw_sprite_tooltip("", "", "", 0, Clipping::Unclipped);
    }
}

static void select_tool(int tool) {
    Tool new_tool = static_cast<Tool>(tool);
    if (new_tool == SelectedTool) {
        return;
    }
    SelectedTool = new_tool;
    draw_tooltip_help();
    invalidate_editor_gui();
}

static void editor_play(bool view_map) {
    invalidate_editor_gui();
    if (LevelChanged || State->editor_filename[0] == 0 || Level->topology_errors) {
        if (!editor_window_save()) {
            return;
        }
    }

    if (Level->topology_errors) {
        return;
    }

    load_level_play(State->editor_filename);

    Rec1->erase(State->editor_filename);
    Rec2->erase(State->editor_filename);
    game_loop(State->editor_filename, view_map ? CameraMode::MapViewer : CameraMode::Normal);

    empty_keypress_buffer();

    EditorPalette->set();
}

static bool editor_dialog_exit() {
    invalidate_editor_gui();
    if (LevelChanged) {
        if (dialog("There are unsaved changes in the level file.",
                   "If you exit now, you will lose these changes.", "Do you still want to exit?",
                   DIALOG_BUTTONS, "Yes", "No") == 1) {
            return false;
        }
    }
    invalidate_level();
    return true;
}

static void editor_zoom_out() {
    zoom_out();
    invalidate_editor_level();
}

void editor() {
    if (!BufferMain) {
        internal_error("editor !BufferMain!");
    }

    EditorPalette->set();

    draw_editor_gui();
    bltfront(BufferMain);

    CursorDrawn = false;
    draw_cursor();

    LevelChanged = false;
    editor_window_welcome();
    if (State->editor_filename[0] == 0) {
        load_level_editor(DEFAULT_LEVEL_FILENAME);
    } else {
        if (!load_level_editor(State->editor_filename)) {
            State->editor_filename[0] = 0;
        }
    }

    SelectedPolygon = nullptr;
    SelectedVertexIndex = 0;
    CreateVertexDirection = false;

    SelectedObject = nullptr;
    SelectedSprite = nullptr;

    DefaultFoodProperty = object::Property::None;
    DefaultFoodAnimation = 0;

    SelectedTool = Tool::Move;
    draw_tooltip_help();

    zoom_fill();
    invalidate_editor_gui();
    draw_editor();

    while (true) {
        handle_events();
        // Control keys
        if (was_key_just_pressed(DIK_ESCAPE)) {
            if (SelectedTool == Tool::Move) {
                tool_move_esc();
            }
            if (SelectedTool == Tool::ZoomIn) {
                tool_zoom_in_esc();
            }
            if (SelectedTool == Tool::CreateVertex) {
                tool_create_vertex_esc();
            }
        }
        if (was_key_just_pressed(DIK_SPACE)) {
            if (SelectedTool == Tool::CreateVertex) {
                tool_create_vertex_space();
            }
        }
        if (was_key_just_pressed(DIK_RETURN)) {
            if (SelectedTool == Tool::CreateVertex) {
                tool_create_vertex_enter();
            }
        }
        // Character shortcuts from text buffer
        while (has_text_input()) {
            char c = pop_text_input();
            if (!(SelectedPolygon || SelectedObject || SelectedSprite || SelectingZoomInBox ||
                  CreatingPolygon)) {
                if (c == 'p') {
                    editor_play(is_key_down(DIK_F1));
                }
                if (c == 'x') {
                    if (editor_dialog_exit()) {
                        erase_cursor();
                        return;
                    }
                }
                if (c == 'o') {
                    editor_window_open();
                }
                if (c == 's') {
                    editor_window_save();
                }
                if (c == 'z') {
                    editor_zoom_out();
                }
                // Undocumented screenshot button
                if (c == 'i') {
                    platform_save_screenshot();
                }
            }
        }
        bool left_click = was_left_mouse_just_clicked();
        bool right_click = was_right_mouse_just_clicked();
        if (left_click || right_click) {
            int click_x = 0;
            int click_y = 0;
            get_mouse_position(&click_x, &click_y);
            if (click_y < EDITOR_MENU_Y) {
                // Mouse is not allowed in tooltip section
                click_y = EDITOR_MENU_Y;
            }
            if ((SelectedPolygon || SelectedObject || SelectedSprite || SelectingZoomInBox ||
                 CreatingPolygon) &&
                click_x < EDITOR_MENU_X) {
                // If holding something, mouse is not allowed in menu section
                click_x = EDITOR_MENU_X;
                set_mouse_position(click_x, click_y);
            }
            if (click_x != MouseX || click_y != MouseY) {
                erase_cursor();
                MouseX = click_x;
                MouseY = click_y;
                draw_cursor();
            }
            if (click_x < EDITOR_MENU_X) {
                // Menu clicks
                for (int i = 0; i < MENU_LENGTH; i++) {
                    if (EDITOR_MENU_Y + i * MENU_ENTRY_HEIGHT <= click_y &&
                        click_y < EDITOR_MENU_Y + (i + 1) * MENU_ENTRY_HEIGHT) {
                        // Commands
                        if (i == 0 && left_click) {
                            if (editor_dialog_exit()) {
                                erase_cursor();
                                return;
                            }
                        }
                        if (i == 0 && right_click) {
                            editor_help_exit();
                        }
                        if (i == 1 && left_click) {
                            editor_new();
                        }
                        if (i == 1 && right_click) {
                            editor_help_new();
                        }
                        if (i == 2 && left_click) {
                            editor_window_open();
                        }
                        if (i == 2 && right_click) {
                            editor_help_open();
                        }
                        if (i == 3 && left_click) {
                            editor_window_save_as();
                        }
                        if (i == 3 && right_click) {
                            editor_help_save_as();
                        }
                        if (i == 4 && left_click) {
                            editor_window_save();
                        }
                        if (i == 4 && right_click) {
                            editor_help_save();
                        }
                        if (i == 5 && left_click) {
                            editor_play(is_key_down(DIK_F1));
                            SelectedTool = Tool::Move;
                        }
                        if (i == 5 && right_click) {
                            editor_help_save_and_play();
                        }
                        if (i == 6 && left_click) {
                            check_topology(true);
                        }
                        if (i == 6 && right_click) {
                            editor_help_check_topology();
                        }
                        if (i == 7 && left_click) {
                            editor_window_level_properties();
                        }
                        if (i == 7 && right_click) {
                            editor_help_properties();
                        }
                        if (i == 8 && left_click) {
                            editor_zoom_out();
                        }
                        if (i == 8 && right_click) {
                            editor_help_zoom_out();
                        }
                        if (i == 9 && left_click) {
                            zoom_fill();
                            invalidate_editor_level();
                        }
                        if (i == 9 && right_click) {
                            editor_help_zoom_fill();
                        }
                        if (i == 10 && left_click) {
                            editor_window_view_options();
                        }
                        if (i == 10 && right_click) {
                            editor_help_view_options();
                        }
                        if (i == 11) {
                            editor_help();
                        }

                        // Tools
                        if (left_click) {
                            if (i >= COMMANDS_LENGTH && i < COMMANDS_LENGTH + TOOLS_LENGTH) {
                                select_tool(i - COMMANDS_LENGTH);
                            }
                        } else {
                            if (i == COMMANDS_LENGTH + 0) {
                                editor_help_move();
                            }
                            if (i == COMMANDS_LENGTH + 1) {
                                editor_help_zoom_in();
                            }
                            if (i == COMMANDS_LENGTH + 2) {
                                editor_help_create_vertex();
                            }
                            if (i == COMMANDS_LENGTH + 3) {
                                editor_help_delete_vertex();
                            }
                            if (i == COMMANDS_LENGTH + 4) {
                                editor_help_delete_polygon();
                            }
                            if (i == COMMANDS_LENGTH + 5) {
                                editor_help_create_food();
                            }
                            if (i == COMMANDS_LENGTH + 6) {
                                editor_help_create_killer();
                            }
                            if (i == COMMANDS_LENGTH + 7) {
                                editor_help_delete_object();
                            }
                            if (i == COMMANDS_LENGTH + 8) {
                                editor_help_create_sprite();
                            }
                            if (i == COMMANDS_LENGTH + 9) {
                                editor_help_delete_sprite();
                            }
                        }
                    }
                }
            } else {
                // Canvas clicks
                if (SelectedTool == Tool::Move && left_click) {
                    tool_move_leftclick(click_x, click_y);
                }
                if (SelectedTool == Tool::Move && right_click) {
                    tool_move_rightclick(click_x, click_y);
                }
                if (SelectedTool == Tool::ZoomIn && left_click) {
                    tool_zoom_in_leftclick(click_x, click_y);
                }
                if (SelectedTool == Tool::ZoomIn && right_click) {
                    tool_zoom_in_esc();
                }
                if (SelectedTool == Tool::CreateVertex && left_click) {
                    tool_create_vertex_leftclick(click_x, click_y);
                }
                if (SelectedTool == Tool::CreateVertex && right_click) {
                    tool_create_vertex_esc();
                }
                if (SelectedTool == Tool::DeleteVertex && left_click) {
                    tool_delete_vertex_leftclick(click_x, click_y);
                }
                if (SelectedTool == Tool::DeletePolygon && left_click) {
                    tool_delete_polygon_leftclick(click_x, click_y);
                }
                if (SelectedTool == Tool::CreateFood && left_click) {
                    tool_create_object_leftclick(click_x, click_y, true);
                }
                if (SelectedTool == Tool::CreateFood && right_click) {
                    tool_create_food_rightclick();
                }
                if (SelectedTool == Tool::CreateKiller && left_click) {
                    tool_create_object_leftclick(click_x, click_y, false);
                }
                if (SelectedTool == Tool::DeleteObject && left_click) {
                    tool_delete_object_leftclick(click_x, click_y);
                }
                if (SelectedTool == Tool::CreateSprite && left_click) {
                    tool_create_sprite_leftclick(click_x, click_y);
                }
                if (SelectedTool == Tool::CreateSprite && right_click) {
                    tool_create_sprite_rightclick();
                }
                if (SelectedTool == Tool::DeleteSprite && left_click) {
                    tool_delete_sprite_leftclick(click_x, click_y);
                }
            }
        }

        if (RedrawEditorGui || RedrawEditorCanvas) {
            draw_editor();
        }

        int mouse_x = 0;
        int mouse_y = 0;
        get_mouse_position(&mouse_x, &mouse_y);
        if (mouse_y < EDITOR_MENU_Y) {
            // Mouse is not allowed in tooltip section
            mouse_y = EDITOR_MENU_Y;
        }
        if ((SelectedPolygon || SelectedObject || SelectedSprite || SelectingZoomInBox ||
             CreatingPolygon) &&
            mouse_x < EDITOR_MENU_X) {
            // If holding something, mouse is not allowed in menu section
            mouse_x = EDITOR_MENU_X;
            set_mouse_position(mouse_x, mouse_y);
        }

        // Mouse movement
        if (mouse_x == MouseX && mouse_y == MouseY) {
            continue;
        }
        if (SelectedTool == Tool::ZoomIn) {
            tool_zoom_in_mousemove(mouse_x, mouse_y);
            continue;
        }
        if (SelectedTool == Tool::CreateVertex && (SelectedPolygon || CreatingPolygon)) {
            tool_create_vertex_mousemove(mouse_x, mouse_y);
            continue;
        }
        if (!SelectedPolygon && !SelectedObject && !SelectedSprite) {
            erase_cursor();
            draw_mouse_tooltip(mouse_x, mouse_y);
            MouseX = mouse_x;
            MouseY = mouse_y;
            draw_cursor();
        } else {
            if (SelectedTool == Tool::Move) {
                tool_move_mousemove(mouse_x, mouse_y);
            }
        }
    }
}
