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

abc8 *EditorWhiteFont = nullptr, *EditorBlackFont = nullptr;

polygon* SelectedPolygon = nullptr;
int SelectedVertexIndex = 0;
bool CreateVertexDirection = false;

object* SelectedObject = nullptr;

sprite* SelectedSprite = nullptr;

int LevelChanged = 0;

palette* EditorPalette = nullptr;
static unsigned char Pal_editor_byteok[768];

void create_editor_palette() {
    unsigned char* pal = Pal_editor_byteok;
    // Paletta elso 64 szine menu szinek,
    // Masodik 64 szin truecolor tabla
    // Masodik 128 szin eger szine:

    // Lenullazzuk:
    for (int i = 0; i < 768; i++) {
        pal[i] = 128;
    }

    pal[0 + 0] = 0;
    pal[0 + 1] = 0;
    pal[0 + 2] = 0; //  0.
    pal[3 + 0] = 122;
    pal[3 + 1] = 131;
    pal[3 + 2] = 181; //  1.
    pal[6 + 0] = 48;
    pal[6 + 1] = 56;
    pal[6 + 2] = 107; //  2.
    pal[9 + 0] = 171;
    pal[9 + 1] = 146;
    pal[9 + 2] = 142; //  3.
    pal[12 + 0] = 252;
    pal[12 + 1] = 218;
    pal[12 + 2] = 213; //  4.
    pal[15 + 0] = 0;
    pal[15 + 1] = 0;
    pal[15 + 2] = 0; //  5.
    pal[18 + 0] = 213;
    pal[18 + 1] = 148;
    pal[18 + 2] = 139; //  6.
    pal[21 + 0] = 223;
    pal[21 + 1] = 209;
    pal[21 + 2] = 206; //  7.
    pal[24 + 0] = 198;
    pal[24 + 1] = 202;
    pal[24 + 2] = 231; //  8.
    pal[27 + 0] = 255;
    pal[27 + 1] = 255;
    pal[27 + 2] = 255; //  9.
    pal[30 + 0] = 180;
    pal[30 + 1] = 120;
    pal[30 + 2] = 110; //  10.
    pal[39 * 3 + 0] = 0;
    pal[39 * 3 + 1] = 0;
    pal[39 * 3 + 2] = 0; //  39.
    pal[59 * 3 + 0] = 0;
    pal[59 * 3 + 1] = 0;
    pal[59 * 3 + 2] = 0; //  59. Kisabc
    pal[62 * 3 + 0] = 255;
    pal[62 * 3 + 1] = 255;
    pal[62 * 3 + 2] = 255; //  62. Kisabc

    // Kitoltjuk trucolor paletta szineit:
    for (int i = 0; i < 64; i++) {
        int r = i >> 4;
        int g = (i >> 2) & 3;
        int b = i & 3;
        pal[(64 + i) * 3 + 0] = (unsigned char)(r * 64 + 8);
        pal[(64 + i) * 3 + 1] = (unsigned char)(g * 64 + 8);
        pal[(64 + i) * 3 + 2] = (unsigned char)(b * 64 + 8);
    }

    // Kitoltjuk eger szineket:
    for (int i = 0; i < 128; i++) {
        if (pal[3 * i] + pal[3 * i + 1] + pal[3 * i + 2] >= 384) {
            pal[3 * (i + 128)] = pal[3 * (i + 128) + 1] = pal[3 * (i + 128) + 2] = 0;
        } else {
            pal[3 * (i + 128)] = pal[3 * (i + 128) + 1] = pal[3 * (i + 128) + 2] = 255;
        }
    }

    if (EditorPalette) {
        internal_error("create_editor_palette EditorPalette already exists!");
    }
    EditorPalette = new palette(pal);
}

static int completed_levels() {
    player* cur_player = State->get_player(State->player1);
    int skips = 0;
    for (int i = 0; i < cur_player->levels_completed; i++) {
        if (cur_player->skipped[i]) {
            skips++;
        }
    }
    return cur_player->levels_completed - skips;
}

// EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER
// EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER
// EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER
// EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER
// EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER

static int CursorDrawn = 0;

