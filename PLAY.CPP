#include "PLAY.H"
#include "abc8.h"
#include "best_times.h"
#include "EDITUJ.H"
#include "flagtag.h"
#include "keys.h"
#include "LEJATSZO.H"
#include "level.h"
#include "LOAD.H"
#include "main.h"
#include "menu_external.h"
#include "menu_nav.h"
#include "menu_pic.h"
#include "platform_impl.h"
#include "platform_utils.h"
#include "physics_init.h"
#include "skip.h"
#include "timer.h"
#include <cstdlib>
#include <cstring>

// Siker eseten igazzal ter vissza:
static int menu_prompt_replay_name(char* filename) {
    menu_pic menu;

    int i = 0;
    empty_keypress_buffer();
    int rerender = 1;
    filename[0] = 0;
    while (1) {
        while (has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC) {
                return 0;
            }
            if (c == KEY_ENTER) {
                // ENTER
                if (i > 0) {
                    return 1;
                }
            }
            if (MenuFont->has_char(c) && is_char_valid_for_filename(c)) {
                if (i >= 8) {
                    continue;
                }
                filename[i] = (char)c;
                filename[i + 1] = 0;
                i++;
                rerender = 1;
            }
            if (c == KEY_BACKSPACE) {
                // <- (torles):
                if (i > 0) {
                    i--;
                    filename[i] = 0;
                    rerender = 1;
                }
            }
        }
        if (rerender) {
            rerender = 0;

            menu.clear();

            filename[i] = '_';
            filename[i + 1] = 0;
            menu.add_line_centered(filename, 320, 240);
            filename[i] = 0;

            menu.add_line_centered("Please enter the filename:", 320, 180);
            // menu.add_line_centered( "Press ESC to exit", 320, 300 );
        }
        menu.render();
    }
}

void menu_save_play(int level_id) {
    // Korny->pabc_saveplay->write( Korny->picbuffer, 20, 2*SM, "To exit press ESC!" );
    // Korny->pabc_saveplay->write( Korny->picbuffer, 20, 5*SM, "Please enter filename:" );
    // lassufizre( Korny->picbuffer, Korny->pal_saveplay );
    char tmp[20] = "";
    if (!menu_prompt_replay_name(tmp)) {
        return;
    }
    strcat(tmp, ".rec");
    recorder::save_rec_file(tmp, level_id, State->flag_tag);
}

