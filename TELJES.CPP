#include "TELJES.H"
#include "abc8.h"
#include "ball_collision.h"
#include "EDITUJ.H"
#include "eol_settings.h"
#include "JATEKOS.H"
#include "keys.h"
#include "M_PIC.H"
#include "main.h"
#include "menu_nav.h"
#include "menu_pic.h"
#include "physics_init.h"
#include "pic8.h"
#include "platform_impl.h"
#include "recorder.h"
#include "state.h"
#include "qopen.h"
#include <cstring>
#include <filesystem>

state* State = NULL;

void test_player(void) {
    // Egyetlen jatekosnak sem szabad lennie:
    if (State->player_count != 0) {
        internal_error("test_player requires a fresh state file (player_count)!");
    }

    State->player_count = 1;

    int completed_count = INTERNAL_LEVEL_COUNT;

    player* cur_player = &State->players[0];
    cur_player->levels_completed = completed_count;
    strcpy(cur_player->name, "Test");
    cur_player->selected_level = 0;

    for (int i = 0; i < completed_count; i++) {
        topten* tten = &State->toptens[i].single;
        if (tten->times_count != 0) {
            internal_error("test_player requires a fresh state file (times_count)!");
        }
        tten->times_count = 1;
        tten->times[0] = 180000;
        strcpy(tten->names1[0], "Test");
    }
}

static void merge_states(void);

void menu_intro(void) {
    init_qopen();

    init_menu_pictures();

    State = new state;
    if (!State) {
        external_error("memory");
    }

    merge_states();
    eol_settings::sync_controls_to_state(State);

    init_physics_data();

    // test_player();

    // Ezt majd elso kiirloading szabaditja fel (menukep-ben van definialva):
    Intro = new pic8("intro.pcx");
    // Itt most atirjuk verziot 1.1-re:
    // blit8( Intro, Intro, 343, 433, 520, 459, 530, 468 ); // a betu
    blit8(Intro, Intro, 321, 420, 297, 420, 315, 441);
    blit8(Intro, Intro, 321 + 15, 420, 297, 420, 315, 441);
    blit8(Intro, Intro, 321 + 15 + 16, 432, 88, 458, 98, 468);
    spriteosit(Intro);
    pic8* static_intro_screen =
        new pic8(SCREEN_WIDTH, SCREEN_HEIGHT); // ezt majd kesobb szabaditjuk fel
    static_intro_screen->fill_box(BLACK_PALETTE_ID);
    blit8(static_intro_screen, Intro, SCREEN_WIDTH / 2 - Intro->get_width() / 2,
          SCREEN_HEIGHT / 2 - Intro->get_height() / 2);

    palette* intro_palette = NULL;
    // Gany, ezzel az egy nevvel get_pcx_pal(pic.cpp) elso harom byte-ot 0-azza:
    get_pcx_pal("intro.pcx", &intro_palette);

    // Hatrebb tettem folyamatos kirakasra (Alt-Tab miatt):
    intro_palette->set();
    bltfront(static_intro_screen);
    delete intro_palette;
    intro_palette = NULL;

    init_sound();

    // Egy par beolvasas:
    Pabc1 = new abc8("kisbetu1.abc");
    Pabc1->set_spacing(1);
    Pabc2 = new abc8("kisbetu2.abc");
    Pabc2->set_spacing(1);

    Rec1 = new recorder;
    Rec2 = new recorder;

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
        bltfront(static_intro_screen);
    }
    delete static_intro_screen;
    static_intro_screen = NULL;

    MenuPalette->set();

    // Ez kell menu_nav-hoz:
    menu_nav_entries_init();

    if (State->player_count == 0) {
        newjatekos(1, 0); // Innen nem jon vissza
    } else {
        jatekosvalasztas(1, 0); // Innen nem jon vissza
    }
    internal_error("menu_intro!");
}

void menu_exit(void) {
    WallsDisabled = true; // golyok ezentul kimenek falon kivulre
    menu_pic* menu = new menu_pic;
    menu->add_line_centered("Thank you for registering the game!", 320, 220);
    menu->add_line_centered("Please do not distribute!", 320, 300);

    double start_time = stopwatch();
    while (1) {
        menu->render();
        double minimum_delay = 50.0;
        if ((stopwatch() > (start_time + minimum_delay)) && has_keypress()) {
            Keycode c = get_keypress();
            if (c == ' ' || c == KEY_ENTER || c == KEY_ESC) {
                quit();
            }
        }
    }
}

