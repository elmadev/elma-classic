#include "TELJES.H"
#include "abc8.h"
#include "ball_collision.h"
#include "EDITUJ.H"
#include "JATEKOS.H"
#include "LEJATSZO.H"
#include "M_PIC.H"
#include "main.h"
#include "menu_nav.h"
#include "menu_pic.h"
#include "physics_init.h"
#include "pic8.h"
#include "platform_impl.h"
#include "platform_utils.h"
#include "TOPOL.H"
#include "qopen.h"
#include <cstring>

state* State = NULL;

void cheatplayer(void) {
    // Egyetlen jatekosnak sem szabad lennie:
    if (State->player_count != 0) {
        internal_error("cheatplayer-ben mar van valaki bejegyezve!");
    }

    State->player_count = 1;

    int hanypalya = INTERNAL_LEVEL_COUNT;
    // hanypalya = 40;

    player* pjat = &State->players[0];
    pjat->levels_completed = hanypalya;
    strcpy(pjat->name, "Cheat");
    pjat->selected_level = 0;

    for (int i = 0; i < hanypalya; i++) {
        topten* pidok = &State->toptens[i].single;
        if (pidok->times_count != 0) {
            internal_error("cheatplayer-ben bug!");
        }
        pidok->times_count = 1;
        pidok->times[0] = 180000l;
        strcpy(pidok->names1[0], "Cheat");
    }
}

/*static void kiraklogot( void ) {
    menu_pic szl;

    empty_keypress_buffer();
    while( 1 ) {
        if( has_keypress() ) {
            Keycode c = get_keypress();
            //if( c == KEY_ESC || c == KEY_ENTER )
                return;
        }
        szl.render( 1, 1 );
    }
} */

static void mergestatedat(void);

void teljes(void) {
    init_qopen();

    keys_init();

    init_menu_pictures();

    State = new state;
    if (!State) {
        external_error("memory");
    }

    mergestatedat();

    init_physics_data();

    // State->save(); // Itt lehet ures state.dat file-t csinalni

    // Itt lehet cheat playert eloallitani:
    // Le kell elotte torolni state.dat-ot:
    // cheatplayer();

    // Feltesszuk tile-ot kepre:
    /*pic8* tmpintro = new pic8( 640, 480 );
    pic8* tile = new pic8( "introtl.pcx" );
    int y = 0;
    while( y < 480 ) {
        int x = 0;
        while( x < 640 ) {
            blit8( tmpintro, tile, x, y );
            x += tile->get_width();
        }
        y += tile->get_height();
    }
    pic8* pintro = new pic8( "intro.pcx" );
    spriteosit( pintro );
    blit8( tmpintro, pintro );
    delete pintro; pintro = NULL;
    delete tile; tile = NULL;*/

    // Ezt majd elso kiirloading szabaditja fel (menukep-ben van definialva):
    Intro = new pic8("intro.pcx");
    // Itt most atirjuk verziot 1.1-re:
    // blit8( Intro, Intro, 343, 433, 520, 459, 530, 468 ); // a betu
    blit8(Intro, Intro, 321, 420, 297, 420, 315, 441);
    blit8(Intro, Intro, 321 + 15, 420, 297, 420, 315, 441);
    blit8(Intro, Intro, 321 + 15 + 16, 432, 88, 458, 98, 468);
    spriteosit(Intro);
    pic8* tmpintro = new pic8(SCREEN_WIDTH, SCREEN_HEIGHT); // ezt majd kesobb szabaditjuk fel
    tmpintro->fill_box(BLACK_PALETTE_ID);
    blit8(tmpintro, Intro, SCREEN_WIDTH / 2 - Intro->get_width() / 2,
          SCREEN_HEIGHT / 2 - Intro->get_height() / 2);

    palette* pintropal = NULL;
    // Gany, ezzel az egy nevvel get_pcx_pal(pic.cpp) elso harom byte-ot 0-azza:
    get_pcx_pal("intro.pcx", &pintropal);

    // Hatrebb tettem folyamatos kirakasra (Alt-Tab miatt):
    pintropal->set();
    bltfront(tmpintro);
    delete pintropal;
    pintropal = NULL;

    init_sound();

    // Egy par beolvasas:
    Pabc1 = new abc8("kisbetu1.abc");
    Pabc1->set_spacing(1);
    Pabc2 = new abc8("kisbetu2.abc");
    Pabc2->set_spacing(1);

    Prec1 = new recorder;
    Prec2 = new recorder;

    seteditorpal();

    // Eger:
    int mmx = 355;
    int mmy = 265;
    Moux = mmx;
    Mouy = mmy;
    set_mouse_position(mmx, mmy);
    Moux = mmx;
    Mouy = mmy;

    // Csak hogy mindig menjen stopper:
    stopwatch_reset();

    // Intro keppel varunk, amig nem toltodik be minden:
    while (1) {
        if (has_keypress()) {
            get_keypress();
            break;
        }
        bltfront(tmpintro);
    }
    delete tmpintro;
    tmpintro = NULL;

    /*delete tmpintro;
    tmpintro = new pic8( "torol.pcx" );
    get_keypress();
    for( int i = 1; i < 480; i += 5  ) {
        pbuffer->fill_box( BLACK_PALETTE_ID );
        //blit8( pbuffer, tmpintro, 0, -240+i, 0, 0, 639, 239 );
        //blit8( pbuffer, tmpintro, 0, 479-i, 0, 240, 639, 479 );
        blit8( pbuffer, tmpintro, 0, -480+i );
        blit8( pbuffer, pintro );
        bltfront( pbuffer );
    }
    for( i = 0; i < 480; i += 5  ) {
        blit8( pbuffer, tmpintro );
        //blit8( pbuffer, pintro1, i, 0 );
        //blit8( pbuffer, pintro2, -i, 204 );
        blit8( pbuffer, pintro, 0, i );
        bltfront( pbuffer );
    } */

    MenuPal->set();

    // Ez kell menu_nav-hoz:
    menu_nav_entries_init();

    // kiraklogot();

    if (State->player_count == 0) {
        newjatekos(1, 0); // Innen nem jon vissza
    } else {
        jatekosvalasztas(1, 0); // Innen nem jon vissza
    }
    internal_error("Ide nem kellett volna jonnie!");
}