// Idot beirja tablaba, ha kell:
void update_top_ten(long time, char* time_message, int internal_index,
                    const char* external_filename) {
    int external_level = 0;
    if (external_filename) {
        external_level = 1;
    }

    time_message[0] = 0;
    if (time <= 0) {
        // Nem fejeztek be:
        // Azt jelzi menu_nav::navigate-nak, hogy kozepre tegye:
        strcpy(time_message, MENU_CENTER_TEXT "You Failed to Finish!");
        if (MeghalteloszorAB == 1) {
            strcat(time_message, "    (A died first)");
        }
        if (MeghalteloszorAB == 2) {
            strcat(time_message, "    (B died first)");
        }
        return;
    }

    // Befejeztek, kiirjuk idot:

    if (!external_level) {
        State->reload_toptens();
    }

    char tmp[10];
    centiseconds_to_string(time, tmp);
    if (Single) {
        sprintf(time_message, MENU_CENTER_TEXT "%s", tmp);
    } else {
        if (Aerintetteviragot) {
            sprintf(time_message, MENU_CENTER_TEXT "A:   %s", tmp);
        } else {
            sprintf(time_message, MENU_CENTER_TEXT "B:   %s", tmp);
        }
    }

    // Kivalasztjuk hogy single, vagy multiplayer idok-e:
    topten_set* tten_set;
    if (external_level) {
        tten_set = &Ptop->toptens;
    } else {
        tten_set = &State->toptens[internal_index];
    }

    topten* tten = &tten_set->single;
    if (!Single) {
        tten = &tten_set->multi;
    }

    if (tten->times_count == MAX_TIMES && tten->times[MAX_TIMES - 1] < time) {
        return;
    }

    // Felkerult tablara!
    if (tten->times_count == 0) {
        // Elso ido:
        tten->times_count = 1;
        tten->times[0] = time;
        strcpy(tten->names1[0], State->player1);
        strcpy(tten->names2[0], State->player2);
        strcat(time_message, "     Best Time!");
        if (external_level) {
            Ptop->save_topten(external_filename);
        }
        return;
    }
    // Tablan van mar mas ido is:
    // Uzenetek eldontese:
    int not_last = 0;
    if (tten->times[unsigned(tten->times_count - 1)] > time) {
        // Vkit leelozott:
        not_last = 1;
    }
    if (tten->times[0] > time) {
        // Legjobb ido!:
        strcat(time_message, "     Best Time!");
    } else {
        if (not_last) {
            strcat(time_message, "     You Made the Top Ten");
        }
    }
    // Beillesztjuk uj idot vegere!:
    if (tten->times_count == MAX_TIMES) {
        tten->times[MAX_TIMES - 1] = time;
        strcpy(tten->names1[MAX_TIMES - 1], State->player1);
        strcpy(tten->names2[MAX_TIMES - 1], State->player2);
    } else {
        tten->times[int(tten->times_count)] = time;
        strcpy(tten->names1[int(tten->times_count)], State->player1);
        strcpy(tten->names2[int(tten->times_count)], State->player2);
        tten->times_count++;
    }
    // Sorrendbe allitjuk idoket:
    for (int i = 0; i < MAX_TIMES + 1; i++) {
        for (int j = 0; j < tten->times_count - 1; j++) {
            if (tten->times[j] > tten->times[j + 1]) {
                // Csere:
                long tmp = tten->times[j];
                tten->times[j] = tten->times[j + 1];
                tten->times[j + 1] = tmp;

                player_name tmp_name;
                strcpy(tmp_name, tten->names1[j]);
                strcpy(tten->names1[j], tten->names1[j + 1]);
                strcpy(tten->names1[j + 1], tmp_name);

                strcpy(tmp_name, tten->names2[j]);
                strcpy(tten->names2[j], tten->names2[j + 1]);
                strcpy(tten->names2[j + 1], tmp_name);
            }
        }
    }

    if (external_level) {
        Ptop->save_topten(external_filename);
    }
}

void replay_previous_run(void) {
    floadlevel_p(Rec1->level_filename);
    int reset_player_visibility = 1;
    while (1) {
        Rec1->rewind();
        Rec2->rewind();
        if (lejatszo_r(Rec1->level_filename, !reset_player_visibility)) {
            if (Ptop->objects_flipped) {
                internal_error("replay_previous_run flipped!");
            }
            return;
        }
        reset_player_visibility = 0;
    }
}

// A jelenleg beolvasott palyan jatszik vissza:
// Azt a nevet kerik, amit topol::topol kap:
// BestTime beallitasahoz kell neki:
void replay_from_file(const char* filename) {
    int reset_play_visibility = 1;
    while (1) {
        Rec1->rewind();
        Rec2->rewind();
        if (lejatszo_r(filename, !reset_play_visibility)) {
            if (Ptop->objects_flipped) {
                internal_error("replay_from_file flipped!");
            }
            return;
        }
        if (Ptop->objects_flipped) {
            internal_error("replay_from_file flipped!");
        }
        reset_play_visibility = 0;
    }
}

static text_line ExtraTimeText[14];

