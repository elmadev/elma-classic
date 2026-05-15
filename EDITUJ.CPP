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
        internal_error("uvcguyew");
    }
    EditorPalette = new palette(pal);
}

static int hanypalyavanmeg() {
    player* pjatekos = State->get_player(State->player1);
    int skippelt = 0;
    for (int i = 0; i < pjatekos->levels_completed; i++) {
        if (pjatekos->skipped[i]) {
            skippelt++;
        }
    }
    return pjatekos->levels_completed - skippelt;
}

// EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER
// EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER
// EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER
// EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER
// EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER

static int Egerkint = 0;

int MouseX = 0, MouseY = 0;
bool Kozelvan = false;

static void elintezegymousepixelt(int x, int y) {
    if (x < 0 || y < 0 || x > (SCREEN_WIDTH - 1) || y > (SCREEN_HEIGHT - 1)) {
        return;
    }
    unsigned char szin = BufferMain->gpixel(x, y);
    szin += 128;
    BufferMain->ppixel(x, y, szin);
    ppixelfront(x, y, szin);
}

static void egeretrajzolja() {
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
    int sugar = 4;
    for (int i = -sugar; i <= sugar; i++) {
        if (Kozelvan) {
            elintezegymousepixelt(MouseX + i, MouseY + i);
            if (i != 0) {
                elintezegymousepixelt(MouseX + i, MouseY - i);
            }
        } else {
            elintezegymousepixelt(MouseX + i, MouseY);
            if (i != 0) {
                elintezegymousepixelt(MouseX, MouseY + i);
            }
        }
    }
    unlockfrontbuffer_pic();
}

// Egeret leveszi kepernyorol:
void erase_cursor() {
    if (!Egerkint) {
        internal_error("erase_cursor-ban !Egerkint!");
    }
    Egerkint = 0;

    egeretrajzolja();
}

// Egeret felteszi kepernyore:
void draw_cursor() {
    if (Egerkint) {
        internal_error("draw_cursor-ban Egerkint!");
    }
    Egerkint = 1;

    egeretrajzolja();
}

void update_and_draw_cursor() {
    int x, y;
    get_mouse_position(&x, &y);
    if (x == MouseX && y == MouseY) {
        return;
    }
    erase_cursor();
    MouseX = x;
    MouseY = y;
    draw_cursor();
}

// TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP
// TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP
// TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP
// TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP

static char Toolhelpszoveg[110] = "";

void draw_tooltip(const char* text) {
    if (text) {
        if (strlen(text) > 100) {
            internal_error(std::string("draw_tooltip-ben strlen( text ) > 100!\n") + text);
        }
        strcpy(Toolhelpszoveg, text);
    }
    int x1 = 1, y1 = 18, x2 = SCREEN_WIDTH - 2, y2 = EDITOR_MENU_Y - 2;
    BufferMain->fill_box(x1, y1, x2, y2, EditorPaletteId_Background);
    EditorWhiteFont->write(BufferMain, 10, y1 + 12, Toolhelpszoveg);
    bltfront(BufferMain, x1, y1, x2, y2);
}

// KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK
// KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK
// KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK
// KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK

static int Kellredraw = 0;
void invalidate_editor_level() { Kellredraw = 1; }

static int Kellredrawegesz = 0;
void invalidate_editor_gui() { Kellredrawegesz = 1; }

static int Elsotool = 13, Toolszam = 10;
static int Menuszam = Elsotool + Toolszam, Menudy = 19;

