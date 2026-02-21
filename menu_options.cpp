#include "menu_options.h"
#include "eol_settings.h"
#include "fs_utils.h"
#include "JATEKOS.H"
#include "level_load.h"
#include "menu_controls.h"
#include "menu_nav.h"
#include "menu_pic.h"
#include "platform_utils.h"
#include "state.h"
#include <algorithm>
#include <cstring>
#include <cmath>
#include <format>
#include <vector>

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

    menu.loop();
}

static void menu_lgr() {
    menu_nav nav("Pick an LGR!");
    nav.search_pattern = SearchPattern::Sorted;

    finame filename;
    bool done = find_first("lgr/*.lgr", filename);
    while (!done) {
        constexpr int LGR_EXT_LEN = 4;
        int len = strlen(filename);
        std::string lgrname = std::string(filename, len - LGR_EXT_LEN);

        nav.add_row(lgrname, NAV_FUNC() { EolSettings->set_default_lgr_name(left); });

        done = find_next(filename);
    }
    find_close();

    nav.sort_rows();
    nav.select_row(EolSettings->default_lgr_name());

    nav.navigate();
}

void menu_options() {
    int choice = 0;
    while (true) {
        menu_nav nav("Options");
        nav.select_row(choice);
        nav.x_left = 72;
        nav.x_right = 390;
        nav.y_entries = 77;
        nav.dy = 36;

        nav.add_row(
            "Play mode:", State->single ? "Single Player" : "Multiplayer",
            NAV_FUNC() { State->single = !State->single; });

        if (!State->single) {
            nav.add_row(
                "Flag Tag:", State->flag_tag ? "On" : "Off",
                NAV_FUNC() { State->flag_tag = !State->flag_tag; });
        }

        nav.add_row("Player A:", State->player1, NAV_FUNC() { jatekosvalasztas(1, 1); });

        nav.add_row("Player B:", State->player2, NAV_FUNC() { jatekosvalasztas(0, 1); });

        nav.add_row(
            "Sound:", State->sound_on ? "Enabled" : "Disabled",
            NAV_FUNC() { State->sound_on = !State->sound_on; });

        nav.add_row(
            "Animated Menus:", State->animated_menus ? "Yes" : "No",
            NAV_FUNC() { State->animated_menus = !State->animated_menus; });

        nav.add_row(
            "Video Detail:", State->high_quality ? "High" : "Low", NAV_FUNC() {
                State->high_quality = !State->high_quality;
                invalidate_level();
            });

        nav.add_row(
            "Animated Objects:", State->animated_objects ? "Yes" : "No",
            NAV_FUNC() { State->animated_objects = !State->animated_objects; });

        nav.add_row(
            "Still Objects:", EolSettings->still_objects() ? "Yes" : "No",
            NAV_FUNC() { EolSettings->set_still_objects(!EolSettings->still_objects()); });

        nav.add_row(
            "Swap Bikes:", State->player1_bike1 ? "No" : "Yes",
            NAV_FUNC() { State->player1_bike1 = !State->player1_bike1; });

        nav.add_row("Customize Controls ...", NAV_FUNC() { menu_customize_controls(); });

        nav.add_row(
            "Pics In Background:", EolSettings->pictures_in_background() ? "Yes" : "No",
            NAV_FUNC() {
                EolSettings->set_pictures_in_background(!EolSettings->pictures_in_background());
                invalidate_level();
            });

        nav.add_row(
            "Centered Camera:", EolSettings->center_camera() ? "Yes" : "No",
            NAV_FUNC() { EolSettings->set_center_camera(!EolSettings->center_camera()); });

        nav.add_row(
            "Centered Minimap:", EolSettings->center_map() ? "Yes" : "No",
            NAV_FUNC() { EolSettings->set_center_map(!EolSettings->center_map()); });

        nav.add_row(
            "Minimap Alignment:",
            [] {
                switch (EolSettings->map_alignment()) {
                case MapAlignment::None:
                    return "None";
                case MapAlignment::Left:
                    return "Left";
                case MapAlignment::Middle:
                    return "Middle";
                case MapAlignment::Right:
                    return "Right";
                }
                return "";
            }(),
            NAV_FUNC() {
                switch (EolSettings->map_alignment()) {
                case MapAlignment::None:
                    EolSettings->set_map_alignment(MapAlignment::Left);
                    return;
                case MapAlignment::Left:
                    EolSettings->set_map_alignment(MapAlignment::Middle);
                    return;
                case MapAlignment::Middle:
                    EolSettings->set_map_alignment(MapAlignment::Right);
                    return;
                case MapAlignment::Right:
                    EolSettings->set_map_alignment(MapAlignment::None);
                    return;
                }
            });

        nav.add_row(
            "Resolution:",
            std::format("{}x{}", EolSettings->screen_width(), EolSettings->screen_height()),
            NAV_FUNC() {
                switch (EolSettings->screen_width()) {
                case 640:
                    EolSettings->set_screen_width(1024);
                    EolSettings->set_screen_height(768);
                    return;
                case 1024:
                    EolSettings->set_screen_width(640);
                    EolSettings->set_screen_height(480);
                    return;
                }
            });

        nav.add_row(
            "Zoom:", std::format("{:.2f}", EolSettings->zoom()), NAV_FUNC() {
                double old_zoom = EolSettings->zoom();
                EolSettings->set_zoom(old_zoom + 0.25);
                if (old_zoom == EolSettings->zoom()) {
                    EolSettings->set_zoom(0.25);
                }
            });

        nav.add_row(
            "Zoom Textures:", EolSettings->zoom_textures() ? "Yes" : "No",
            NAV_FUNC() { EolSettings->set_zoom_textures(!EolSettings->zoom_textures()); });

        nav.add_row(
            "Renderer:",
            [] {
                switch (EolSettings->renderer()) {
                case RendererType::Software:
                    return "Software";
                case RendererType::OpenGL:
                    return "OpenGL";
                }
                return "";
            }(),
            NAV_FUNC() {
                switch (EolSettings->renderer()) {
                case RendererType::Software:
                    EolSettings->set_renderer(RendererType::OpenGL);
                    return;
                case RendererType::OpenGL:
                    EolSettings->set_renderer(RendererType::Software);
                    return;
                }
            });

        nav.add_row(
            "Turn Time:",
            [] {
                if (EolSettings->turn_time() == 0.0) {
                    return std::string("Instant");
                } else {
                    return std::format("{:.2f}s", EolSettings->turn_time());
                }
            }(),
            NAV_FUNC() {
                double old_turn_time = EolSettings->turn_time();
                double new_turn_time = std::round((old_turn_time - 0.10) * 100.0) / 100.0;
                EolSettings->set_turn_time(new_turn_time);
                if (old_turn_time == EolSettings->turn_time()) {
                    EolSettings->set_turn_time(0.35);
                }
            });

        nav.add_row(
            "LCtrl search:", EolSettings->lctrl_search() ? "Yes" : "No",
            NAV_FUNC() { EolSettings->set_lctrl_search(!EolSettings->lctrl_search()); });

        nav.add_row("Default LGR:", EolSettings->default_lgr_name(), NAV_FUNC() { menu_lgr(); });

        nav.add_row(
            "Show Apple Time:", EolSettings->show_last_apple_time() ? "Yes" : "No", NAV_FUNC() {
                EolSettings->set_show_last_apple_time(!EolSettings->show_last_apple_time());
            });

        nav.add_row(
            "Record Replay FPS:", std::to_string(EolSettings->recording_fps()), NAV_FUNC() {
                int old_fps = EolSettings->recording_fps();
                int new_fps;
                if (old_fps == 30) {
                    new_fps = 60;
                } else if (old_fps == 60) {
                    new_fps = 120;
                } else {
                    new_fps = 30;
                }
                EolSettings->set_recording_fps(new_fps);
            });

        nav.add_row(
            "Demo menu:", EolSettings->show_demo_menu() ? "Yes" : "No",
            NAV_FUNC() { EolSettings->set_show_demo_menu(!EolSettings->show_demo_menu()); });

        nav.add_row(
            "Help menu:", EolSettings->show_help_menu() ? "Yes" : "No",
            NAV_FUNC() { EolSettings->set_show_help_menu(!EolSettings->show_help_menu()); });

        nav.add_row(
            "Best times menu:", EolSettings->show_best_times_menu() ? "Yes" : "No", NAV_FUNC() {
                EolSettings->set_show_best_times_menu(!EolSettings->show_best_times_menu());
            });

        choice = nav.navigate();

        if (choice < 0) {
            eol_settings::write_settings();
            State->save();
            return;
        }
    }
}