static void merge_toptens(topten* src, topten* mrg, int single) {
    int combined_count = src->times_count + mrg->times_count;
    int times[MAX_TIMES * 2];
    int from_mrg[MAX_TIMES * 2]; // Igaz, ha merge file-bol van ido
    player_name names1[MAX_TIMES * 2];
    player_name names2[MAX_TIMES * 2];
    // Feltoltjuk tablat:
    for (int i = 0; i < src->times_count; i++) {
        times[i] = src->times[i];
        from_mrg[i] = 0;
        strcpy(names1[i], src->names1[i]);
        if (single) {
            *(names2[i]) = 0;
        } else {
            strcpy(names2[i], src->names2[i]);
        }

        // Ezzel minden nem VJ32-es idot 50 percre allitom:
        // if( strcmp( names1[i], "VJ32" ) != 0 && strcmp( names2[i], "VJ32" ) )
        //	times[i] = 300000;
    }
    for (int i = 0; i < mrg->times_count; i++) {
        int idest = src->times_count + i;
        times[idest] = mrg->times[i];
        from_mrg[idest] = 1;
        strcpy(names1[idest], mrg->names1[i]);
        if (single) {
            *(names2[idest]) = 0;
        } else {
            strcpy(names2[idest], mrg->names2[i]);
        }
    }
    // Rendezzuk oket:
    for (int pass = 0; pass < combined_count + 4; pass++) {
        for (int i = 0; i < combined_count - 1; i++) {
            // Megnezzuk hogy kell-e csere:
            // Eloszor ido szamit, utana elso es masodik nev, majd
            // az hogy merge.dat-bol van-e ido:

            int swap = 0;
            if (times[i] > times[i + 1]) {
                swap = 1;
            }

            if (times[i] == times[i + 1]) {
                // Idejuk azonos:
                if (strcmp(names1[i], names1[i + 1]) == 0) {
                    if (strcmp(names2[i], names2[i + 1]) == 0) {
                        if (from_mrg[i] && !from_mrg[i + 1]) {
                            swap = 1;
                        }
                    } else {
                        // Elso nevig ugyanaz, masodik nev mar nem:
                        if (strcmp(names2[i], names2[i + 1]) > 0) {
                            swap = 1;
                        }
                    }
                } else {
                    // Bar idejuk azonos, elso nev mar nem:
                    if (strcmp(names1[i], names1[i + 1]) > 0) {
                        swap = 1;
                    }
                }
            }

            if (swap) {
                // Cserelni kell:
                int tmp = times[i];
                times[i] = times[i + 1];
                times[i + 1] = tmp;

                tmp = from_mrg[i];
                from_mrg[i] = from_mrg[i + 1];
                from_mrg[i + 1] = tmp;

                player_name tmp_name;
                strcpy(tmp_name, names1[i]);
                strcpy(names1[i], names1[i + 1]);
                strcpy(names1[i + 1], tmp_name);

                strcpy(tmp_name, names2[i]);
                strcpy(names2[i], names2[i + 1]);
                strcpy(names2[i + 1], tmp_name);
            }
        }
    }

    // Most egyforma idoket egy forma versenyzoktol kiszedjuk:
    // (csak az uj idoket):
    for (int pass = 0; pass < combined_count + 2; pass++) {
        for (int i = 0; i < combined_count - 1; i++) {
            if (times[i] == times[i + 1] && strcmp(names1[i], names1[i + 1]) == 0 &&
                (single || strcmp(names2[i], names2[i + 1]) == 0) && from_mrg[i + 1]) {

                // Kiszedjuk j+1 -ediket:
                for (int j = i + 1; j < combined_count - 1; j++) {
                    // k+1 -ediket k -adikba tesszuk:
                    times[j] = times[j + 1];
                    from_mrg[j] = from_mrg[j + 1];
                    strcpy(names1[j], names1[j + 1]);
                    strcpy(names2[j], names2[j + 1]);
                }
                combined_count--;
                break; // csak biztonsagert
            }
        }
    }

    // Visszairjuk state.dat-ba:
    if (combined_count > MAX_TIMES) {
        combined_count = MAX_TIMES;
    }
    src->times_count = combined_count;

    // Biztonsagi nullazas:
    for (int i = 0; i < MAX_TIMES; i++) {
        src->times[i] = -1;
        *(src->names1[i]) = 0;
        *(src->names2[i]) = 0;
    }
    // Masolas:
    for (int i = 0; i < combined_count; i++) {
        src->times[i] = times[i];
        strcpy(src->names1[i], names1[i]);
        strcpy(src->names2[i], names2[i]);
    }
}

static void merge_states(void) {
    char merge_dat[20] = "merge.dat";
    if (!std::filesystem::exists(merge_dat)) {
        return; // Nincsen merge file
    }

    state* mrg = new state(merge_dat);
    if (!mrg) {
        external_error("memory");
    }

    State->reload_toptens();

    // Most osszefesuljuk besttime tablakat:
    for (int i = 0; i < STATE_LEVEL_COUNT; i++) {
        merge_toptens(&State->toptens[i].single, &mrg->toptens[i].single, 1);
        merge_toptens(&State->toptens[i].multi, &mrg->toptens[i].multi, 0);
    }

    delete mrg;

    State->save(); // par sorral feljebb van reload
}
