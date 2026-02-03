#include "EDITUJ.H"
#include "abc8.h"
#include "ED_CHECK.H"
#include "EDITHELP.H"
#include "EDITMENU.H"
#include "editor_canvas.h"
#include "editor_dialog.h"
#include "EDITPLAY.H"
#include "EDITTOLT.H"
#include "EDITTOOL.H"
#include "keys.h"
#include "level.h"
#include "lgr.h"
#include "LOAD.H"
#include "M_PIC.H"
#include "main.h"
#include "menu_pic.h"
#include "object.h"
#include "pic8.h"
#include "platform_impl.h"
#include "platform_utils.h"
#include "polygon.h"
#include "sprite.h"
#include <cstring>
#include <directinput/scancodes.h>

level* Ptop = NULL;

abc8 *Pabc1 = NULL, *Pabc2 = NULL;

polygon* Pgy = NULL;
int K = 0;
bool Fel = false;

object* Pker = NULL;

sprite* Psp = NULL;

int Valtozott = 0;

static unsigned char Menulineszin = 0, Menuszin = 1, Menu_nyomottszin = 2;
unsigned char Hatterindex = 3;
unsigned char Feherszin = 9;
// unsigned char Feketeindex = Hatterindex+128;

palette* Pal_editor = NULL;
static unsigned char Pal_editor_byteok[768];

void seteditorpal(void) {
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

    if (Pal_editor) {
        internal_error("uvcguyew");
    }
    Pal_editor = new palette(pal);
}