// 0-ESC, 1-this level, 2-next level, 3-skip level
// Ha filenev nem NULL, akkor external file:
int menu_level(int internal_index, int nav_on_play_next, const char* time_message,
               const char* external_filename) {
    int external_level = 0;
    if (external_filename) {
        external_level = 1;
    }

    player* player1 = State->get_player(State->player1);
    player* player2 = State->get_player(State->player2);

    // next, skip, kurrens beallitasa:
    int show_play_next = 0;
    int show_skip_level = 0;
    int default_choice = 0;
    if (Single && !external_level) {
        if (player1->levels_completed > internal_index &&
            internal_index < INTERNAL_LEVEL_COUNT - 1) {
            show_play_next = 1;
        }
        if (!show_play_next && internal_index < INTERNAL_LEVEL_COUNT - 1) {
            show_skip_level = 1;
        }
        default_choice = nav_on_play_next;
        // Ellenorzes:
        if (nav_on_play_next && !show_play_next) {
            internal_error("nav_on_play_next && !show_play_next!");
        }
        if (show_skip_level && show_play_next) {
            internal_error("show_skip_level && show_play_next!");
        }
    }

    while (1) {
        char title[100] = "";
        if (external_level) {
            if (strlen(Ptop->level_name) > LEVEL_NAME_LENGTH) {
                internal_error("menu_level level_name too long!");
            }
            sprintf(title, "External: %s", Ptop->level_name);
            if (MenuFont->len(title) > 630) {
                sprintf(title, "Ext: %s", Ptop->level_name);
            }
        } else {
            strcpy(title, "Level ");
            char tmp[10];
            itoa(internal_index + 1, tmp, 10);
            strcat(title, tmp);
            strcat(title, ": ");
            strcat(title, get_internal_level_name(internal_index));
        }

        if (!Single && Tag) {
            if (FlagTagAStarts) {
                sprintf(ExtraTimeText[0].text, MENU_CENTER_TEXT "A start with the flag next.");
            } else {
                sprintf(ExtraTimeText[0].text, MENU_CENTER_TEXT "B start with the flag next.");
            }
        } else {
            strcpy(ExtraTimeText[0].text, time_message);
        }
        ExtraTimeText[0].x = 320;
        if (Single) {
            ExtraTimeText[0].y = 370;
        } else {
            ExtraTimeText[0].y = 314;
        }

        int extra_time_text_length = 1;

        if (Single) {
        } else {
            int is_flagtag = 0;
            if (Tag) {
                is_flagtag = 1;
            }

            int dx = 0;
            if (!is_flagtag) {
                dx = 100; // Ha nem kell fogoidot irni, mindent jobbra tolunk
            }

            int y1 = 380;
            int y2 = 415;
            int long_name = 0;
            if (MenuFont->len(player1->name) > 160 || MenuFont->len(player2->name) > 160) {
                long_name = 1;
            }
            // A jatekos:
            if (long_name) {
                sprintf(ExtraTimeText[1].text, "Player A: %s", player1->name);
            } else {
                sprintf(ExtraTimeText[1].text, "Player A:     %s", player1->name);
            }
            ExtraTimeText[1].x = 10 + dx;
            ExtraTimeText[1].y = y1;

            sprintf(ExtraTimeText[2].text, "%d", Motor1->apple_count);
            ExtraTimeText[2].x = 380 + dx;
            ExtraTimeText[2].y = y1;

            if (is_flagtag) {
                long time = FlagTimeA * TimeToCentiseconds;
                centiseconds_to_string(time, ExtraTimeText[3].text);
                ExtraTimeText[3].x = 440 + dx;
                ExtraTimeText[3].y = y1;
            } else {
                strcpy(ExtraTimeText[3].text, " ");
                ExtraTimeText[3].x = 100 + dx;
                ExtraTimeText[3].y = 100;
            }

            // B jatekos:
            if (long_name) {
                sprintf(ExtraTimeText[4].text, "Player B: %s", player2->name);
            } else {
                sprintf(ExtraTimeText[4].text, "Player B:     %s", player2->name);
            }
            ExtraTimeText[4].x = 10 + dx;
            ExtraTimeText[4].y = y2;

            sprintf(ExtraTimeText[5].text, "%d", Motor2->apple_count);
            ExtraTimeText[5].x = 380 + dx;
            ExtraTimeText[5].y = y2;

            if (is_flagtag) {
                long time = FlagTimeB * TimeToCentiseconds;
                centiseconds_to_string(time, ExtraTimeText[6].text);
                ExtraTimeText[6].x = 440 + dx;
                ExtraTimeText[6].y = y2;
            }
            if (is_flagtag) {
                extra_time_text_length += 6;
            } else {
                extra_time_text_length += 5;
            }

            if (long_name) {
                ExtraTimeText[2].x += 40;
                ExtraTimeText[3].x += 40;
                ExtraTimeText[5].x += 40;
                ExtraTimeText[6].x += 40;
            }
        }

        // Valasztas:
        menu_nav nav;
        // nav.szam = 5+show_play_next+show_skip_level;
        nav.selected_index = default_choice;
        nav.x_left = 230;
        if (Single) {
            nav.y_entries = 110;
            nav.dy = 42;
        } else {
            nav.y_entries = 110;
            nav.dy = 42;
        }
        strcpy(nav.title, title);

        if (show_play_next || show_skip_level) {
            strcpy(NavEntriesLeft[0], "Play again");
            if (show_play_next) {
                strcpy(NavEntriesLeft[1], "Play next");
            } else {
                strcpy(NavEntriesLeft[1], "Skip level");
            }
            strcpy(NavEntriesLeft[2], "Replay");
            strcpy(NavEntriesLeft[3], "Save play");
            strcpy(NavEntriesLeft[4], "Best times");
            // strcpy( NavEntriesLeft[5], "Main menu" );
        } else {
            strcpy(NavEntriesLeft[0], "Play again");
            strcpy(NavEntriesLeft[1], "Replay");
            strcpy(NavEntriesLeft[2], "Save play");
            strcpy(NavEntriesLeft[3], "Best times");
            // strcpy( NavEntriesLeft[4], "Main menu" );
        }

        nav.setup(4 + show_play_next + show_skip_level);

        // Idoszoveg helyere beirjuk ecset-ben levok lefoglaltak szamat:
        // sprintf( ExtraTimeText[0].text, "%d", Osszegszam );

        int choice = nav.navigate(ExtraTimeText, extra_time_text_length);
        default_choice = choice;
        // choice-t show_play_next esetere konvertalja:
        if (choice > 0 && !show_play_next && !show_skip_level) {
            choice++;
        }

        if (choice < 0 /*|| choice == 5*/) {
            return 0;
        }
        if (choice == 0) {
            return 1;
        }
        if (choice == 1) {
            // Next, vagy skip:
            if (!show_play_next && !show_skip_level) {
                internal_error("choice == 1 && (!show_play_next && !show_skip_level)!");
            }
            if (show_play_next) {
                return 2;
            }
            // SKIP:
            if (is_skippable(internal_index)) {
                return 3;
            }
        }
        if (choice == 2) {
            replay_previous_run();
            MenuPalette->set();
        }
        if (choice == 3) {
            if (internal_index != INTERNAL_LEVEL_COUNT - 1) { // Utolso palyat nem lehet menteni
                menu_save_play(Ptop->level_id);
            }
        }
        if (choice == 4) {
            if (external_level) {
                menu_external_topten(Ptop, Single);
            } else {
                menu_internal_topten(internal_index, Single);
            }
        }
    }
}