int MouseX = 0, MouseY = 0;
bool CursorShapeIsX = false;

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
    /*if( SelectedTool == Tool_Move && !SelectedPolygon && !SelectedObject && !SelectedSprite &&
    Kurzornegyzet ) {
        //internal_error( "Most itt van benn!" );
        vect2 r = pixel_to_meter( MouseX, MouseY );
        double x1 = r.x;
        double y1 = r.y;
        double x2 = x1+640.0/MetersToPixels;
        double y2 = y1+480.0/MetersToPixels;
        render_line( vect2( x1, y1 ), vect2( x1, y2 ), false );
        render_line( vect2( x1, y2 ), vect2( x2, y2 ), false );
        render_line( vect2( x2, y2 ), vect2( x2, y1 ), false );
        render_line( vect2( x2, y1 ), vect2( x1, y1 ), false );
        return;
    } */

    // Lockolunk is:
    lockfrontbuffer_pic();
    int radius = 4;
    for (int i = -radius; i <= radius; i++) {
        if (CursorShapeIsX) {
            draw_cursor_pixel(MouseX + i, MouseY + i);
            if (i != 0) {
                draw_cursor_pixel(MouseX + i, MouseY - i);
            }
        } else {
            draw_cursor_pixel(MouseX + i, MouseY);
            if (i != 0) {
                draw_cursor_pixel(MouseX, MouseY + i);
            }
        }
    }
    unlockfrontbuffer_pic();
}

// Egeret leveszi kepernyorol:
void erase_cursor() {
    if (!CursorDrawn) {
        internal_error("erase_cursor-ban !CursorDrawn!");
    }
    CursorDrawn = 0;

    draw_cursor_shape();
}

// Egeret felteszi kepernyore:
void draw_cursor() {
    if (CursorDrawn) {
        internal_error("draw_cursor-ban CursorDrawn!");
    }
    CursorDrawn = 1;

    draw_cursor_shape();
}

void update_and_draw_cursor() {
    int mouse_x, mouse_y;
    get_mouse_position(&mouse_x, &mouse_y);
    if (mouse_x == MouseX && mouse_y == MouseY) {
        return;
    }
    erase_cursor();
    MouseX = mouse_x;
    MouseY = mouse_y;
    draw_cursor();
}

// TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP
// TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP
// TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP
// TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP

static char TooltipText[110] = "";

void draw_tooltip(const char* text) {
    if (text) {
        if (strlen(text) > 100) {
            internal_error(std::string("draw_tooltip strlen( text ) > 100!\n") + text);
        }
        strcpy(TooltipText, text);
    }
    int x1 = 1, y1 = 18, x2 = SCREEN_WIDTH - 2, y2 = EDITOR_MENU_Y - 2;
    BufferMain->fill_box(x1, y1, x2, y2, EditorPaletteId_Background);
    EditorWhiteFont->write(BufferMain, 10, y1 + 12, TooltipText);
    bltfront(BufferMain, x1, y1, x2, y2);
}

// KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK
// KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK
// KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK
// KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK

static int RedrawEditorCanvas = 0;
void invalidate_editor_level() { RedrawEditorCanvas = 1; }

static int RedrawEditorGui = 0;
void invalidate_editor_gui() { RedrawEditorGui = 1; }

static int CommandsLength = 13, ToolsLength = 10;
static int MenuLength = CommandsLength + ToolsLength, MenuEntryHeight = 19;

