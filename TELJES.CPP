#include "TELJES.H"
#include "abc8.h"
#include "ball_collision.h"
#include "EDITUJ.H"
#include "eol_settings.h"
#include "JATEKOS.H"
#include "KIRAJZOL.H"
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

state* State = nullptr;

// Create a test player with unlocked levels and dummy times
void test_player(void) {
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
    init_shirt();

    init_physics_data();

    // test_player();

    // Load intro.pcx and presumably patch the version number
    Intro = new pic8("intro.pcx");
    blit8(Intro, Intro, 321, 420, 297, 420, 315, 441);
    blit8(Intro, Intro, 321 + 15, 420, 297, 420, 315, 441);
    blit8(Intro, Intro, 321 + 15 + 16, 432, 88, 458, 98, 468);
    spriteosit(Intro);
    pic8* static_intro_screen = new pic8(SCREEN_WIDTH, SCREEN_HEIGHT);
    static_intro_screen->fill_box(BLACK_PALETTE_ID);
    blit8(static_intro_screen, Intro, SCREEN_WIDTH / 2 - Intro->get_width() / 2,
          SCREEN_HEIGHT / 2 - Intro->get_height() / 2);

    // Display intro.pcx
    palette* intro_palette = nullptr;
    get_pcx_pal("intro.pcx", &intro_palette);
    intro_palette->set();
    bltfront(static_intro_screen);

    init_sound();

    // Load globals
    Pabc1 = new abc8("kisbetu1.abc"); // "small letter 1"
    Pabc1->set_spacing(true);
    Pabc2 = new abc8("kisbetu2.abc"); // "small letter 2"
    Pabc2->set_spacing(true);

    Rec1 = new recorder;
    Rec2 = new recorder;

    seteditorpal();

    // Mouse
    int mmx = 355;
    int mmy = 265;
    Moux = mmx;
    Mouy = mmy;
    set_mouse_position(mmx, mmy);
    Moux = mmx;
    Mouy = mmy;

    // Initialize stopwatch, just in case
    stopwatch_reset();

    // Await for key input before scrolling intro.pcx
    while (true) {
        if (has_keypress()) {
            get_keypress();
            break;
        }
        bltfront(static_intro_screen);
    }
    delete static_intro_screen;
    static_intro_screen = nullptr;

    MenuPalette->set();
    delete intro_palette;
    intro_palette = nullptr;

    menu_nav_entries_init();

    if (State->player_count == 0) {
        newjatekos(1, 0);
    } else {
        jatekosvalasztas(1, 0);
    }
    internal_error("menu_intro!");
}

void menu_exit(void) {
    WallsDisabled = true;
    menu_pic* menu = new menu_pic;
    menu->add_line_centered("Thank you for registering the game!", 320, 220);
    menu->add_line_centered("Please do not distribute!", 320, 300);

    empty_keypress_buffer();
    while (true) {
        menu->render();
        if (has_keypress()) {
            Keycode c = get_keypress();
            if (c == ' ' || c == KEY_ENTER || c == KEY_ESC) {
                quit();
            }
        }
    }
}

// Merge two top tens of one level together
static void merge_toptens(topten* src, topten* mrg, bool single) {
    int combined_count = src->times_count + mrg->times_count;

    // Top ten times
    int times[MAX_TIMES * 2];
    // Whether the time comes from state.dat or merge.dat
    bool from_mrg[MAX_TIMES * 2];
    // Player names (two arrays because multiplayer)
    player_name names1[MAX_TIMES * 2];
    player_name names2[MAX_TIMES * 2];

    // state.dat: copy over the top ten data at position 0
    for (int i = 0; i < src->times_count; i++) {
        times[i] = src->times[i];
        from_mrg[i] = false;
        strcpy(names1[i], src->names1[i]);
        if (single) {
            names2[i][0] = 0;
        } else {
            strcpy(names2[i], src->names2[i]);
        }
    }

    // merge.dat: copy over the top ten data, right after the state.dat times
    for (int i = 0; i < mrg->times_count; i++) {
        int idest = src->times_count + i;
        times[idest] = mrg->times[i];
        from_mrg[idest] = true;
        strcpy(names1[idest], mrg->names1[i]);
        if (single) {
            names2[idest][0] = 0;
        } else {
            strcpy(names2[idest], mrg->names2[i]);
        }
    }

    // Bubble sort
    for (int pass = 0; pass < combined_count + 4; pass++) {
        for (int i = 0; i < combined_count - 1; i++) {
            bool swap = false;

            // First sort by time
            if (times[i] > times[i + 1]) {
                swap = true;
            }

            if (times[i] == times[i + 1]) {
                // Then sort by name
                if (strcmp(names1[i], names1[i + 1]) == 0) {
                    if (strcmp(names2[i], names2[i + 1]) == 0) {
                        // Then prioritize state.dat over merge.dat (already identical?)
                        if (from_mrg[i] && !from_mrg[i + 1]) {
                            swap = true;
                        }
                    } else {
                        if (strcmp(names2[i], names2[i + 1]) > 0) {
                            swap = true;
                        }
                    }
                } else {
                    if (strcmp(names1[i], names1[i + 1]) > 0) {
                        swap = true;
                    }
                }
            }

            if (swap) {
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

    // Remove duplicates
    for (int pass = 0; pass < combined_count + 2; pass++) {
        for (int i = 0; i < combined_count - 1; i++) {
            // If all data is identical, and if the time comes from mrg, then delete
            if (times[i] == times[i + 1] && strcmp(names1[i], names1[i + 1]) == 0 &&
                (single || strcmp(names2[i], names2[i + 1]) == 0) && from_mrg[i + 1]) {
                for (int j = i + 1; j < combined_count - 1; j++) {
                    times[j] = times[j + 1];
                    from_mrg[j] = from_mrg[j + 1];
                    strcpy(names1[j], names1[j + 1]);
                    strcpy(names2[j], names2[j + 1]);
                }
                combined_count--;
                break;
            }
        }
    }

    if (combined_count > MAX_TIMES) {
        combined_count = MAX_TIMES;
    }
    src->times_count = combined_count;

    // Clear the best times
    for (int i = 0; i < MAX_TIMES; i++) {
        src->times[i] = -1;
        src->names1[i][0] = 0;
        src->names2[i][0] = 0;
    }

    // Write new data
    for (int i = 0; i < combined_count; i++) {
        src->times[i] = times[i];
        strcpy(src->names1[i], names1[i]);
        strcpy(src->names2[i], names2[i]);
    }
}

// Try to merge merge.dat into state.dat
static void merge_states(void) {
    static const char MERGE_DAT[20] = "merge.dat";
    if (!std::filesystem::exists(MERGE_DAT)) {
        return;
    }

    state* mrg = new state(MERGE_DAT);
    if (!mrg) {
        external_error("memory");
    }

    State->reload_toptens();
    for (int i = 0; i < STATE_LEVEL_COUNT; i++) {
        merge_toptens(&State->toptens[i].single, &mrg->toptens[i].single, true);
        merge_toptens(&State->toptens[i].multi, &mrg->toptens[i].multi, false);
    }

    delete mrg;
    State->save();
}
