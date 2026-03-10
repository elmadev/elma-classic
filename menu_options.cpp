#include "menu_options.h"
#include "eol_settings.h"
#include "fs_utils.h"
#include "JATEKOS.H"
#include "level_load.h"
#include "M_PIC.H"
#include "menu_controls.h"
#include "menu_dialog.h"
#include "menu_nav.h"
#include "menu_pic.h"
#include "platform_impl.h"
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

struct resolution {
    int width;
    int height;
};

static constexpr resolution RESOLUTIONS[] = {
    {800, 600},  {1024, 768},  {1280, 720},  {1280, 960},  {1280, 1024}, {1366, 768},  {1440, 900},
    {1600, 900}, {1600, 1200}, {1920, 1080}, {1920, 1200}, {2560, 1440}, {3840, 1600}, {3840, 2160},
};

static void menu_resolution() {
    menu_nav nav("Pick a resolution!");

    if (EolSettings->fullscreen() == FullscreenMode::Fullscreen) {
        auto display_modes = platform_get_display_modes();
        for (const auto& [w, h] : display_modes) {
            std::string label = std::format("{}x{}", w, h);
            nav.add_row(label, NAV_FUNC(w, h) { update_resolution(w, h); });
        }
    } else {
        auto [desktop_w, desktop_h] = platform_get_desktop_resolution();
        for (const auto& res : RESOLUTIONS) {
            if (res.width > desktop_w || res.height > desktop_h) {
                continue;
            }
            std::string label = std::format("{}x{}", res.width, res.height);
            nav.add_row(label, NAV_FUNC(&res) { update_resolution(res.width, res.height); });
        }
    }

    std::string current =
        std::format("{}x{}", EolSettings->screen_width(), EolSettings->screen_height());
    nav.select_row(current);

    nav.navigate();
}

static const char* fullscreen_mode_label(FullscreenMode mode) {
    switch (mode) {
    case FullscreenMode::Windowed:
        return "Off";
    case FullscreenMode::Fullscreen:
        return "Exclusive";
    case FullscreenMode::FullscreenDesktop:
        return "Fullscreen (Desktop)";
    }
    return "";
}

static void menu_fullscreen() {
    menu_nav nav("Pick a fullscreen mode!");

    constexpr FullscreenMode MODES[] = {
        FullscreenMode::Windowed,
        FullscreenMode::Fullscreen,
        FullscreenMode::FullscreenDesktop,
    };

    for (auto mode : MODES) {
        nav.add_row(
            fullscreen_mode_label(mode), NAV_FUNC(mode) { EolSettings->set_fullscreen(mode); });
    }

    nav.select_row(fullscreen_mode_label(EolSettings->fullscreen()));

    nav.navigate();
}

#define BOOL_OPTION(text, setting)                                                                 \
    nav.add_row(                                                                                   \
        text, EolSettings->setting() ? "Yes" : "No",                                               \
        NAV_FUNC() { EolSettings->set_##setting(!EolSettings->setting()); });

void menu_options() {
    menu_nav nav("Options");
    nav.x_left = 0;
    nav.x_right = 390;
    nav.y_entries = 77;
    nav.dy = 36;
    nav.search_pattern = SearchPattern::Filter;

    int choice = 0;
    while (true) {
        nav.clear_entries();
        nav.select_row(choice);

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

        BOOL_OPTION("Still Objects:", still_objects);

        BOOL_OPTION("Access all internals:", all_internals_accessible);

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

        BOOL_OPTION("Centered Camera:", center_camera);
        BOOL_OPTION("Centered Minimap:", center_map);

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
            "Minimap Size:",
            std::format("{}x{}", EolSettings->minimap_width(), EolSettings->minimap_height()),
            NAV_FUNC() {
                int w = EolSettings->minimap_width();
                if (w == 140) {
                    EolSettings->set_minimap_width(180);
                    EolSettings->set_minimap_height(90);
                } else if (w == 180) {
                    EolSettings->set_minimap_width(220);
                    EolSettings->set_minimap_height(110);
                } else if (w == 220) {
                    EolSettings->set_minimap_width(280);
                    EolSettings->set_minimap_height(140);
                } else if (w == 280) {
                    EolSettings->set_minimap_width(350);
                    EolSettings->set_minimap_height(175);
                } else if (w == 350) {
                    EolSettings->set_minimap_width(420);
                    EolSettings->set_minimap_height(210);
                } else {
                    EolSettings->set_minimap_width(140);
                    EolSettings->set_minimap_height(70);
                }
            });

        nav.add_row(
            "Minimap Opacity:", std::format("{}%", EolSettings->minimap_opacity()), NAV_FUNC() {
                int opacity = EolSettings->minimap_opacity();
                if (opacity == 25) {
                    EolSettings->set_minimap_opacity(50);
                } else if (opacity == 50) {
                    EolSettings->set_minimap_opacity(75);
                } else if (opacity == 75) {
                    EolSettings->set_minimap_opacity(100);
                } else {
                    EolSettings->set_minimap_opacity(25);
                }
            });

        nav.add_row(
            "Resolution:",
            std::format("{}x{}", EolSettings->screen_width(), EolSettings->screen_height()),
            NAV_FUNC() {
                if (EolSettings->fullscreen() == FullscreenMode::FullscreenDesktop) {
                    menu_dialog("Resolution is locked to desktop", "in Fullscreen (Desktop) mode.");
                    return;
                }
                menu_resolution();
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
            "Minimap Zoom:", std::format("{:.2f}", EolSettings->minimap_zoom()), NAV_FUNC() {
                double old_zoom = EolSettings->minimap_zoom();
                EolSettings->set_minimap_zoom(old_zoom + 0.25);
                if (old_zoom == EolSettings->minimap_zoom()) {
                    EolSettings->set_minimap_zoom(0.25);
                }
            });

        BOOL_OPTION("Zoom Textures:", zoom_textures);

        BOOL_OPTION("Zoom Grass:", zoom_grass);

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
            "Fullscreen:", fullscreen_mode_label(EolSettings->fullscreen()),
            NAV_FUNC() { menu_fullscreen(); });

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

        BOOL_OPTION("LCtrl search:", lctrl_search);

        nav.add_row("Default LGR:", EolSettings->default_lgr_name(), NAV_FUNC() { menu_lgr(); });

        BOOL_OPTION("Show Apple Time:", show_last_apple_time);

        BOOL_OPTION("Gravity Arrows:", show_gravity_arrows);

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

        BOOL_OPTION("Show Total Time:", show_total_time);
        BOOL_OPTION("Demo menu:", show_demo_menu);
        BOOL_OPTION("Help menu:", show_help_menu);
        BOOL_OPTION("Best Times menu:", show_best_times_menu);

        nav.add_row(
            "Num Chat Lines:", std::format("{}", EolSettings->chat_lines()), NAV_FUNC() {
                int old_chat_lines = EolSettings->chat_lines();
                EolSettings->set_chat_lines(old_chat_lines + 1);
                if (old_chat_lines == EolSettings->chat_lines()) {
                    EolSettings->set_chat_lines(1);
                }
            });

        choice = nav.navigate();

        if (choice < 0) {
            eol_settings::write_settings();
            State->save();
            return;
        }
    }
}

#undef BOOL_OPTION