/* Regi kilepes
void kilep( void ) {
    pic8* ppic = new pic8( "vegekep.pcx" );
    palette* pvegepal = NULL;
    get_pcx_pal( "vegekep.pcx", &pvegepal );
    pvegepal->set();
    bltfront( ppic );
    while( 1 ) {
        Keycode c = get_keypress();
        if( c == ' ' || c == KEY_ENTER || c == KEY_ESC ) {
            quit();
        }
    }
} */

void kilep(void) {
    WallsDisabled = true; // golyok ezentul kimenek falon kivulre
    menu_pic* pszovlist = new menu_pic;
    pszovlist->add_line_centered("Thank you for registering the game!", 320, 220);
    pszovlist->add_line_centered("Please do not distribute!", 320, 300);

    double kezdoido = stopwatch();
    while (1) {
        pszovlist->render();
        double idovaras = 50.0;
        if ((stopwatch() > (kezdoido + idovaras)) && has_keypress()) {
            Keycode c = get_keypress();
            if (c == ' ' || c == KEY_ENTER || c == KEY_ESC) {
                quit();
            }
        }
    }
}

static void mergeegyfele(topten* pst, topten* pmrg, int single) {
    int idokszama = pst->times_count + pmrg->times_count;
    int idok[MAX_TIMES * 2];
    int ujetomb[MAX_TIMES * 2]; // Igaz, ha merge file-bol van ido
    player_name nevek1[MAX_TIMES * 2];
    player_name nevek2[MAX_TIMES * 2];
    // Feltoltjuk tablat:
    for (int i = 0; i < pst->times_count; i++) {
        idok[i] = pst->times[i];
        ujetomb[i] = 0;
        strcpy(nevek1[i], pst->names1[i]);
        if (single) {
            *(nevek2[i]) = 0;
        } else {
            strcpy(nevek2[i], pst->names2[i]);
        }

        // Ezzel minden nem VJ32-es idot 50 percre allitom:
        // if( strcmp( names1[i], "VJ32" ) != 0 && strcmp( names2[i], "VJ32" ) )
        //	idok[i] = 300000;
    }
    for (int i = 0; i < pmrg->times_count; i++) {
        int idest = pst->times_count + i;
        idok[idest] = pmrg->times[i];
        ujetomb[idest] = 1;
        strcpy(nevek1[idest], pmrg->names1[i]);
        if (single) {
            *(nevek2[idest]) = 0;
        } else {
            strcpy(nevek2[idest], pmrg->names2[i]);
        }
    }
    // Rendezzuk oket:
    for (int i = 0; i < idokszama + 4; i++) {
        for (int j = 0; j < idokszama - 1; j++) {
            // Megnezzuk hogy kell-e csere:
            // Eloszor ido szamit, utana elso es masodik nev, majd
            // az hogy merge.dat-bol van-e ido:

            int cserelendo = 0;
            if (idok[j] > idok[j + 1]) {
                cserelendo = 1;
            }

            if (idok[j] == idok[j + 1]) {
                // Idejuk azonos:
                if (strcmp(nevek1[j], nevek1[j + 1]) == 0) {
                    if (strcmp(nevek2[j], nevek2[j + 1]) == 0) {
                        if (ujetomb[j] && !ujetomb[j + 1]) {
                            cserelendo = 1;
                        }
                    } else {
                        // Elso nevig ugyanaz, masodik nev mar nem:
                        if (strcmp(nevek2[j], nevek2[j + 1]) > 0) {
                            cserelendo = 1;
                        }
                    }
                } else {
                    // Bar idejuk azonos, elso nev mar nem:
                    if (strcmp(nevek1[j], nevek1[j + 1]) > 0) {
                        cserelendo = 1;
                    }
                }
            }

            if (cserelendo) {
                // Cserelni kell:
                int tmp = idok[j];
                idok[j] = idok[j + 1];
                idok[j + 1] = tmp;

                tmp = ujetomb[j];
                ujetomb[j] = ujetomb[j + 1];
                ujetomb[j + 1] = tmp;

                player_name tmpnev;
                strcpy(tmpnev, nevek1[j]);
                strcpy(nevek1[j], nevek1[j + 1]);
                strcpy(nevek1[j + 1], tmpnev);

                strcpy(tmpnev, nevek2[j]);
                strcpy(nevek2[j], nevek2[j + 1]);
                strcpy(nevek2[j + 1], tmpnev);
            }
        }
    }

    // Most egyforma idoket egy forma versenyzoktol kiszedjuk:
    // (csak az uj idoket):
    for (int i = 0; i < idokszama + 2; i++) {
        for (int j = 0; j < idokszama - 1; j++) {
            if (idok[j] == idok[j + 1] && strcmp(nevek1[j], nevek1[j + 1]) == 0 &&
                (single || strcmp(nevek2[j], nevek2[j + 1]) == 0) && ujetomb[j + 1]) {

                // Kiszedjuk j+1 -ediket:
                for (int k = j + 1; k < idokszama - 1; k++) {
                    // k+1 -ediket k -adikba tesszuk:
                    idok[k] = idok[k + 1];
                    ujetomb[k] = ujetomb[k + 1];
                    strcpy(nevek1[k], nevek1[k + 1]);
                    strcpy(nevek2[k], nevek2[k + 1]);
                }
                idokszama--;
                break; // csak biztonsagert
            }
        }
    }

    // Visszairjuk state.dat-ba:
    if (idokszama > MAX_TIMES) {
        idokszama = MAX_TIMES;
    }
    pst->times_count = idokszama;

    // Biztonsagi nullazas:
    for (int i = 0; i < MAX_TIMES; i++) {
        pst->times[i] = -1;
        *(pst->names1[i]) = 0;
        *(pst->names2[i]) = 0;
    }
    // Masolas:
    for (int i = 0; i < idokszama; i++) {
        pst->times[i] = idok[i];
        strcpy(pst->names1[i], nevek1[i]);
        strcpy(pst->names2[i], nevek2[i]);
    }
}

static void mergestatedat(void) {
    char nev[20] = "merge.dat";
    if (access(nev, 0) != 0) {
        return; // Nincsen merge file
    }

    state* pmerge = new state(nev);
    if (!pmerge) {
        external_error("memory");
    }

    State->reload_toptens();

    // Most osszefesuljuk besttime tablakat:
    for (int i = 0; i < STATE_LEVEL_COUNT; i++) {
        mergeegyfele(&State->toptens[i].single, &pmerge->toptens[i].single, 1);
        mergeegyfele(&State->toptens[i].multi, &pmerge->toptens[i].multi, 0);
    }

    delete pmerge;

    State->save(); // par sorral feljebb van reload
}
