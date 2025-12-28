#include "ALL.H"
#include "eol_settings.h"

int Hangmarmegy = 0;

void help(void) {
    menu_pic szl;

    int x1 = 90;
    int x2 = 220;
    int yo = 80;
    int dy = 32;
    // Korny->pabc_help->write( Korny->picbuffer, x1+60, yo,      "Help" );
    szl.add_line_centered("Default controls:", 320, 20);

    szl.add_line("UP", x1, yo);
    szl.add_line("- Accelerate", x2, yo);

    szl.add_line("DOWN", x1, yo + dy);
    szl.add_line("- Block Wheels", x2, yo + dy);

    szl.add_line("LEFT", x1, yo + dy * 2);
    szl.add_line("- Rotate AntiClockwise", x2, yo + dy * 2);

    szl.add_line("RIGHT", x1, yo + dy * 3);
    szl.add_line("- Rotate Clockwise", x2, yo + dy * 3);

    szl.add_line("SPACE", x1, yo + dy * 4);
    szl.add_line("- Turn Around", x2, yo + dy * 4);

    szl.add_line("V", x1, yo + dy * 5);
    szl.add_line("- View Box Toggle", x2, yo + dy * 5);

    szl.add_line("T", x1, yo + dy * 6);
    szl.add_line("- Time Display Toggle", x2, yo + dy * 6);

    szl.add_line_centered("After you have eaten all the fruits,", 320, yo + dy * 9);
    szl.add_line_centered("touch the flower!", 320, yo + dy * 10);

    mk_emptychar();
    while (1) {
        if (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC || c == MK_ENTER) {
                return;
            }
        }
        szl.render();
    }
}

