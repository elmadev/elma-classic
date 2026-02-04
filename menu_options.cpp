#include "menu_options.h"
#include "eol_settings.h"
#include "JATEKOS.H"
#include "keys.h"
#include "LOAD.H"
#include "menu_controls.h"
#include "menu_nav.h"
#include "menu_pic.h"
#include "state.h"
#include <cstring>
#include <cmath>

void menu_help() {
    menu_pic menu;

    int x1 = 90;
    int x2 = 220;
    int y0 = 80;
    int dy = 32;
    menu.add_line_centered("Default controls:", 320, 20);

    menu.add_line("UP", x1, y0);
    menu.add_line("- Accelerate", x2, y0);

    menu.add_line("DOWN", x1, y0 + dy);
    menu.add_line("- Block Wheels", x2, y0 + dy);

    menu.add_line("LEFT", x1, y0 + dy * 2);
    menu.add_line("- Rotate AntiClockwise", x2, y0 + dy * 2);

    menu.add_line("RIGHT", x1, y0 + dy * 3);
    menu.add_line("- Rotate Clockwise", x2, y0 + dy * 3);

    menu.add_line("SPACE", x1, y0 + dy * 4);
    menu.add_line("- Turn Around", x2, y0 + dy * 4);

    menu.add_line("V", x1, y0 + dy * 5);
    menu.add_line("- View Box Toggle", x2, y0 + dy * 5);

    menu.add_line("T", x1, y0 + dy * 6);
    menu.add_line("- Time Display Toggle", x2, y0 + dy * 6);

    menu.add_line_centered("After you have eaten all the fruits,", 320, y0 + dy * 9);
    menu.add_line_centered("touch the flower!", 320, y0 + dy * 10);

    empty_keypress_buffer();
    while (true) {
        if (has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC || c == KEY_ENTER) {
                return;
            }
        }
        menu.render();
    }
}

