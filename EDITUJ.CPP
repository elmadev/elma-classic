#include "all.h"

topol* Ptop = NULL;

abc8 *Pabc1 = NULL, *Pabc2 = NULL;

gyuru* Pgy = NULL;
int K = 0;
int Fel = 0;

kerek* Pker = NULL;

sprite* Psp = NULL;

int Valtozott = 0;

static unsigned char Menulineszin = 0, Menuszin = 1, Menu_nyomottszin = 2;
unsigned char Hatterindex = 3;
unsigned char Feherszin = 9;
// unsigned char Feketeindex = Hatterindex+128;

ddpal* Pal_editor = NULL;
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

    for (int i = 0; i < 768; i++) {
        pal[i] /= 4;
    }

    if (Pal_editor) {
        hiba("uvcguyew");
    }
    Pal_editor = new ddpal(pal);
}

static int hanypalyavanmeg(void) {
    int jatekosindex = getjatekosindex(State->jatekosa);
    jatekos* pjatekos = &State->jatekosok[jatekosindex];
    int skippelt = 0;
    for (int i = 0; i < pjatekos->sikerespalyakszama; i++) {
        if (pjatekos->skippedtomb[i]) {
            skippelt++;
        }
    }
    return pjatekos->sikerespalyakszama - skippelt;
}

// EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER
// EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER
// EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER
// EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER
// EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER EGER

static int Egerkint = 0;

int Moux = 0, Mouy = 0, Kozelvan = 0;

static void elintezegymousepixelt(int x, int y) {
    if (x < 0 || y < 0 || x > 639 || y > 479) {
        return;
    }
    unsigned char szin = Buffsima->gpixel(x, y);
    szin += 128;
    Buffsima->ppixel(x, y, szin);
    ppixelfront(x, y, szin);
}