void loading_screen(void) {
    menu_pic menu;
    menu.clear();
    menu.add_line_centered("Loading", 320, 230);
    menu.render(true); // Golyok nelkul
}

static void play_internal(int internal_index) {
    player* cur_player = State->get_player(State->player1);
    while (1) {
        char filename[20];
        if (internal_index + 1 < 10) {
            sprintf(filename, "QWQUU00%d.LEV", internal_index + 1);
        } else {
            if (internal_index + 1 < 100) {
                sprintf(filename, "QWQUU0%d.LEV", internal_index + 1);
            } else {
                sprintf(filename, "QWQUU%d.LEV", internal_index + 1);
            }
        }
        loading_screen();
        floadlevel_p(filename);

        Rec1->erase(filename);
        Rec2->erase(filename);
        long time = lejatszo(filename, F1Pressed ? CameraMode::MapViewer : CameraMode::Normal);
        MenuPalette->set();
        if (Ptop->objects_flipped) {
            internal_error("play_internal objects_flipped!");
        }
        char time_message[100] = "";
        update_top_ten(time, time_message, internal_index, NULL);

        int unlocked_new_level = 0;
        if (time > 0) {
            // Sikeres volt szint:
            cur_player->skipped[internal_index] = 0;
            if (cur_player->levels_completed == internal_index && Single) {
                // Csak SINGLE jatek alatt novelheto levels_completed!!!
                // Novelte eggyel sikeres palyak szamat:
                cur_player->levels_completed++;
                if (cur_player->levels_completed < INTERNAL_LEVEL_COUNT) {
                    unlocked_new_level = 1;
                }
            }
            // update_top_ten ido > 0 es nem external eseten reload-olja:
            State->save();
        }
        int choice = menu_level(internal_index, unlocked_new_level, time_message, NULL);
        Rec1->erase(filename);
        Rec2->erase(filename);
        if (choice == 0) {
            cur_player->selected_level = internal_index + unlocked_new_level;
            return;
        }
        if (choice == 2) {
            internal_index++;
            cur_player->selected_level = internal_index;
        }
        if (choice == 3) {
            // Skip level:
            if (internal_index != cur_player->levels_completed) {
                internal_error("internal_index != cur_player->levels_completed!");
            }
            cur_player->skipped[internal_index] = 1;
            internal_index++;
            cur_player->selected_level = internal_index;
            cur_player->levels_completed++;
            State->reload_toptens();
            State->save();
        }
    }
}