static char MenuText[23][15] = { // Masodik egy string hossza
    "Exit",
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

int SelectedTool = Tool_Move;

static void draw_editor_gui() {
    BufferMain->fill_box(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, EditorPaletteId_Menu);
    // Eloszor is egy nagy keret:
    BufferMain->line(0, 0, SCREEN_WIDTH - 1, 0, EditorPaletteId_MenuBorder);
    BufferMain->line(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1,
                     EditorPaletteId_MenuBorder);
    BufferMain->line(0, 0, 0, SCREEN_HEIGHT - 1, EditorPaletteId_MenuBorder);
    BufferMain->line(SCREEN_WIDTH - 1, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1,
                     EditorPaletteId_MenuBorder);
    // Meg ket belso vonal:
    BufferMain->line(0, EDITOR_MENU_Y - 1, SCREEN_WIDTH - 1, EDITOR_MENU_Y - 1,
                     EditorPaletteId_MenuBorder);
    BufferMain->line(EDITOR_MENU_X - 1, EDITOR_MENU_Y, EDITOR_MENU_X - 1, SCREEN_HEIGHT - 1,
                     EditorPaletteId_MenuBorder);
    for (int i = 0; i < MenuLength; i++) {
        // x1... stb a belso reszt jelenti:
        int x1 = 1;
        int y1 = EDITOR_MENU_Y + i * MenuEntryHeight + 1;
        int x2 = EDITOR_MENU_X - 2;
        int y2 = EDITOR_MENU_Y + (i + 1) * MenuEntryHeight - 1;
        if (i == CommandsLength + SelectedTool) {
            BufferMain->fill_box(x1, y1, x2, y2, EditorPaletteId_MenuSelected);
        }
        BufferMain->line(x1, y2 + 1, x2, y2 + 1, EditorPaletteId_MenuBorder);
        EditorWhiteFont->write(BufferMain, 5, EDITOR_MENU_Y + i * MenuEntryHeight + 14,
                               MenuText[i]);
    }
    // Meg benyomunk par felirat ala vonalakat:
    // Save and Play felirat P-je ala:
    int x = 66;
    int y = EDITOR_MENU_Y + 5 * MenuEntryHeight + 15;
    BufferMain->line(x, y, x + 6, y, 247);
    // Exit felirat x-e ala:
    x = 13;
    y = EDITOR_MENU_Y + 0 * MenuEntryHeight + 15;
    BufferMain->line(x, y, x + 7, y, 247);
    // Open felirat o-ja ala:
    x = 5;
    y = EDITOR_MENU_Y + 2 * MenuEntryHeight + 15;
    BufferMain->line(x, y, x + 6, y, 247);
    // Zoom Out felirat z-je ala:
    x = 5;
    y = EDITOR_MENU_Y + 8 * MenuEntryHeight + 15;
    BufferMain->line(x, y, x + 7, y, 247);
    // Save felirat s-e ala:
    x = 4;
    y = EDITOR_MENU_Y + 4 * MenuEntryHeight + 15;
    BufferMain->line(x, y, x + 7, y, 247);
}

static pic8* TooltipBuffer = nullptr;
// Ezekbe kikereknev() is neha beleir (gany):
static char TooltipPictureName[10] = "", TooltipTextureName[10] = "", TooltipMaskName[10] = "";

void draw_sprite_tooltip(const char* picture_name, const char* texture_name, const char* mask_name,
                         int distance, Clipping clipping) {
    if (strcmp(picture_name, TooltipPictureName) == 0 &&
        strcmp(texture_name, TooltipTextureName) == 0 && strcmp(mask_name, TooltipMaskName) == 0) {
        return;
    }

    strcpy(TooltipPictureName, picture_name);
    strcpy(TooltipTextureName, texture_name);
    strcpy(TooltipMaskName, mask_name);

    if (!TooltipBuffer) {
        TooltipBuffer = new pic8(400, 17);
    }

    TooltipBuffer->fill_box(EditorPaletteId_Background);
    // TooltipBuffer->fill_box( 70 );

    if (picture_name[0] && (texture_name[0] || mask_name[0])) {
        internal_error("draw_sprite_tooltip too many params!");
    }

    if (picture_name[0]) {
        EditorWhiteFont->write(TooltipBuffer, 0, 14, "PICTURE:");
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
        EditorBlackFont->write(TooltipBuffer, 60, 14, tmp);
    }
    if (texture_name[0] || mask_name[0]) {
        EditorWhiteFont->write(TooltipBuffer, 0, 14, "TEXTURE:");
        EditorBlackFont->write(TooltipBuffer, 60, 14, texture_name);
        EditorWhiteFont->write(TooltipBuffer, 126, 14, "MASK:");
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
        EditorBlackFont->write(TooltipBuffer, 164, 14, tmp);
    }

    blit8(BufferMain, TooltipBuffer, 226, 2);
    bltfront(BufferMain, 226, 2, 540, 30);
}

void draw_object_tooltip(object::Type type, object::Property property, int animation) {
    // Belevaltoztatunk draw_sprite_tooltip() valtozojaba, hogy majd toroljon:
    strcpy(TooltipPictureName, "aaccbbdde");

    if (!TooltipBuffer) {
        TooltipBuffer = new pic8(400, 17);
    }

    TooltipBuffer->fill_box(EditorPaletteId_Background);
    // TooltipBuffer->fill_box( 70 );

    EditorWhiteFont->write(TooltipBuffer, 0, 14, "Object:");
    char tmp[50];
    //	object::Type::Exit = 1, object::Type::Food, object::Type::Killer, object::Type::Start };
    // enum { object::Property::NONE, object::Property::GRAVITY_UP, object::Property::GRAVITY_DOWN,
    // object::Property::GRAVITY_LEFT, object::Property::GRAVITY_RIGHT };
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

        EditorWhiteFont->write(TooltipBuffer, 130, 14, "Anim num:");
        char tmp2[10];
        sprintf(tmp2, "%d", (int)(animation + 1));
        EditorBlackFont->write(TooltipBuffer, 193, 14, tmp2);
    }

    if (type == object::Type::Killer) {
        sprintf(tmp, "Killer");
    }

    EditorBlackFont->write(TooltipBuffer, 50, 14, tmp);

    blit8(BufferMain, TooltipBuffer, 226, 2);
    bltfront(BufferMain, 226, 2, 540, 30);
}