void menu_options() {
    int choice = 0;
    while (true) {
        menu_nav nav;
        nav.selected_index = choice;
        nav.x_left = 72;
        nav.x_right = 390;
        nav.y_entries = 77;
        nav.dy = 36;
        strcpy(nav.title, "Options");

        int flag_tag_opt = 0;
        strcpy(NavEntriesLeft[0], "Play mode:");
        if (State->single) {
            strcpy(NavEntriesRight[0], "Single Player");
        } else {
            strcpy(NavEntriesRight[0], "Multiplayer");
            flag_tag_opt = 1;
        }

        if (flag_tag_opt) {
            strcpy(NavEntriesLeft[1], "Flag Tag:");
            if (State->flag_tag) {
                strcpy(NavEntriesRight[1], "On");
            } else {
                strcpy(NavEntriesRight[1], "Off");
            }
        }

        strcpy(NavEntriesLeft[1 + flag_tag_opt], "Player A:");
        strcpy(NavEntriesRight[1 + flag_tag_opt], State->player1);

        strcpy(NavEntriesLeft[2 + flag_tag_opt], "Player B:");
        strcpy(NavEntriesRight[2 + flag_tag_opt], State->player2);

        strcpy(NavEntriesLeft[3 + flag_tag_opt], "Sound:");
        strcpy(NavEntriesRight[3 + flag_tag_opt], State->sound_on ? "Enabled" : "Disabled");

        strcpy(NavEntriesLeft[4 + flag_tag_opt], "Animated Menus:");
        strcpy(NavEntriesRight[4 + flag_tag_opt], State->animated_menus ? "Yes" : "No");

        strcpy(NavEntriesLeft[5 + flag_tag_opt], "Video Detail:");
        strcpy(NavEntriesRight[5 + flag_tag_opt], State->high_quality ? "High" : "Low");

        strcpy(NavEntriesLeft[6 + flag_tag_opt], "Animated Objects:");
        strcpy(NavEntriesRight[6 + flag_tag_opt], State->animated_objects ? "Yes" : "No");

        strcpy(NavEntriesLeft[7 + flag_tag_opt], "Swap Bikes:");
        strcpy(NavEntriesRight[7 + flag_tag_opt], State->player1_bike1 ? "No" : "Yes");

        strcpy(NavEntriesLeft[8 + flag_tag_opt], "Customize Controls ...");
        strcpy(NavEntriesRight[8 + flag_tag_opt], "");

        strcpy(NavEntriesLeft[9 + flag_tag_opt], "Pics In Background:");
        strcpy(NavEntriesRight[9 + flag_tag_opt],
               EolSettings->pictures_in_background() ? "Yes" : "No");

        strcpy(NavEntriesLeft[10 + flag_tag_opt], "Centered Camera:");
        strcpy(NavEntriesRight[10 + flag_tag_opt], EolSettings->center_camera() ? "Yes" : "No");

        strcpy(NavEntriesLeft[11 + flag_tag_opt], "Centered Minimap:");
        strcpy(NavEntriesRight[11 + flag_tag_opt], EolSettings->center_map() ? "Yes" : "No");

        strcpy(NavEntriesLeft[12 + flag_tag_opt], "Minimap Alignment:");
        switch (EolSettings->map_alignment()) {
        case MapAlignment::None:
            strcpy(NavEntriesRight[12 + flag_tag_opt], "None");
            break;
        case MapAlignment::Left:
            strcpy(NavEntriesRight[12 + flag_tag_opt], "Left");
            break;
        case MapAlignment::Middle:
            strcpy(NavEntriesRight[12 + flag_tag_opt], "Middle");
            break;
        case MapAlignment::Right:
            strcpy(NavEntriesRight[12 + flag_tag_opt], "Right");
            break;
        }

        strcpy(NavEntriesLeft[13 + flag_tag_opt], "Resolution:");
        sprintf(NavEntriesRight[13 + flag_tag_opt], "%dx%d", EolSettings->screen_width(),
                EolSettings->screen_height());

        strcpy(NavEntriesLeft[14 + flag_tag_opt], "Zoom:");
        sprintf(NavEntriesRight[14 + flag_tag_opt], "%.2f", EolSettings->zoom());

        strcpy(NavEntriesLeft[15 + flag_tag_opt], "Zoom Textures:");
        strcpy(NavEntriesRight[15 + flag_tag_opt], EolSettings->zoom_textures() ? "Yes" : "No");

        strcpy(NavEntriesLeft[16 + flag_tag_opt], "Renderer:");
        switch (EolSettings->renderer()) {
        case RendererType::Software:
            strcpy(NavEntriesRight[16 + flag_tag_opt], "Software");
            break;
        case RendererType::OpenGL:
            strcpy(NavEntriesRight[16 + flag_tag_opt], "OpenGL");
            break;
        }

        strcpy(NavEntriesLeft[17 + flag_tag_opt], "Turn Time:");
        if (EolSettings->turn_time() == 0.0) {
            strcpy(NavEntriesRight[17 + flag_tag_opt], "Instant");
        } else {
            sprintf(NavEntriesRight[17 + flag_tag_opt], "%.2fs", EolSettings->turn_time());
        }

        strcpy(NavEntriesLeft[18 + flag_tag_opt], "LCtrl search:");
        strcpy(NavEntriesRight[18 + flag_tag_opt], EolSettings->lctrl_search() ? "Yes" : "No");

        nav.setup(19 + flag_tag_opt, true);

        choice = nav.navigate();

        if (choice < 0) {
            eol_settings::write_settings();
            State->save();
            return;
        }

        if (choice == 0) {
            State->single = !State->single;
        }

        if (choice == 1 && flag_tag_opt) {
            State->flag_tag = !State->flag_tag;
        }

        if (flag_tag_opt) {
            choice -= 1;
        }

        if (choice == 1) {
            jatekosvalasztas(1, 1);
        }

        if (choice == 2) {
            jatekosvalasztas(0, 1);
        }

        if (choice == 3) {
            State->sound_on = !State->sound_on;
        }

        if (choice == 4) {
            State->animated_menus = !State->animated_menus;
        }

        if (choice == 5) {
            State->high_quality = !State->high_quality;
            invalidate_ptop();
        }

        if (choice == 6) {
            State->animated_objects = !State->animated_objects;
        }

        if (choice == 7) {
            State->player1_bike1 = !State->player1_bike1;
        }

        if (choice == 8) {
            menu_customize_controls();
        }

        if (choice == 9) {
            EolSettings->set_pictures_in_background(!EolSettings->pictures_in_background());
            invalidate_level();
        }

        if (choice == 10) {
            EolSettings->set_center_camera(!EolSettings->center_camera());
        }

        if (choice == 11) {
            EolSettings->set_center_map(!EolSettings->center_map());
        }

        if (choice == 12) {
            switch (EolSettings->map_alignment()) {
            case MapAlignment::None:
                EolSettings->set_map_alignment(MapAlignment::Left);
                break;
            case MapAlignment::Left:
                EolSettings->set_map_alignment(MapAlignment::Middle);
                break;
            case MapAlignment::Middle:
                EolSettings->set_map_alignment(MapAlignment::Right);
                break;
            case MapAlignment::Right:
                EolSettings->set_map_alignment(MapAlignment::None);
                break;
            }
        }

        if (choice == 13) {
            switch (EolSettings->screen_width()) {
            case 640: {
                EolSettings->set_screen_width(1024);
                EolSettings->set_screen_height(768);
                break;
            }
            case 1024: {
                EolSettings->set_screen_width(640);
                EolSettings->set_screen_height(480);
                break;
            }
            }
        }

        if (choice == 14) {
            double old_zoom = EolSettings->zoom();
            EolSettings->set_zoom(old_zoom + 0.25);
            if (old_zoom == EolSettings->zoom()) {
                EolSettings->set_zoom(0.25);
            }
        }

        if (choice == 15) {
            EolSettings->set_zoom_textures(!EolSettings->zoom_textures());
        }

        if (choice == 16) {
            switch (EolSettings->renderer()) {
            case RendererType::Software: {
                EolSettings->set_renderer(RendererType::OpenGL);
                break;
            }
            case RendererType::OpenGL: {
                EolSettings->set_renderer(RendererType::Software);
                break;
            }
            }
        }

        if (choice == 17) {
            double old_turn_time = EolSettings->turn_time();
            double new_turn_time = std::round((old_turn_time - 0.10) * 100.0) / 100.0;
            EolSettings->set_turn_time(new_turn_time);
            if (old_turn_time == EolSettings->turn_time()) {
                EolSettings->set_turn_time(0.35);
            }
        }

        if (choice == 18) {
            EolSettings->set_lctrl_search(!EolSettings->lctrl_search());
        }

        if (flag_tag_opt) {
            choice += 1;
        }
    }
}