static int hanypalyavanmeg(void) {
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

int Moux = 0, Mouy = 0, Kozelvan = 0;

static void elintezegymousepixelt(int x, int y) {
    if (x < 0 || y < 0 || x > (SCREEN_WIDTH - 1) || y > (SCREEN_HEIGHT - 1)) {
        return;
    }
    unsigned char szin = BufferMain->gpixel(x, y);
    szin += 128;
    BufferMain->ppixel(x, y, szin);
    ppixelfront(x, y, szin);
}

static void egeretrajzolja(void) {
    /*if( Tool == T_MOVE && !Pgy && !Pker && !Psp && Kurzornegyzet ) {
        //internal_error( "Most itt van benn!" );
        vect2 r = pixel_to_meter( Moux, Mouy );
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
            elintezegymousepixelt(Moux + i, Mouy + i);
            if (i != 0) {
                elintezegymousepixelt(Moux + i, Mouy - i);
            }
        } else {
            elintezegymousepixelt(Moux + i, Mouy);
            if (i != 0) {
                elintezegymousepixelt(Moux, Mouy + i);
            }
        }
    }
    unlockfrontbuffer_pic();
}

// Egeret leveszi kepernyorol:
void push(void) {
    if (!Egerkint) {
        internal_error("push-ban !Egerkint!");
    }
    Egerkint = 0;

    egeretrajzolja();
}

// Egeret felteszi kepernyore:
void pop(void) {
    if (Egerkint) {
        internal_error("pop-ban Egerkint!");
    }
    Egerkint = 1;

    egeretrajzolja();
}

void update_and_draw_cursor() {
    int x, y;
    get_mouse_position(&x, &y);
    if (x == Moux && y == Mouy) {
        return;
    }
    push();
    Moux = x;
    Mouy = y;
    pop();
}

// TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP
// TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP
// TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP
// TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP

static char Toolhelpszoveg[110] = "";

void toolhelp(const char* text) {
    if (text) {
        if (strlen(text) > 100) {
            internal_error("toolhelp-ben strlen( text ) > 100!", text);
        }
        strcpy(Toolhelpszoveg, text);
    }
    int x1 = 1, y1 = 18, x2 = SCREEN_WIDTH - 2, y2 = EDITOR_MENU_Y - 2;
    BufferMain->fill_box(x1, y1, x2, y2, Hatterindex);
    Pabc1->write(BufferMain, 10, y1 + 12, Toolhelpszoveg);
    bltfront(BufferMain, x1, y1, x2, y2);
}

// KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK
// KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK
// KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK
// KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK KIRAJZOLASOK

static int Kellredraw = 0;
void invalidate(void) { Kellredraw = 1; }

static int Kellredrawegesz = 0;
void invalidateegesz(void) { Kellredrawegesz = 1; }

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

int Tool = T_MOVE;

static void editbe_balmenu(void) {
    BufferMain->fill_box(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, Menuszin);
    // Eloszor is egy nagy keret:
    BufferMain->line(0, 0, SCREEN_WIDTH - 1, 0, Menulineszin);
    BufferMain->line(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, Menulineszin);
    BufferMain->line(0, 0, 0, SCREEN_HEIGHT - 1, Menulineszin);
    BufferMain->line(SCREEN_WIDTH - 1, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, Menulineszin);
    // Meg ket belso vonal:
    BufferMain->line(0, EDITOR_MENU_Y - 1, SCREEN_WIDTH - 1, EDITOR_MENU_Y - 1, Menulineszin);
    BufferMain->line(EDITOR_MENU_X - 1, EDITOR_MENU_Y, EDITOR_MENU_X - 1, SCREEN_HEIGHT - 1,
                     Menulineszin);
    for (int i = 0; i < Menuszam; i++) {
        // x1... stb a belso reszt jelenti:
        int x1 = 1;
        int y1 = EDITOR_MENU_Y + i * Menudy + 1;
        int x2 = EDITOR_MENU_X - 2;
        int y2 = EDITOR_MENU_Y + (i + 1) * Menudy - 1;
        if (i == Elsotool + Tool) {
            BufferMain->fill_box(x1, y1, x2, y2, Menu_nyomottszin);
        }
        BufferMain->line(x1, y2 + 1, x2, y2 + 1, Menulineszin);
        Pabc1->write(BufferMain, 5, EDITOR_MENU_Y + i * Menudy + 14, Menuszovegek[i]);
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

static pic8* Pspritenevbuffer = NULL;
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

    Pspritenevbuffer->fill_box(Hatterindex);
    // Pspritenevbuffer->fill_box( 70 );

    if (kepnev[0] && (texturanev[0] || maszknev[0])) {
        internal_error("89hukjk");
    }

    if (kepnev[0]) {
        Pabc1->write(Pspritenevbuffer, 0, 14, "PICTURE:");
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
        Pabc2->write(Pspritenevbuffer, 60, 14, tmp);
    }
    if (texturanev[0] || maszknev[0]) {
        Pabc1->write(Pspritenevbuffer, 0, 14, "TEXTURE:");
        Pabc2->write(Pspritenevbuffer, 60, 14, texturanev);
        Pabc1->write(Pspritenevbuffer, 126, 14, "MASK:");
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
        Pabc2->write(Pspritenevbuffer, 164, 14, tmp);
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

    Pspritenevbuffer->fill_box(Hatterindex);
    // Pspritenevbuffer->fill_box( 70 );

    Pabc1->write(Pspritenevbuffer, 0, 14, "Object:");
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

        Pabc1->write(Pspritenevbuffer, 130, 14, "Anim num:");
        char tmp2[10];
        sprintf(tmp2, "%d", (int)(foodsorszam + 1));
        Pabc2->write(Pspritenevbuffer, 193, 14, tmp2);
    }

    if (tipus == object::Type::Killer) {
        sprintf(tmp, "Killer");
    }

    Pabc2->write(Pspritenevbuffer, 50, 14, tmp);

    blit8(BufferMain, Pspritenevbuffer, 226, 2);
    bltfront(BufferMain, 226, 2, 540, 30);
}

static void ujrarajzol(int egesz) {
    push();

    Kepnevkint[0] = Texturanevkint[0] = Maszknevkint[0] = 0;

    if (egesz) {
        editbe_balmenu();
    }

    BufferMain->fill_box(EDITOR_MENU_X, EDITOR_MENU_Y, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2,
                         Hatterindex);
    BufferMain->fill_box(1, 1, SCREEN_WIDTH - 2, EDITOR_MENU_Y - 2, Hatterindex);
    // Kiirjuk file nevet:
    Pabc1->write(BufferMain, 6, 15, "File:");
    strupr(State->editor_filename);
    int len = 0;
    int xfile = 41;
    if (State->editor_filename[0] == 0) {
        Pabc2->write(BufferMain, xfile, 15, "UNNAMED");
        len = Pabc2->len("UNNAMED");
    } else {
        Pabc2->write(BufferMain, xfile, 15, State->editor_filename);
        len = Pabc2->len(State->editor_filename);
    }
    if (Valtozott) {
        Pabc1->write(BufferMain, xfile + len + 3, 15, "(Changed)");
    } else {
        Pabc1->write(BufferMain, xfile + len + 3, 15, "(Unchanged)");
    }

    // Kiirjuk sprite allando szoveget ha olyan tool:
    // if( Tool == T_MOVE || Tool == T_CREATE_SPRITE || Tool == T_DELETE_SPRITE )
    //	Pabc1->write( BufferMain, 392, 15, "Picture:" );

    // Kiirjuk tool szoveget:
    toolhelp();
    // Kiirjuk zoomot:
    Pabc1->write(BufferMain, 543, 15, "Zoom:");
    char text[30];
    sprintf(text, "%lf", get_zoom());
    // Tizedespont utani jegyek szamat maximaljuk 2-re:
    if (*strstr(text, ".")) {
        *(strstr(text, ".") + 5) = 0; // + 3
    }
    if (strlen(text) < 7) {
        Pabc2->write(BufferMain, 584, 15, text); // 1234.5
    } else {
        Pabc2->write(BufferMain, 580, 15, text); // 10000.0
    }

    RedrawingEditor = 1; // Csak edit bufferbe rajzoljon kirajzol

    Ptop->render();
    if (Pgy) {
        // Atalakitjuk szaggatotta amit kell:
        Pgy->render_one_line(K, Fel, false);
        Pgy->render_one_line(K, Fel, true);
    }

    RedrawingEditor = 0;

    bltfront(BufferMain);

    Kellredraw = 0;
    if (egesz) {
        Kellredrawegesz = 0;
    }

    pop();

    if (Tool == T_CREATE_SPRITE) {
        if (!Lgr) {
            internal_error("89u5t4");
        }
        kispritenev(Lgr->editor_picture_name, Lgr->editor_texture_name, Lgr->editor_mask_name, 0,
                    Clipping::Unclipped);
    }
}

static void beallitkozelvane(int mx, int my) {
    if (Tool == T_CREATE_FOOD) {
        kikereknev(object::Type::Food, Food_kajatipus, Food_foodsorszam);
        return;
    }
    if (Tool == T_CREATE_SPRITE) {
        if (!Lgr) {
            internal_error("7896675");
        }
        kispritenev(Lgr->editor_picture_name, Lgr->editor_texture_name, Lgr->editor_mask_name, 0,
                    Clipping::Unclipped);
        return;
    }

    double x = pixel_to_meter_x(mx);
    double y = pixel_to_meter_y(my);

    Kozelvan = 0;
    double tav = 1000000.0;
    if (Tool == T_MOVE || Tool == T_CREATE_VERT || Tool == T_MOVE || Tool == T_DELETE_VERT ||
        Tool == T_DELETE_POLY) {
        if (Ptop->get_closest_vertex(x, y, &K, &tav)) {
            Kozelvan = 1;
        }
    }
    object* pker = NULL;
    if (Tool == T_MOVE || Tool == T_DELETE_KEREK) {
        double ujtav;
        pker = Ptop->get_closest_object(x, y, &ujtav);
        if (pker) {
            Kozelvan = 1;
            if (ujtav < tav) {
                tav = ujtav;
            } else {
                pker = NULL;
            }
        }
    }
    sprite* ps = NULL;
    if (Tool == T_MOVE || Tool == T_DELETE_SPRITE) {
        double ujtav;
        ps = Ptop->get_closest_sprite(x, y, &ujtav);
        if (ps) {
            Kozelvan = 1;
            if (ujtav < tav) {
                pker = NULL;
            } else {
                ps = NULL;
            }
        }
    }

    /*if( Tool == T_CREATE_SPRITE ) {
        if( !Lgr )
            internal_error( "89u5t4" );
        if( Lgr->aktivindex >= Lgr->picture_count )
            internal_error( "89045t308u" );
        kispritenev( Lgr->nevek[Lgr->aktivindex],
                     Lgr->tavolsagok[Lgr->aktivindex] );
    } */

    if (ps) {
        kispritenev(ps->picture_name, ps->texture_name, ps->mask_name, ps->distance, ps->clipping);
    }

    if (pker) {
        kikereknev(pker->type, pker->property, pker->animation);
    }

    if (!ps && !pker) {
        kispritenev("", "", "", 0, Clipping::Unclipped);
    }
}

static void toolvaltas(int t) {
    if (t == Tool) {
        return;
    }
    Tool = t;
    alaphelp();
    // kibalmenu();
    invalidateegesz();
}

// static int Mouseinited = 0;

void editor(void) {
    if (!BufferMain) {
        internal_error("editor-ban !BufferMain!");
    }

    Pal_editor->set();

    editbe_balmenu();
    bltfront(BufferMain);

    // Egeret eloszor helyezzuk ki:
    Egerkint = 0;
    pop();

    Valtozott = 0;

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
    kivalasztkezdetit();

    Valtozott = 0;
    if (State->editor_filename[0] == 0) {
        floadlevel_e("_uj_topol_");
    } else {
        if (!floadlevel_e(State->editor_filename)) {
            State->editor_filename[0] = 0;
        }
    }

    // Alaphelyzetbe allitunk egy-ket valtozot:
    Pgy = NULL;
    K = 0;
    Fel = false;

    Pker = NULL;
    Psp = NULL;

    Tool = T_MOVE;
    alaphelp();

    zoom_fill();
    ujrarajzol(1);

    left_mouse_clicked();
    right_mouse_clicked();
    while (1) {
        while (has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC) {
                if (Tool == T_MOVE) {
                    t_move_esc();
                }
                if (Tool == T_ZOOMIN) {
                    t_zoomin_esc();
                }
                if (Tool == T_CREATE_VERT) {
                    t_create_vert_esc();
                }
            }
            if (c == ' ') {
                if (Tool == T_CREATE_VERT) {
                    t_create_vert_space();
                }
                // if( Tool == T_MOVE ) Kurzor negyzette valik merethez
                //	t_move_space();
            }
            if (c == KEY_ENTER) {
                if (Tool == T_CREATE_VERT) {
                    t_create_vert_enter();
                }
            }
            if (!(Pgy || Pker || Psp || Zoomfogva || Egypont)) {
                if (c == 'p') {
                    editplay(is_key_down(DIK_F1));
                }
                if (c == 'x') {
                    if (editor_exit_dialog()) {
                        push();
                        return;
                    }
                }
                if (c == 'o') {
                    tolt_open();
                }
                if (c == 's') {
                    tolt_save();
                }
                if (c == 'z') {
                    menu_zoomout();
                }

                if (c == 'i') {
                    // Kiirjuk kepet:
                    int i = 0;
                    while (1) {
                        char filenev[20];
                        if (i < 10) {
                            sprintf(filenev, "snap0%d.pcx", i);
                        } else {
                            sprintf(filenev, "snap%d.pcx", i);
                        }
                        if (access(filenev, 0) != 0) {
                            // Ez a nev nem foglalt:
                            // forditkepet( ppic );
                            BufferMain->save(filenev, Pal_editor_byteok);
                            // forditkepet( ppic );
                            break;
                        }
                        i++;
                    }
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
                invalidateegesz();
            } */
        }
        // Megnezzuk eger gombok lemenetelet:
        bool balnyomva = left_mouse_clicked();
        bool jobbnyomva = right_mouse_clicked();
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
            if ((Pgy || Pker || Psp || Zoomfogva || Egypont) && ujx < EDITOR_MENU_X) {
                // Ha meg van valami fogva, nem mehetunk gombok fole:
                ujx = EDITOR_MENU_X;
                set_mouse_position(ujx, ujy);
            }
            if (ujx != Moux || ujy != Mouy) {
                push();
                Moux = ujx;
                Mouy = ujy;
                pop();
            }
            if (ujx < EDITOR_MENU_X) {
                // Billentyuk folott tortent lenyomas:
                for (int i = 0; i < Menuszam; i++) {
                    if (EDITOR_MENU_Y + i * Menudy <= ujy &&
                        ujy < EDITOR_MENU_Y + (i + 1) * Menudy) {
                        // i-edik gombot lenyomtak menuben:
                        if (i == 0 && balnyomva) {
                            if (editor_exit_dialog()) {
                                push();
                                return;
                            }
                        }
                        if (i == 0 && jobbnyomva) {
                            menuhelp_exit();
                        }

                        if (i == 1 && balnyomva) {
                            tolt_new();
                        }
                        if (i == 1 && jobbnyomva) {
                            menuhelp_new();
                        }

                        if (i == 2 && balnyomva) {
                            tolt_open();
                        }

                        if (i == 2 && jobbnyomva) {
                            menuhelp_open();
                        }

                        if (i == 3 && balnyomva) {
                            tolt_save_as();
                        }
                        if (i == 3 && jobbnyomva) {
                            menuhelp_save_as();
                        }

                        if (i == 4 && balnyomva) {
                            tolt_save();
                        }
                        if (i == 4 && jobbnyomva) {
                            menuhelp_save();
                        }

                        if (i == 5 && balnyomva) {
                            editplay(is_key_down(DIK_F1));
                            Tool = T_MOVE;
                        }
                        if (i == 5 && jobbnyomva) {
                            menuhelp_play();
                        }

                        if (i == 6 && balnyomva) {
                            check_topology(1);
                        }
                        if (i == 6 && jobbnyomva) {
                            menuhelp_check();
                        }

                        if (i == 7 && balnyomva) {
                            change_level_properties();
                        }
                        if (i == 7 && jobbnyomva) {
                            menuhelp_properties();
                        }

                        if (i == 8 && balnyomva) {
                            menu_zoomout();
                        }
                        if (i == 8 && jobbnyomva) {
                            menuhelp_zoomout();
                        }

                        if (i == 9 && balnyomva) {
                            // Zoom Fill:
                            zoom_fill();
                            invalidate();
                        }
                        if (i == 9 && jobbnyomva) {
                            menuhelp_zoom_fill();
                        }

                        if (i == 10 && balnyomva) {
                            setviewoptions();
                        }
                        // menu_set_lgr_file();
                        if (i == 10 && jobbnyomva) {
                            menuhelp_view();
                        }

                        if (i == 11) {
                            edithelp();
                        }

                        // Toolok:
                        if (balnyomva) {
                            if (i >= Elsotool && i < Elsotool + Toolszam) {
                                toolvaltas(i - Elsotool);
                            }
                        } else {
                            // Jobb gomb nyomva:
                            if (i == Elsotool + 0) {
                                menuhelp_move();
                            }
                            if (i == Elsotool + 1) {
                                menuhelp_zoom_in();
                            }
                            if (i == Elsotool + 2) {
                                menuhelp_create_vertex();
                            }
                            if (i == Elsotool + 3) {
                                menuhelp_delete_vertex();
                            }
                            if (i == Elsotool + 4) {
                                menuhelp_delete_polygon();
                            }
                            if (i == Elsotool + 5) {
                                menuhelp_create_food();
                            }
                            if (i == Elsotool + 6) {
                                menuhelp_create_killer();
                            }
                            if (i == Elsotool + 7) {
                                menuhelp_delete_object();
                            }
                            if (i == Elsotool + 8) {
                                menuhelp_create_sprite();
                            }
                            if (i == Elsotool + 9) {
                                menuhelp_delete_sprite();
                            }
                        }
                    }
                }
            } else {
                if (Tool == T_MOVE && balnyomva) {
                    t_move_nyomva(ujx, ujy);
                }
                if (Tool == T_MOVE && jobbnyomva) {
                    // Altalaban t_move_esc-et hivja:
                    t_move_nyomva_jobb(ujx, ujy);
                }
                if (Tool == T_ZOOMIN && balnyomva) {
                    t_zoomin_nyomva(ujx, ujy);
                }
                if (Tool == T_ZOOMIN && jobbnyomva) {
                    t_zoomin_esc();
                }
                if (Tool == T_CREATE_VERT && balnyomva) {
                    t_create_vert_nyomva(ujx, ujy);
                }
                if (Tool == T_CREATE_VERT && jobbnyomva) {
                    t_create_vert_esc();
                }
                if (Tool == T_DELETE_VERT && balnyomva) {
                    t_delete_vert_nyomva(ujx, ujy);
                }
                if (Tool == T_DELETE_POLY && balnyomva) {
                    t_delete_poly_nyomva(ujx, ujy);
                }
                if (Tool == T_CREATE_FOOD && balnyomva) {
                    t_create_kerek_nyomva(ujx, ujy, 1);
                }
                /* Most nem lehet create food-nal allitani gravitaciot
                if( Tool == T_CREATE_FOOD && jobbnyomva )
                    t_create_food_nyomva_jobb(); */
                if (Tool == T_CREATE_KILLER && balnyomva) {
                    t_create_kerek_nyomva(ujx, ujy, 0);
                }
                if (Tool == T_DELETE_KEREK && balnyomva) {
                    t_delete_kerek_nyomva(ujx, ujy);
                }
                if (Tool == T_CREATE_SPRITE && balnyomva) {
                    t_create_sprite_nyomva(ujx, ujy);
                }
                if (Tool == T_CREATE_SPRITE && jobbnyomva) {
                    t_create_sprite_nyomva_jobb();
                }
                if (Tool == T_DELETE_SPRITE && balnyomva) {
                    t_delete_sprite_nyomva(ujx, ujy);
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
        if ((Pgy || Pker || Psp || Zoomfogva || Egypont) && ujx < EDITOR_MENU_X) {
            // Ha meg van valami fogva, nem mehetunk gombok fole:
            ujx = EDITOR_MENU_X;
            set_mouse_position(ujx, ujy);
        }
        if (ujx == Moux && ujy == Mouy) {
            continue;
        }
        // Eger mozdult:
        if (Tool == T_ZOOMIN) {
            t_zoomin_mmove(ujx, ujy);
            continue;
        }
        if (Tool == T_CREATE_VERT && (Pgy || Egypont)) {
            t_create_vert_mmove(ujx, ujy);
            continue;
        }
        if (!Pgy && !Pker && !Psp) {
            // Ha nincsen megfogva semmi, akkor itt rajzoljuk kurzort:
            push();
            beallitkozelvane(ujx, ujy);
            Moux = ujx;
            Mouy = ujy;
            pop();
        } else {
            // Ha meg van valami fogva, akkor tool rajzolja kurzort:
            if (Tool == T_MOVE) {
                t_move_mmove(ujx, ujy);
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