static void draw_editor(int draw_gui) {
    erase_cursor();

    TooltipPictureName[0] = TooltipTextureName[0] = TooltipMaskName[0] = 0;

    if (draw_gui) {
        draw_editor_gui();
    }

    BufferMain->fill_box(EDITOR_MENU_X, EDITOR_MENU_Y, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2,
                         EditorPaletteId_Background);
    BufferMain->fill_box(1, 1, SCREEN_WIDTH - 2, EDITOR_MENU_Y - 2, EditorPaletteId_Background);
    // Kiirjuk file nevet:
    EditorWhiteFont->write(BufferMain, 6, 15, "File:");
    strupr(State->editor_filename);
    int len = 0;
    int filename_x = 41;
    if (State->editor_filename[0] == 0) {
        EditorBlackFont->write(BufferMain, filename_x, 15, "UNNAMED");
        len = EditorBlackFont->len("UNNAMED");
    } else {
        EditorBlackFont->write(BufferMain, filename_x, 15, State->editor_filename);
        len = EditorBlackFont->len(State->editor_filename);
    }
    if (LevelChanged) {
        EditorWhiteFont->write(BufferMain, filename_x + len + 3, 15, "(Changed)");
    } else {
        EditorWhiteFont->write(BufferMain, filename_x + len + 3, 15, "(Unchanged)");
    }

    // Kiirjuk sprite allando szoveget ha olyan tool:
    // if( SelectedTool == Tool_Move || SelectedTool == Tool_CreateSprite || SelectedTool ==
    // Tool_DeleteSprite )
    //	EditorWhiteFont->write( BufferMain, 392, 15, "Picture:" );

    // Kiirjuk tool szoveget:
    draw_tooltip();
    // Kiirjuk zoomot:
    EditorWhiteFont->write(BufferMain, 543, 15, "Zoom:");
    char text[30];
    sprintf(text, "%lf", get_zoom());
    // Tizedespont utani jegyek szamat maximaljuk 2-re:
    if (*strstr(text, ".")) {
        *(strstr(text, ".") + 5) = 0; // + 3
    }
    if (strlen(text) < 7) {
        EditorBlackFont->write(BufferMain, 584, 15, text); // 1234.5
    } else {
        EditorBlackFont->write(BufferMain, 580, 15, text); // 10000.0
    }

    RedrawingEditor = 1; // Csak edit bufferbe rajzoljon kirajzol

    Level->render();
    if (SelectedPolygon) {
        // Atalakitjuk szaggatotta amit kell:
        SelectedPolygon->render_one_line(SelectedVertexIndex, CreateVertexDirection, false);
        SelectedPolygon->render_one_line(SelectedVertexIndex, CreateVertexDirection, true);
    }

    RedrawingEditor = 0;

    bltfront(BufferMain);

    RedrawEditorCanvas = 0;
    if (draw_gui) {
        RedrawEditorGui = 0;
    }

    draw_cursor();

    if (SelectedTool == Tool_CreateSprite) {
        if (!Lgr) {
            internal_error("draw_editor !Lgr");
        }
        draw_sprite_tooltip(Lgr->editor_picture_name, Lgr->editor_texture_name,
                            Lgr->editor_mask_name, 0, Clipping::Unclipped);
    }
}