// Egyben allithato:
void options(void) {
    int kurrens = 0;
    while (1) {
        menu_nav val;
        val.selected_index = kurrens;
        val.x_left = 72;
        val.x_right = 390;
        val.y_entries = 77;
        val.dy = 36;
        strcpy(val.title, "Options");

        int vantagis = 0;
        strcpy(NavEntriesLeft[0], "Play mode:");
        if (State->single) {
            strcpy(NavEntriesRight[0], "Single Player");
        } else {
            strcpy(NavEntriesRight[0], "Multiplayer");
            vantagis = 1;
        }

        if (vantagis) {
            strcpy(NavEntriesLeft[1], "Flag Tag:");
            if (State->flag_tag) {
                strcpy(NavEntriesRight[1], "On");
            } else {
                strcpy(NavEntriesRight[1], "Off");
            }
        }

        // Jatekos valasztas:
        strcpy(NavEntriesLeft[1 + vantagis], "Player A:");
        strcpy(NavEntriesRight[1 + vantagis], State->player1);

        strcpy(NavEntriesLeft[2 + vantagis], "Player B:");
        strcpy(NavEntriesRight[2 + vantagis], State->player2);

        strcpy(NavEntriesLeft[3 + vantagis], "Sound:");
        if (State->sound_on) {
            strcpy(NavEntriesRight[3 + vantagis], "Enabled");
        } else {
            strcpy(NavEntriesRight[3 + vantagis], "Disabled");
        }

        strcpy(NavEntriesLeft[4 + vantagis], "Sound Optimization:");
        if (State->compatibility_mode) {
            strcpy(NavEntriesRight[4 + vantagis], "Compatibility");
        } else {
            strcpy(NavEntriesRight[4 + vantagis], "Best Quality");
        }

        strcpy(NavEntriesLeft[5 + vantagis], "Animated Menus:");
        if (State->animated_menus) {
            strcpy(NavEntriesRight[5 + vantagis], "Yes");
        } else {
            strcpy(NavEntriesRight[5 + vantagis], "No");
        }

        strcpy(NavEntriesLeft[6 + vantagis], "Video Detail:");
        if (State->high_quality) {
            strcpy(NavEntriesRight[6 + vantagis], "High");
        } else {
            strcpy(NavEntriesRight[6 + vantagis], "Low");
        }

        strcpy(NavEntriesLeft[7 + vantagis], "Animated Objects:");
        if (State->animated_objects) {
            strcpy(NavEntriesRight[7 + vantagis], "Yes");
        } else {
            strcpy(NavEntriesRight[7 + vantagis], "No");
        }

        strcpy(NavEntriesLeft[8 + vantagis], "Swap Bikes:");
        if (State->player1_bike1) {
            strcpy(NavEntriesRight[8 + vantagis], "No");
        } else {
            strcpy(NavEntriesRight[8 + vantagis], "Yes");
        }

        strcpy(NavEntriesLeft[9 + vantagis], "Customize Controls ...");
        strcpy(NavEntriesRight[9 + vantagis], "");

        strcpy(NavEntriesLeft[10 + vantagis], "Pics In Background:");
        if (EolSettings->pictures_in_background) {
            strcpy(NavEntriesRight[10 + vantagis], "Yes");
        } else {
            strcpy(NavEntriesRight[10 + vantagis], "No");
        }

        strcpy(NavEntriesLeft[11 + vantagis], "Centered Camera:");
        if (EolSettings->center_camera) {
            strcpy(NavEntriesRight[11 + vantagis], "Yes");
        } else {
            strcpy(NavEntriesRight[11 + vantagis], "No");
        }

        strcpy(NavEntriesLeft[12 + vantagis], "Centered Minimap:");
        if (EolSettings->center_map) {
            strcpy(NavEntriesRight[12 + vantagis], "Yes");
        } else {
            strcpy(NavEntriesRight[12 + vantagis], "No");
        }

        strcpy(NavEntriesLeft[13 + vantagis], "Minimap Alignment:");
        switch (EolSettings->map_alignment) {
        case MapAlignment::None:
            strcpy(NavEntriesRight[13 + vantagis], "None");
            break;
        case MapAlignment::Left:
            strcpy(NavEntriesRight[13 + vantagis], "Left");
            break;
        case MapAlignment::Middle:
            strcpy(NavEntriesRight[13 + vantagis], "Middle");
            break;
        case MapAlignment::Right:
            strcpy(NavEntriesRight[13 + vantagis], "Right");
            break;
        }

        val.setup(14 + vantagis, true);

        kurrens = val.navigate();

        if (kurrens < 0) {
            return;
        }

        if (kurrens == 0) {
            State->single = !State->single;
        }

        if (kurrens == 1 && vantagis) {
            State->flag_tag = !State->flag_tag;
        }

        if (vantagis) {
            kurrens -= 1;
        }

        if (kurrens == 1) {
            jatekosvalasztas(1, 1);
        }

        if (kurrens == 2) {
            jatekosvalasztas(0, 1);
        }

        if (kurrens == 3) {
            State->sound_on = !State->sound_on;
        }

        if (kurrens == 4) {
            State->compatibility_mode = !State->compatibility_mode;
        }

        if (kurrens == 5) {
            State->animated_menus = !State->animated_menus;
        }

        if (kurrens == 6) {
            State->high_quality = !State->high_quality;
            invalidate_ptop();
        }

        if (kurrens == 7) {
            State->animated_objects = !State->animated_objects;
        }

        if (kurrens == 8) {
            State->player1_bike1 = !State->player1_bike1;
        }

        if (kurrens == 9) {
            menu_customize_controls();
        }

        if (kurrens == 10) {
            EolSettings->pictures_in_background = !EolSettings->pictures_in_background;
            invalidate_level();
        }

        if (kurrens == 11) {
            EolSettings->center_camera = !EolSettings->center_camera;
        }

        if (kurrens == 12) {
            EolSettings->center_map = !EolSettings->center_map;
        }

        if (kurrens == 13) {
            switch (EolSettings->map_alignment) {
            case MapAlignment::None:
                EolSettings->map_alignment = MapAlignment::Left;
                break;
            case MapAlignment::Left:
                EolSettings->map_alignment = MapAlignment::Middle;
                break;
            case MapAlignment::Middle:
                EolSettings->map_alignment = MapAlignment::Right;
                break;
            case MapAlignment::Right:
                EolSettings->map_alignment = MapAlignment::None;
                break;
            }
        }

        if (vantagis) { // Visszaalitjuk eredeti helyzetebe
            kurrens += 1;
        }
    }
}