static char Menuszovegek[23][15] = { // Masodik egy string hossza
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

static void editbe_balmenu() {
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
    for (int i = 0; i < Menuszam; i++) {
        // x1... stb a belso reszt jelenti:
        int x1 = 1;
        int y1 = EDITOR_MENU_Y + i * Menudy + 1;
        int x2 = EDITOR_MENU_X - 2;
        int y2 = EDITOR_MENU_Y + (i + 1) * Menudy - 1;
        if (i == Elsotool + SelectedTool) {
            BufferMain->fill_box(x1, y1, x2, y2, EditorPaletteId_MenuSelected);
        }
        BufferMain->line(x1, y2 + 1, x2, y2 + 1, EditorPaletteId_MenuBorder);
        EditorWhiteFont->write(BufferMain, 5, EDITOR_MENU_Y + i * Menudy + 14, Menuszovegek[i]);
    }
    // Meg benyomunk par felirat ala vonalakat:
    // Save and Play felirat P-je ala:
    int x = 66;
    int y = EDITOR_MENU_Y + 5 * Menudy + 15;
    BufferMain->line(x, y, x + 6, y, 247);
    // Exit felirat x-e ala:
    x = 13;
    y = EDITOR_MENU_Y + 0 * Menudy + 15;
    BufferMain->line(x, y, x + 7, y, 247);
    // Open felirat o-ja ala:
    x = 5;
    y = EDITOR_MENU_Y + 2 * Menudy + 15;
    BufferMain->line(x, y, x + 6, y, 247);
    // Zoom Out felirat z-je ala:
    x = 5;
    y = EDITOR_MENU_Y + 8 * Menudy + 15;
    BufferMain->line(x, y, x + 7, y, 247);
    // Save felirat s-e ala:
    x = 4;
    y = EDITOR_MENU_Y + 4 * Menudy + 15;
    BufferMain->line(x, y, x + 7, y, 247);
}

static pic8* Pspritenevbuffer = nullptr;
// Ezekbe kikereknev() is neha beleir (gany):
static char Kepnevkint[10] = "", Texturanevkint[10] = "", Maszknevkint[10] = "";

void kispritenev(const char* kepnev, const char* texturanev, const char* maszknev, int tav,
                 Clipping hatarol) {
    if (strcmp(kepnev, Kepnevkint) == 0 && strcmp(texturanev, Texturanevkint) == 0 &&
        strcmp(maszknev, Maszknevkint) == 0) {
        return;
    }

    strcpy(Kepnevkint, kepnev);
    strcpy(Texturanevkint, texturanev);
    strcpy(Maszknevkint, maszknev);

    if (!Pspritenevbuffer) {
        Pspritenevbuffer = new pic8(400, 17);
    }

    Pspritenevbuffer->fill_box(EditorPaletteId_Background);
    // Pspritenevbuffer->fill_box( 70 );

    if (kepnev[0] && (texturanev[0] || maszknev[0])) {
        internal_error("89hukjk");
    }

    if (kepnev[0]) {
        EditorWhiteFont->write(Pspritenevbuffer, 0, 14, "PICTURE:");
        char tmp[30];
        sprintf(tmp, "%s (%d)  ", kepnev, tav);
        if (hatarol == Clipping::Unclipped) {
            strcat(tmp, "U");
        }
        if (hatarol == Clipping::Ground) {
            strcat(tmp, "G");
        }
        if (hatarol == Clipping::Sky) {
            strcat(tmp, "S");
        }
        EditorBlackFont->write(Pspritenevbuffer, 60, 14, tmp);
    }
    if (texturanev[0] || maszknev[0]) {
        EditorWhiteFont->write(Pspritenevbuffer, 0, 14, "TEXTURE:");
        EditorBlackFont->write(Pspritenevbuffer, 60, 14, texturanev);
        EditorWhiteFont->write(Pspritenevbuffer, 126, 14, "MASK:");
        char tmp[30];
        sprintf(tmp, "%s (%d)  ", maszknev, tav);
        if (hatarol == Clipping::Unclipped) {
            strcat(tmp, "U");
        }
        if (hatarol == Clipping::Ground) {
            strcat(tmp, "G");
        }
        if (hatarol == Clipping::Sky) {
            strcat(tmp, "S");
        }
        EditorBlackFont->write(Pspritenevbuffer, 164, 14, tmp);
    }

    blit8(BufferMain, Pspritenevbuffer, 226, 2);
    bltfront(BufferMain, 226, 2, 540, 30);
}

void kikereknev(object::Type tipus, object::Property kajatipus, int foodsorszam) {
    // Belevaltoztatunk kispritenev() valtozojaba, hogy majd toroljon:
    strcpy(Kepnevkint, "aaccbbdde");

    if (!Pspritenevbuffer) {
        Pspritenevbuffer = new pic8(400, 17);
    }

    Pspritenevbuffer->fill_box(EditorPaletteId_Background);
    // Pspritenevbuffer->fill_box( 70 );

    EditorWhiteFont->write(Pspritenevbuffer, 0, 14, "Object:");
    char tmp[50];
    //	object::Type::Exit = 1, object::Type::Food, object::Type::Killer, object::Type::Start };
    // enum { object::Property::NONE, object::Property::GRAVITY_UP, object::Property::GRAVITY_DOWN,
    // object::Property::GRAVITY_LEFT, object::Property::GRAVITY_RIGHT };
    if (tipus == object::Type::Exit) {
        sprintf(tmp, "Exit");
    }
    if (tipus == object::Type::Start) {
        sprintf(tmp, "Start");
    }

    if (tipus == object::Type::Food) {
        if (kajatipus == object::Property::None) {
            sprintf(tmp, "Food");
        }
        if (kajatipus == object::Property::GravityUp) {
            sprintf(tmp, "Grav. Up");
        }
        if (kajatipus == object::Property::GravityDown) {
            sprintf(tmp, "Grav. Down");
        }
        if (kajatipus == object::Property::GravityLeft) {
            sprintf(tmp, "Grav. Left");
        }
        if (kajatipus == object::Property::GravityRight) {
            sprintf(tmp, "Grav. Right");
        }

        EditorWhiteFont->write(Pspritenevbuffer, 130, 14, "Anim num:");
        char tmp2[10];
        sprintf(tmp2, "%d", (int)(foodsorszam + 1));
        EditorBlackFont->write(Pspritenevbuffer, 193, 14, tmp2);
    }

    if (tipus == object::Type::Killer) {
        sprintf(tmp, "Killer");
    }

    EditorBlackFont->write(Pspritenevbuffer, 50, 14, tmp);

    blit8(BufferMain, Pspritenevbuffer, 226, 2);
    bltfront(BufferMain, 226, 2, 540, 30);
}

static void ujrarajzol(int egesz) {
    erase_cursor();

    Kepnevkint[0] = Texturanevkint[0] = Maszknevkint[0] = 0;

    if (egesz) {
        editbe_balmenu();
    }

    BufferMain->fill_box(EDITOR_MENU_X, EDITOR_MENU_Y, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2,
                         EditorPaletteId_Background);
    BufferMain->fill_box(1, 1, SCREEN_WIDTH - 2, EDITOR_MENU_Y - 2, EditorPaletteId_Background);
    // Kiirjuk file nevet:
    EditorWhiteFont->write(BufferMain, 6, 15, "File:");
    strupr(State->editor_filename);
    int len = 0;
    int xfile = 41;
    if (State->editor_filename[0] == 0) {
        EditorBlackFont->write(BufferMain, xfile, 15, "UNNAMED");
        len = EditorBlackFont->len("UNNAMED");
    } else {
        EditorBlackFont->write(BufferMain, xfile, 15, State->editor_filename);
        len = EditorBlackFont->len(State->editor_filename);
    }
    if (LevelChanged) {
        EditorWhiteFont->write(BufferMain, xfile + len + 3, 15, "(Changed)");
    } else {
        EditorWhiteFont->write(BufferMain, xfile + len + 3, 15, "(Unchanged)");
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

    Kellredraw = 0;
    if (egesz) {
        Kellredrawegesz = 0;
    }

    draw_cursor();

    if (SelectedTool == Tool_CreateSprite) {
        if (!Lgr) {
            internal_error("89u5t4");
        }
        kispritenev(Lgr->editor_picture_name, Lgr->editor_texture_name, Lgr->editor_mask_name, 0,
                    Clipping::Unclipped);
    }
}

static void beallitkozelvane(int mx, int my) {
    if (SelectedTool == Tool_CreateFood) {
        kikereknev(object::Type::Food, DefaultFoodProperty, DefaultFoodAnimation);
        return;
    }
    if (SelectedTool == Tool_CreateSprite) {
        if (!Lgr) {
            internal_error("7896675");
        }
        kispritenev(Lgr->editor_picture_name, Lgr->editor_texture_name, Lgr->editor_mask_name, 0,
                    Clipping::Unclipped);
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
    Level->get_closest_entity(mx, my, hover_poly ? &poly : nullptr, &SelectedVertexIndex,
                              hover_obj ? &obj : nullptr, hover_spr ? &spr : nullptr);

    Kozelvan = poly || obj || spr;
    if (spr) {
        kispritenev(spr->picture_name, spr->texture_name, spr->mask_name, spr->distance,
                    spr->clipping);
    } else if (obj) {
        kikereknev(obj->type, obj->property, obj->animation);
    } else {
        kispritenev("", "", "", 0, Clipping::Unclipped);
    }
}

static void toolvaltas(int t) {
    if (t == SelectedTool) {
        return;
    }
    SelectedTool = t;
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
        internal_error("editor-ban !BufferMain!");
    }

    EditorPalette->set();

    editbe_balmenu();
    bltfront(BufferMain);

    // Egeret eloszor helyezzuk ki:
    Egerkint = 0;
    draw_cursor();

    LevelChanged = 0;

    // Most megnezzuk, hogy megvannak-e palyai fazonnak:
    if (hanypalyavanmeg() < 5) {
        char tmp[50];
        if (hanypalyavanmeg() > 1) {
            sprintf(tmp, "You have completed %d levels so far.", hanypalyavanmeg());
        } else {
            sprintf(tmp, "You have completed %d level so far.", hanypalyavanmeg());
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
    ujrarajzol(1);

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
        bool balnyomva = was_left_mouse_just_clicked();
        bool jobbnyomva = was_right_mouse_just_clicked();
        if (balnyomva || jobbnyomva) {

            /*lockfrontbuffer_pic();
            ppixelfront( 300, magassag, 0 );
            unlockfrontbuffer_pic();
            magassag += 2;
            if( magassag > 460 )
                magassag = 460;	*/

            int ujx = 0, ujy = 0;
            get_mouse_position(&ujx, &ujy);
            if (ujy < EDITOR_MENU_Y) {
                ujy = EDITOR_MENU_Y;
            }
            if ((SelectedPolygon || SelectedObject || SelectedSprite || SelectingZoomInBox ||
                 CreatingPolygon) &&
                ujx < EDITOR_MENU_X) {
                // Ha meg van valami fogva, nem mehetunk gombok fole:
                ujx = EDITOR_MENU_X;
                set_mouse_position(ujx, ujy);
            }
            if (ujx != MouseX || ujy != MouseY) {
                erase_cursor();
                MouseX = ujx;
                MouseY = ujy;
                draw_cursor();
            }
            if (ujx < EDITOR_MENU_X) {
                // Billentyuk folott tortent lenyomas:
                for (int i = 0; i < Menuszam; i++) {
                    if (EDITOR_MENU_Y + i * Menudy <= ujy &&
                        ujy < EDITOR_MENU_Y + (i + 1) * Menudy) {
                        // i-edik gombot lenyomtak menuben:
                        if (i == 0 && balnyomva) {
                            if (editor_dialog_exit()) {
                                erase_cursor();
                                return;
                            }
                        }
                        if (i == 0 && jobbnyomva) {
                            editor_help_exit();
                        }

                        if (i == 1 && balnyomva) {
                            editor_new();
                        }
                        if (i == 1 && jobbnyomva) {
                            editor_help_new();
                        }

                        if (i == 2 && balnyomva) {
                            editor_window_open();
                        }

                        if (i == 2 && jobbnyomva) {
                            editor_help_open();
                        }

                        if (i == 3 && balnyomva) {
                            editor_window_save_as();
                        }
                        if (i == 3 && jobbnyomva) {
                            editor_help_save_as();
                        }

                        if (i == 4 && balnyomva) {
                            editor_window_save();
                        }
                        if (i == 4 && jobbnyomva) {
                            editor_help_save();
                        }

                        if (i == 5 && balnyomva) {
                            editor_play(is_key_down(DIK_F1));
                            SelectedTool = Tool_Move;
                        }
                        if (i == 5 && jobbnyomva) {
                            editor_help_save_and_play();
                        }

                        if (i == 6 && balnyomva) {
                            check_topology(true);
                        }
                        if (i == 6 && jobbnyomva) {
                            editor_help_check_topology();
                        }

                        if (i == 7 && balnyomva) {
                            editor_window_level_properties();
                        }
                        if (i == 7 && jobbnyomva) {
                            editor_help_properties();
                        }

                        if (i == 8 && balnyomva) {
                            editor_zoom_out();
                        }
                        if (i == 8 && jobbnyomva) {
                            editor_help_zoom_out();
                        }

                        if (i == 9 && balnyomva) {
                            // Zoom Fill:
                            zoom_fill();
                            invalidate_editor_level();
                        }
                        if (i == 9 && jobbnyomva) {
                            editor_help_zoom_fill();
                        }

                        if (i == 10 && balnyomva) {
                            editor_window_view_options();
                        }
                        // menu_set_lgr_file();
                        if (i == 10 && jobbnyomva) {
                            editor_help_view_options();
                        }

                        if (i == 11) {
                            editor_help();
                        }

                        // Toolok:
                        if (balnyomva) {
                            if (i >= Elsotool && i < Elsotool + Toolszam) {
                                toolvaltas(i - Elsotool);
                            }
                        } else {
                            // Jobb gomb nyomva:
                            if (i == Elsotool + 0) {
                                editor_help_move();
                            }
                            if (i == Elsotool + 1) {
                                editor_help_zoom_in();
                            }
                            if (i == Elsotool + 2) {
                                editor_help_create_vertex();
                            }
                            if (i == Elsotool + 3) {
                                editor_help_delete_vertex();
                            }
                            if (i == Elsotool + 4) {
                                editor_help_delete_polygon();
                            }
                            if (i == Elsotool + 5) {
                                editor_help_create_food();
                            }
                            if (i == Elsotool + 6) {
                                editor_help_create_killer();
                            }
                            if (i == Elsotool + 7) {
                                editor_help_delete_object();
                            }
                            if (i == Elsotool + 8) {
                                editor_help_create_sprite();
                            }
                            if (i == Elsotool + 9) {
                                editor_help_delete_sprite();
                            }
                        }
                    }
                }
            } else {
                if (SelectedTool == Tool_Move && balnyomva) {
                    tool_move_leftclick(ujx, ujy);
                }
                if (SelectedTool == Tool_Move && jobbnyomva) {
                    // Altalaban tool_move_esc-et hivja:
                    tool_move_rightclick(ujx, ujy);
                }
                if (SelectedTool == Tool_ZoomIn && balnyomva) {
                    tool_zoom_in_leftclick(ujx, ujy);
                }
                if (SelectedTool == Tool_ZoomIn && jobbnyomva) {
                    tool_zoom_in_esc();
                }
                if (SelectedTool == Tool_CreateVertex && balnyomva) {
                    tool_create_vertex_leftclick(ujx, ujy);
                }
                if (SelectedTool == Tool_CreateVertex && jobbnyomva) {
                    tool_create_vertex_esc();
                }
                if (SelectedTool == Tool_DeleteVertex && balnyomva) {
                    tool_delete_vertex_leftclick(ujx, ujy);
                }
                if (SelectedTool == Tool_DeletePolygon && balnyomva) {
                    tool_delete_polygon_leftclick(ujx, ujy);
                }
                if (SelectedTool == Tool_CreateFood && balnyomva) {
                    tool_create_object_leftclick(ujx, ujy, true);
                }
                if (SelectedTool == Tool_CreateFood && jobbnyomva) {
                    tool_create_food_rightclick();
                }
                if (SelectedTool == Tool_CreateKiller && balnyomva) {
                    tool_create_object_leftclick(ujx, ujy, false);
                }
                if (SelectedTool == Tool_DeleteObject && balnyomva) {
                    tool_delete_object_leftclick(ujx, ujy);
                }
                if (SelectedTool == Tool_CreateSprite && balnyomva) {
                    tool_create_sprite_leftclick(ujx, ujy);
                }
                if (SelectedTool == Tool_CreateSprite && jobbnyomva) {
                    tool_create_sprite_rightclick();
                }
                if (SelectedTool == Tool_DeleteSprite && balnyomva) {
                    tool_delete_sprite_leftclick(ujx, ujy);
                }
            }
        }

        if (Kellredrawegesz || Kellredraw) {
            if (Kellredrawegesz) {
                ujrarajzol(1);
            } else {
                ujrarajzol(0);
            }
            // continue;
        }

        // INNENTOL MAR CSAK EGER MOZGATASA AZ ESEMENY:

        int ujx = 0, ujy = 0;
        get_mouse_position(&ujx, &ujy);
        if (ujy < EDITOR_MENU_Y) {
            ujy = EDITOR_MENU_Y;
        }
        if ((SelectedPolygon || SelectedObject || SelectedSprite || SelectingZoomInBox ||
             CreatingPolygon) &&
            ujx < EDITOR_MENU_X) {
            // Ha meg van valami fogva, nem mehetunk gombok fole:
            ujx = EDITOR_MENU_X;
            set_mouse_position(ujx, ujy);
        }
        if (ujx == MouseX && ujy == MouseY) {
            continue;
        }
        // Eger mozdult:
        if (SelectedTool == Tool_ZoomIn) {
            tool_zoom_in_mousemove(ujx, ujy);
            continue;
        }
        if (SelectedTool == Tool_CreateVertex && (SelectedPolygon || CreatingPolygon)) {
            tool_create_vertex_mousemove(ujx, ujy);
            continue;
        }
        if (!SelectedPolygon && !SelectedObject && !SelectedSprite) {
            // Ha nincsen megfogva semmi, akkor itt rajzoljuk kurzort:
            erase_cursor();
            beallitkozelvane(ujx, ujy);
            MouseX = ujx;
            MouseY = ujy;
            draw_cursor();
        } else {
            // Ha meg van valami fogva, akkor tool rajzolja kurzort:
            if (SelectedTool == Tool_Move) {
                tool_move_mousemove(ujx, ujy);
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