static void draw_mouse_tooltip(int mouse_x, int mouse_y) {
    if (SelectedTool == Tool_CreateFood) {
        draw_object_tooltip(object::Type::Food, DefaultFoodProperty, DefaultFoodAnimation);
        return;
    }
    if (SelectedTool == Tool_CreateSprite) {
        if (!Lgr) {
            internal_error("draw_mouse_tooltip !Lgr");
        }
        draw_sprite_tooltip(Lgr->editor_picture_name, Lgr->editor_texture_name,
                            Lgr->editor_mask_name, 0, Clipping::Unclipped);
        return;
    }

    polygon* poly = nullptr;
    object* obj = nullptr;
    sprite* spr = nullptr;
    bool hover_poly = SelectedTool == Tool_Move || SelectedTool == Tool_CreateVertex ||
                      SelectedTool == Tool_Move || SelectedTool == Tool_DeleteVertex ||
                      SelectedTool == Tool_DeletePolygon;
    bool hover_obj = SelectedTool == Tool_Move || SelectedTool == Tool_DeleteObject;
    bool hover_spr = SelectedTool == Tool_Move || SelectedTool == Tool_DeleteSprite;
    Level->get_closest_entity(mouse_x, mouse_y, hover_poly ? &poly : nullptr, &SelectedVertexIndex,
                              hover_obj ? &obj : nullptr, hover_spr ? &spr : nullptr);

    CursorShapeIsX = poly || obj || spr;
    if (spr) {
        draw_sprite_tooltip(spr->picture_name, spr->texture_name, spr->mask_name, spr->distance,
                            spr->clipping);
    } else if (obj) {
        draw_object_tooltip(obj->type, obj->property, obj->animation);
    } else {
        draw_sprite_tooltip("", "", "", 0, Clipping::Unclipped);
    }
}

static void select_tool(int tool) {
    if (tool == SelectedTool) {
        return;
    }
    SelectedTool = tool;
    draw_tooltip_help();
    // kibalmenu();
    invalidate_editor_gui();
}