void menu_play(void) {
    while (1) {
        player* player1 = State->get_player(State->player1);
        int levels_completed = player1->levels_completed + 1;
        if (!State->single) {
            // Ha ketten jatszanak, akkor a nagyobb sikeru szamit:
            player* player2 = State->get_player(State->player2);
            if (levels_completed < player2->levels_completed + 1) {
                levels_completed = player2->levels_completed + 1;
            }
        }

        if (levels_completed > INTERNAL_LEVEL_COUNT) {
            levels_completed = INTERNAL_LEVEL_COUNT;
        }

        // Valasztas:
        menu_nav nav;
        nav.search_pattern = SearchPattern::Internals;
        nav.selected_index = int(player1->selected_level) + 1;
        strcpy(nav.title, "Select Level!");

        strcpy(NavEntriesLeft[0], "External File");
        for (int i = 0; i < levels_completed; i++) {
            char level_name[NAV_ENTRY_TEXT_MAX_LENGTH];
            itoa(i + 1, level_name, 10);
            strcat(level_name, " ");
            if (player1->skipped[i]) {
                strcat(level_name, "SKIPPED!");
            } else {
                strcat(level_name, get_internal_level_name(i));
                // strcat( level_name, "Nincs level nev!" );
            }
            if (strlen(level_name) > NAV_ENTRY_TEXT_MAX_LENGTH - 5) {
                internal_error("menu_play strlen( level_name ) > NAV_ENTRY_TEXT_MAX_LENGTH-5!");
            }
            strcpy(NavEntriesLeft[i + 1], level_name);
        }

        nav.setup(levels_completed + 1);
        int choice = nav.navigate();

        if (choice < 0) {
            return;
        }

        if (choice == 0) {
            // External file lejatszast valasztotta, ilyenkor ha visszajon
            // folytatodik tovabb:
            player1->selected_level = -1;
            menu_external_levels();
        } else {
            // Egy normal level-t valasztott ki:
            play_internal(choice - 1);
            // return;
        }
    }
}