static void egeretrajzolja(void) {
    /*if( Tool == T_MOVE && !Pgy && !Pker && !Psp && Kurzornegyzet ) {
        //hiba( "Most itt van benn!" );
        vekt2 r = konv( Moux, Mouy );
        double x1 = r.x;
        double y1 = r.y;
        double x2 = x1+640.0/Arany;
        double y2 = y1+480.0/Arany;
        vonalrajz( vekt2( x1, y1 ), vekt2( x1, y2 ), 0 );
        vonalrajz( vekt2( x1, y2 ), vekt2( x2, y2 ), 0 );
        vonalrajz( vekt2( x2, y2 ), vekt2( x2, y1 ), 0 );
        vonalrajz( vekt2( x2, y1 ), vekt2( x1, y1 ), 0 );
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
        hiba("push-ban !Egerkint!");
    }
    Egerkint = 0;

    egeretrajzolja();
}

// Egeret felteszi kepernyore:
void pop(void) {
    if (Egerkint) {
        hiba("pop-ban Egerkint!");
    }
    Egerkint = 1;

    egeretrajzolja();
}

// TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP
// TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP
// TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP
// TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP TOOLHELP

static char Toolhelpszoveg[110] = "";

void toolhelp(char* text) {
    if (text) {
        if (strlen(text) > 100) {
            hiba("toolhelp-ben strlen( text ) > 100!", text);
        }
        strcpy(Toolhelpszoveg, text);
    }
    int x1 = 1, y1 = 18, x2 = 638, y2 = Savy - 2;
    Buffsima->fillbox(x1, y1, x2, y2, Hatterindex);
    Pabc1->write(Buffsima, 10, y1 + 12, Toolhelpszoveg);
    bltfront(Buffsima, x1, y1, x2, y2);
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
    Buffsima->fillbox(0, 0, 639, 479, Menuszin);
    // Eloszor is egy nagy keret:
    Buffsima->line(0, 0, 639, 0, Menulineszin);
    Buffsima->line(0, 479, 639, 479, Menulineszin);
    Buffsima->line(0, 0, 0, 479, Menulineszin);
    Buffsima->line(639, 0, 639, 479, Menulineszin);
    // Meg ket belso vonal:
    Buffsima->line(0, Savy - 1, 639, Savy - 1, Menulineszin);
    Buffsima->line(Savx - 1, Savy, Savx - 1, 479, Menulineszin);
    for (int i = 0; i < Menuszam; i++) {
        // x1... stb a belso reszt jelenti:
        int x1 = 1;
        int y1 = Savy + i * Menudy + 1;
        int x2 = Savx - 2;
        int y2 = Savy + (i + 1) * Menudy - 1;
        if (i == Elsotool + Tool) {
            Buffsima->fillbox(x1, y1, x2, y2, Menu_nyomottszin);
        }
        Buffsima->line(x1, y2 + 1, x2, y2 + 1, Menulineszin);
        Pabc1->write(Buffsima, 5, Savy + i * Menudy + 14, Menuszovegek[i]);
    }
    // Meg benyomunk par felirat ala vonalakat:
    // Save and Play felirat P-je ala:
    int x = 66;
    int y = Savy + 5 * Menudy + 15;
    Buffsima->line(x, y, x + 6, y, 247);
    // Exit felirat x-e ala:
    x = 13;
    y = Savy + 0 * Menudy + 15;
    Buffsima->line(x, y, x + 7, y, 247);
    // Open felirat o-ja ala:
    x = 5;
    y = Savy + 2 * Menudy + 15;
    Buffsima->line(x, y, x + 6, y, 247);
    // Zoom Out felirat z-je ala:
    x = 5;
    y = Savy + 8 * Menudy + 15;
    Buffsima->line(x, y, x + 7, y, 247);
    // Save felirat s-e ala:
    x = 4;
    y = Savy + 4 * Menudy + 15;
    Buffsima->line(x, y, x + 7, y, 247);
}

static pic8* Pspritenevbuffer = NULL;
// Ezekbe kikereknev() is neha beleir (gany):
static char Kepnevkint[10] = "", Texturanevkint[10] = "", Maszknevkint[10] = "";

void kispritenev(char* kepnev, char* texturanev, char* maszknev, int tav, int hatarol) {
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

    Pspritenevbuffer->fillbox(Hatterindex);
    // Pspritenevbuffer->fillbox( 70 );

    if (kepnev[0] && (texturanev[0] || maszknev[0])) {
        hiba("89hukjk");
    }

    if (kepnev[0]) {
        Pabc1->write(Pspritenevbuffer, 0, 14, "PICTURE:");
        char tmp[30];
        sprintf(tmp, "%s (%d)  ", kepnev, tav);
        if (hatarol == HATAROL_G) {
            strcat(tmp, "G");
        }
        if (hatarol == HATAROL_S) {
            strcat(tmp, "S");
        }
        if (hatarol != HATAROL_G && hatarol != HATAROL_S) {
            strcat(tmp, "U");
        }
        Pabc2->write(Pspritenevbuffer, 60, 14, tmp);
    }
    if (texturanev[0] || maszknev[0]) {
        Pabc1->write(Pspritenevbuffer, 0, 14, "TEXTURE:");
        Pabc2->write(Pspritenevbuffer, 60, 14, texturanev);
        Pabc1->write(Pspritenevbuffer, 126, 14, "MASK:");
        char tmp[30];
        sprintf(tmp, "%s (%d)  ", maszknev, tav);
        if (hatarol == HATAROL_G) {
            strcat(tmp, "G");
        }
        if (hatarol == HATAROL_S) {
            strcat(tmp, "S");
        }
        if (hatarol != HATAROL_G && hatarol != HATAROL_S) {
            strcat(tmp, "U");
        }
        Pabc2->write(Pspritenevbuffer, 164, 14, tmp);
    }

    blt8(Buffsima, Pspritenevbuffer, 226, 2);
    bltfront(Buffsima, 226, 2, 540, 30);
}

void kikereknev(int tipus, int kajatipus, int foodsorszam) {
    // Belevaltoztatunk kispritenev() valtozojaba, hogy majd toroljon:
    strcpy(Kepnevkint, "aaccbbdd");

    if (!Pspritenevbuffer) {
        Pspritenevbuffer = new pic8(400, 17);
    }

    Pspritenevbuffer->fillbox(Hatterindex);
    // Pspritenevbuffer->fillbox( 70 );

    Pabc1->write(Pspritenevbuffer, 0, 14, "Object:");
    char tmp[50];
    //	T_CEL = 1, T_KAJA, T_HALALOS, T_KEZDO };
    // enum { KT_NORMAL, KT_UP, KT_DOWN, KT_LEFT, KT_RIGHT };
    if (tipus == T_CEL) {
        sprintf(tmp, "Exit");
    }
    if (tipus == T_KEZDO) {
        sprintf(tmp, "Start");
    }

    if (tipus == T_KAJA) {
        if (kajatipus == KT_NORMAL) {
            sprintf(tmp, "Food");
        }
        if (kajatipus == KT_UP) {
            sprintf(tmp, "Grav. Up");
        }
        if (kajatipus == KT_DOWN) {
            sprintf(tmp, "Grav. Down");
        }
        if (kajatipus == KT_LEFT) {
            sprintf(tmp, "Grav. Left");
        }
        if (kajatipus == KT_RIGHT) {
            sprintf(tmp, "Grav. Right");
        }

        Pabc1->write(Pspritenevbuffer, 130, 14, "Anim num:");
        char tmp2[10];
        sprintf(tmp2, "%d", (int)(foodsorszam + 1));
        Pabc2->write(Pspritenevbuffer, 193, 14, tmp2);
    }

    if (tipus == T_HALALOS) {
        sprintf(tmp, "Killer");
    }

    Pabc2->write(Pspritenevbuffer, 50, 14, tmp);

    blt8(Buffsima, Pspritenevbuffer, 226, 2);
    bltfront(Buffsima, 226, 2, 540, 30);
}

static void ujrarajzol(int egesz) {
    push();

    Kepnevkint[0] = Texturanevkint[0] = Maszknevkint[0] = 0;

    if (egesz) {
        editbe_balmenu();
    }

    Buffsima->fillbox(Savx, Savy, 638, 478, Hatterindex);
    Buffsima->fillbox(1, 1, 638, Savy - 2, Hatterindex);
    // Kiirjuk file nevet:
    Pabc1->write(Buffsima, 6, 15, "File:");
    strupr(State->editfilenev);
    int len = 0;
    int xfile = 41;
    if (State->editfilenev[0] == 0) {
        Pabc2->write(Buffsima, xfile, 15, "UNNAMED");
        len = Pabc2->len("UNNAMED");
    } else {
        Pabc2->write(Buffsima, xfile, 15, State->editfilenev);
        len = Pabc2->len(State->editfilenev);
    }
    if (Valtozott) {
        Pabc1->write(Buffsima, xfile + len + 3, 15, "(Changed)");
    } else {
        Pabc1->write(Buffsima, xfile + len + 3, 15, "(Unchanged)");
    }

    // Kiirjuk sprite allando szoveget ha olyan tool:
    // if( Tool == T_MOVE || Tool == T_CREATE_SPRITE || Tool == T_DELETE_SPRITE )
    //	Pabc1->write( Buffsima, 392, 15, "Picture:" );

    // Kiirjuk tool szoveget:
    toolhelp();
    // Kiirjuk zoomot:
    Pabc1->write(Buffsima, 543, 15, "Zoom:");
    char text[30];
    sprintf(text, "%lf", getzoomvalue());
    // Tizedespont utani jegyek szamat maximaljuk 2-re:
    if (*strstr(text, ".")) {
        *(strstr(text, ".") + 5) = 0; // + 3
    }
    if (strlen(text) < 7) {
        Pabc2->write(Buffsima, 584, 15, text); // 1234.5
    } else {
        Pabc2->write(Buffsima, 580, 15, text); // 10000.0
    }

    Vonalcsakeditbe = 1; // Csak edit bufferbe rajzoljon kirajzol

    Ptop->kirajzol();
    if (Pgy) {
        // Atalakitjuk szaggatotta amit kell:
        Pgy->kirajz(K, Fel, 0);
        Pgy->kirajz(K, Fel, 1);
    }

    Vonalcsakeditbe = 0;

    bltfront(Buffsima);

    Kellredraw = 0;
    if (egesz) {
        Kellredrawegesz = 0;
    }

    pop();

    if (Tool == T_CREATE_SPRITE) {
        if (!Plgr) {
            hiba("89u5t4");
        }
        kispritenev(Plgr->aktiv_kepnev, Plgr->aktiv_texturanev, Plgr->aktiv_maszknev, 0, 0);
    }
}

static void beallitkozelvane(int mx, int my) {
    if (Tool == T_CREATE_FOOD) {
        kikereknev(T_KAJA, Food_kajatipus, Food_foodsorszam);
        return;
    }
    if (Tool == T_CREATE_SPRITE) {
        if (!Plgr) {
            hiba("7896675");
        }
        kispritenev(Plgr->aktiv_kepnev, Plgr->aktiv_texturanev, Plgr->aktiv_maszknev, 0, 0);
        return;
    }

    double x = konvx_i2d(mx);
    double y = konvy_i2d(my);

    Kozelvan = 0;
    double tav = 1000000.0;
    if (Tool == T_MOVE || Tool == T_CREATE_VERT || Tool == T_MOVE || Tool == T_DELETE_VERT ||
        Tool == T_DELETE_POLY) {
        if (Ptop->legkozpont(x, y, &K, &tav)) {
            Kozelvan = 1;
        }
    }
    kerek* pker = NULL;
    if (Tool == T_MOVE || Tool == T_DELETE_KEREK) {
        double ujtav;
        pker = Ptop->legkozkerek(x, y, &ujtav);
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
        ps = Ptop->legkozsprite(x, y, &ujtav);
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
        if( !Plgr )
            hiba( "89u5t4" );
        if( Plgr->aktivindex >= Plgr->kepszam )
            hiba( "89045t308u" );
        kispritenev( Plgr->nevek[Plgr->aktivindex],
                     Plgr->tavolsagok[Plgr->aktivindex] );
    } */

    if (ps) {
        kispritenev(ps->kepnev, ps->texturanev, ps->maszknev, ps->tavolsag, ps->hatarol);
    }

    if (pker) {
        kikereknev(pker->tipus, pker->kajatipus, pker->foodsorszam);
    }

    if (!ps && !pker) {
        kispritenev("", "", "", 0, 0);
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
    if (!Buffsima) {
        hiba("editor-ban !Buffsima!");
    }

    Pal_editor->set();

    editbe_balmenu();
    bltfront(Buffsima);

    // Egeret eloszor helyezzuk ki:
    Egerkint = 0;
    pop();

    Valtozott = 0;

    // int magassag = 10;
    /*int lenyomva = 0;
    while( 1 ) {
        if( mk_kbhit() )
            if( mk_getextchar() == MK_ESC )
                hiba( "Vege!" );
        int ujlenyomva = getbutbmou();
        if( lenyomva == ujlenyomva )
            continue;

        lenyomva = ujlenyomva;

        lockfrontbuffer_pic();
        if( lenyomva )
            ppixelfront( 300, magassag, 0 );
        else
            ppixelfront( 310, magassag, 0 );
        unlockfrontbuffer_pic();

        magassag += 5;
        if( magassag > 460 )
            magassag = 460;
    }*/

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
    if (State->editfilenev[0] == 0) {
        floadlevel_e("_uj_topol_");
    } else {
        if (!floadlevel_e(State->editfilenev)) {
            State->editfilenev[0] = 0;
        }
    }

    // Alaphelyzetbe allitunk egy-ket valtozot:
    Pgy = NULL;
    K = 0;
    Fel = 0;

    Pker = NULL;
    Psp = NULL;

    Tool = T_MOVE;
    alaphelp();

    kitolt();
    ujrarajzol(1);

    int egerballevoltnyomva = 0; // Eger gombja csak lemenetnel aktivizalodik
    int egerjobblevoltnyomva = 0;
    while (1) {
        while (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC) {
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
            if (c == MK_ENTER) {
                if (Tool == T_CREATE_VERT) {
                    t_create_vert_enter();
                }
            }
            if (!(Pgy || Pker || Psp || Zoomfogva || Egypont)) {
                if (c == 'p') {
                    editplay();
                }
                if (c == 'x') {
                    if (menu_exit()) {
                        push();
                        return;
                    }
                }
                if (c == 'o') {
                    tolt_open();
                }
                if (c == 's') {
                    tolt_save(1);
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
                            Buffsima->save(filenev, Pal_editor_byteok);
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
        int balnyomva = 0;
        int balallapot = getbutbmou();
        if (balallapot && !egerballevoltnyomva) {
            balnyomva = 1;
        }
        egerballevoltnyomva = balallapot;

        int jobbnyomva = 0;
        int jobballapot = getbutjmou();
        if (jobballapot && !egerjobblevoltnyomva) {
            jobbnyomva = 1;
        }
        egerjobblevoltnyomva = jobballapot;

        if (balnyomva || jobbnyomva) {

            // Ez most azert van, hogy ha peldaul jobb
            // gombbal helpet kertunk, es bal gombbal
            // 'OK'-ra kattintva lepunk ki help boxbol
            // aktiv tool ne erzekelje rogton lenyomasnak
            // gombot, hanem elotte egyszer meg fel
            // kelljen azt engedni:
            egerballevoltnyomva = 1;

            /*lockfrontbuffer_pic();
            ppixelfront( 300, magassag, 0 );
            unlockfrontbuffer_pic();
            magassag += 2;
            if( magassag > 460 )
                magassag = 460;	*/

            int ujx = 0, ujy = 0;
            getmou(&ujx, &ujy);
            if (ujy < Savy) {
                ujy = Savy;
            }
            if ((Pgy || Pker || Psp || Zoomfogva || Egypont) && ujx < Savx) {
                // Ha meg van valami fogva, nem mehetunk gombok fole:
                ujx = Savx;
                setmou(ujx, ujy);
            }
            if (ujx != Moux || ujy != Mouy) {
                push();
                Moux = ujx;
                Mouy = ujy;
                pop();
            }
            if (ujx < Savx) {
                // Billentyuk folott tortent lenyomas:
                for (int i = 0; i < Menuszam; i++) {
                    if (Savy + i * Menudy <= ujy && ujy < Savy + (i + 1) * Menudy) {
                        // i-edik gombot lenyomtak menuben:
                        if (i == 0 && balnyomva) {
                            if (menu_exit()) {
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
                            tolt_save_as(1);
                        }
                        if (i == 3 && jobbnyomva) {
                            menuhelp_save_as();
                        }

                        if (i == 4 && balnyomva) {
                            tolt_save(1);
                        }
                        if (i == 4 && jobbnyomva) {
                            menuhelp_save();
                        }

                        if (i == 5 && balnyomva) {
                            editplay();
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
                            kitolt();
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
        getmou(&ujx, &ujy);
        if (ujy < Savy) {
            ujy = Savy;
        }
        if ((Pgy || Pker || Psp || Zoomfogva || Egypont) && ujx < Savx) {
            // Ha meg van valami fogva, nem mehetunk gombok fole:
            ujx = Savx;
            setmou(ujx, ujy);
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