// static int Mouseinited = 0;

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

    // Egeret eloszor helyezzuk ki:
    CursorDrawn = 0;
    draw_cursor();

    LevelChanged = 0;

    // Most megnezzuk, hogy megvannak-e palyai fazonnak:
    if (completed_levels() < 5) {
        char tmp[50];
        if (completed_levels() > 1) {
            sprintf(tmp, "You have completed %d levels so far.", completed_levels());
        } else {
            sprintf(tmp, "You have completed %d level so far.", completed_levels());
        }
        dialog("You must complete at least 5 levels before you can use the editor!", tmp);
        return;
    }

    // Beolvassuk file-t:
    editor_window_welcome();

    LevelChanged = 0;
    if (State->editor_filename[0] == 0) {
        load_level_editor(DEFAULT_LEVEL_FILENAME);
    } else {
        if (!load_level_editor(State->editor_filename)) {
            State->editor_filename[0] = 0;
        }
    }

    // Alaphelyzetbe allitunk egy-ket valtozot:
    SelectedPolygon = nullptr;
    SelectedVertexIndex = 0;
    CreateVertexDirection = false;

    SelectedObject = nullptr;
    SelectedSprite = nullptr;

    DefaultFoodProperty = object::Property::None;
    DefaultFoodAnimation = 0;

    SelectedTool = Tool_Move;
    draw_tooltip_help();

    zoom_fill();
    draw_editor(1);

    while (1) {
        handle_events();
        // Control keys
        if (was_key_just_pressed(DIK_ESCAPE)) {
            if (SelectedTool == Tool_Move) {
                tool_move_esc();
            }
            if (SelectedTool == Tool_ZoomIn) {
                tool_zoom_in_esc();
            }
            if (SelectedTool == Tool_CreateVertex) {
                tool_create_vertex_esc();
            }
        }
        if (was_key_just_pressed(DIK_SPACE)) {
            if (SelectedTool == Tool_CreateVertex) {
                tool_create_vertex_space();
            }
        }
        if (was_key_just_pressed(DIK_RETURN)) {
            if (SelectedTool == Tool_CreateVertex) {
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

                if (c == 'i') {
                    platform_save_screenshot();
                }
            }
            // Csak debuggolashoz kellettek:
            /*void saveallapot( void );
            void loadallapot( void );
            if( c == 's' ) {
                saveallapot();
            }
            if( c == 'l' ) {
                loadallapot();
                invalidate_editor_gui();
            } */
        }
        // Megnezzuk eger gombok lemenetelet:
        bool left_click = was_left_mouse_just_clicked();
        bool right_click = was_right_mouse_just_clicked();
        if (left_click || right_click) {

            /*lockfrontbuffer_pic();
            ppixelfront( 300, magassag, 0 );
            unlockfrontbuffer_pic();
            magassag += 2;
            if( magassag > 460 )
                magassag = 460;	*/

            int click_x = 0, click_y = 0;
            get_mouse_position(&click_x, &click_y);
            if (click_y < EDITOR_MENU_Y) {
                click_y = EDITOR_MENU_Y;
            }
            if ((SelectedPolygon || SelectedObject || SelectedSprite || SelectingZoomInBox ||
                 CreatingPolygon) &&
                click_x < EDITOR_MENU_X) {
                // Ha meg van valami fogva, nem mehetunk gombok fole:
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
                // Billentyuk folott tortent lenyomas:
                for (int i = 0; i < MenuLength; i++) {
                    if (EDITOR_MENU_Y + i * MenuEntryHeight <= click_y &&
                        click_y < EDITOR_MENU_Y + (i + 1) * MenuEntryHeight) {
                        // i-edik gombot lenyomtak menuben:
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
                            SelectedTool = Tool_Move;
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
                            // Zoom Fill:
                            zoom_fill();
                            invalidate_editor_level();
                        }
                        if (i == 9 && right_click) {
                            editor_help_zoom_fill();
                        }

                        if (i == 10 && left_click) {
                            editor_window_view_options();
                        }
                        // menu_set_lgr_file();
                        if (i == 10 && right_click) {
                            editor_help_view_options();
                        }

                        if (i == 11) {
                            editor_help();
                        }

                        // Toolok:
                        if (left_click) {
                            if (i >= CommandsLength && i < CommandsLength + ToolsLength) {
                                select_tool(i - CommandsLength);
                            }
                        } else {
                            // Jobb gomb nyomva:
                            if (i == CommandsLength + 0) {
                                editor_help_move();
                            }
                            if (i == CommandsLength + 1) {
                                editor_help_zoom_in();
                            }
                            if (i == CommandsLength + 2) {
                                editor_help_create_vertex();
                            }
                            if (i == CommandsLength + 3) {
                                editor_help_delete_vertex();
                            }
                            if (i == CommandsLength + 4) {
                                editor_help_delete_polygon();
                            }
                            if (i == CommandsLength + 5) {
                                editor_help_create_food();
                            }
                            if (i == CommandsLength + 6) {
                                editor_help_create_killer();
                            }
                            if (i == CommandsLength + 7) {
                                editor_help_delete_object();
                            }
                            if (i == CommandsLength + 8) {
                                editor_help_create_sprite();
                            }
                            if (i == CommandsLength + 9) {
                                editor_help_delete_sprite();
                            }
                        }
                    }
                }
            } else {
                if (SelectedTool == Tool_Move && left_click) {
                    tool_move_leftclick(click_x, click_y);
                }
                if (SelectedTool == Tool_Move && right_click) {
                    // Altalaban tool_move_esc-et hivja:
                    tool_move_rightclick(click_x, click_y);
                }
                if (SelectedTool == Tool_ZoomIn && left_click) {
                    tool_zoom_in_leftclick(click_x, click_y);
                }
                if (SelectedTool == Tool_ZoomIn && right_click) {
                    tool_zoom_in_esc();
                }
                if (SelectedTool == Tool_CreateVertex && left_click) {
                    tool_create_vertex_leftclick(click_x, click_y);
                }
                if (SelectedTool == Tool_CreateVertex && right_click) {
                    tool_create_vertex_esc();
                }
                if (SelectedTool == Tool_DeleteVertex && left_click) {
                    tool_delete_vertex_leftclick(click_x, click_y);
                }
                if (SelectedTool == Tool_DeletePolygon && left_click) {
                    tool_delete_polygon_leftclick(click_x, click_y);
                }
                if (SelectedTool == Tool_CreateFood && left_click) {
                    tool_create_object_leftclick(click_x, click_y, true);
                }
                if (SelectedTool == Tool_CreateFood && right_click) {
                    tool_create_food_rightclick();
                }
                if (SelectedTool == Tool_CreateKiller && left_click) {
                    tool_create_object_leftclick(click_x, click_y, false);
                }
                if (SelectedTool == Tool_DeleteObject && left_click) {
                    tool_delete_object_leftclick(click_x, click_y);
                }
                if (SelectedTool == Tool_CreateSprite && left_click) {
                    tool_create_sprite_leftclick(click_x, click_y);
                }
                if (SelectedTool == Tool_CreateSprite && right_click) {
                    tool_create_sprite_rightclick();
                }
                if (SelectedTool == Tool_DeleteSprite && left_click) {
                    tool_delete_sprite_leftclick(click_x, click_y);
                }
            }
        }

        if (RedrawEditorGui || RedrawEditorCanvas) {
            if (RedrawEditorGui) {
                draw_editor(1);
            } else {
                draw_editor(0);
            }
            // continue;
        }

        // INNENTOL MAR CSAK EGER MOZGATASA AZ ESEMENY:

        int mouse_x = 0, mouse_y = 0;
        get_mouse_position(&mouse_x, &mouse_y);
        if (mouse_y < EDITOR_MENU_Y) {
            mouse_y = EDITOR_MENU_Y;
        }
        if ((SelectedPolygon || SelectedObject || SelectedSprite || SelectingZoomInBox ||
             CreatingPolygon) &&
            mouse_x < EDITOR_MENU_X) {
            // Ha meg van valami fogva, nem mehetunk gombok fole:
            mouse_x = EDITOR_MENU_X;
            set_mouse_position(mouse_x, mouse_y);
        }
        if (mouse_x == MouseX && mouse_y == MouseY) {
            continue;
        }
        // Eger mozdult:
        if (SelectedTool == Tool_ZoomIn) {
            tool_zoom_in_mousemove(mouse_x, mouse_y);
            continue;
        }
        if (SelectedTool == Tool_CreateVertex && (SelectedPolygon || CreatingPolygon)) {
            tool_create_vertex_mousemove(mouse_x, mouse_y);
            continue;
        }
        if (!SelectedPolygon && !SelectedObject && !SelectedSprite) {
            // Ha nincsen megfogva semmi, akkor itt rajzoljuk kurzort:
            erase_cursor();
            draw_mouse_tooltip(mouse_x, mouse_y);
            MouseX = mouse_x;
            MouseY = mouse_y;
            draw_cursor();
        } else {
            // Ha meg van valami fogva, akkor tool rajzolja kurzort:
            if (SelectedTool == Tool_Move) {
                tool_move_mousemove(mouse_x, mouse_y);
            }
        }
    }
}

/*static int Mmagassag = 10;

void pottyrajz( void ) {
    lockfrontbuffer_pic();
    ppixelfront( 400, Mmagassag, 0 );
    unlockfrontbuffer_pic();
    Mmagassag += 2;
    if( Mmagassag > 460 )
        Mmagassag = 460;
} */
