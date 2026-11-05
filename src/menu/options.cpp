#include "menu/options.h"
#include "eol/settings.h"
#include "game/state.h"
#include "main.h"
#include "menu/ball.h"
#include "menu/controls.h"
#include "menu/dialog.h"
#include "menu/nav.h"
#include "menu/pic.h"
#include "menu/player.h"
#include "pic/surface.h"
#include "platform/implementation.h"
#include "renderer/canvas.h"
#include "util/file_iter.h"
#include <cmath>
#include <cstring>
#include <format>

void menu_about() {
    BallSpeed = 10.0;

    menu_nav nav("Elasto Mania Online " ELMA_VERSION);

    int i = 1;
    constexpr int DY = 60;
    nav.add_overlay("https://elma.online", 320, nav.y_title + DY * i++, OverlayAlignment::Centered,
                    ScreenAnchor::Center);
    nav.add_overlay("Find Our Community on Discord!", 320, nav.y_title + DY * i++,
                    OverlayAlignment::Centered, ScreenAnchor::Center);
    nav.add_overlay("Thank You to Our Contributors!", 320, nav.y_title + DY * i++,
                    OverlayAlignment::Centered, ScreenAnchor::Center);
    nav.y_entries = nav.y_title + DY * i++;

    // Alphabetical
    nav.add_row("bene", "", NAV_FUNC(){});
    nav.add_row("joey", "", NAV_FUNC(){});
    nav.add_row("Kopaka", "", NAV_FUNC(){});
    nav.add_row("Markku", "", NAV_FUNC(){});
    nav.add_row("sunl", "", NAV_FUNC(){});

    // Alphabetical
    nav.add_row("Hibernatus", "", NAV_FUNC(){});
    nav.add_row("milagros", "", NAV_FUNC(){});

    // Alphabetical
    nav.add_row("amarshalov", "", NAV_FUNC(){});
    nav.add_row("Smibu", "", NAV_FUNC(){});
    nav.add_row("Zweq", "", NAV_FUNC(){});

    nav.navigate();

    BallSpeed = 1.0;
}

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
    bool done = find_first("lgr/*.lgr", filename, MAX_FILENAME_LEN);
    while (!done) {
        constexpr int LGR_EXT_LEN = 4;
        int len = strlen(filename);
        std::string lgrname = std::string(filename, len - LGR_EXT_LEN);

        nav.add_row(lgrname, NAV_FUNC() { EolSettings->persist_default_lgr_name(left); });

        done = find_next(filename);
    }
    find_close();

    nav.sort_rows();
    nav.select_row(EolSettings->default_lgr_name_persisted());

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

    if (EolSettings->fullscreen_persisted() == FullscreenMode::Fullscreen) {
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

    std::string current = std::format("{}x{}", EolSettings->screen_width_persisted(),
                                      EolSettings->screen_height_persisted());
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
            fullscreen_mode_label(mode), NAV_FUNC(mode) { EolSettings->persist_fullscreen(mode); });
    }

    nav.select_row(fullscreen_mode_label(EolSettings->fullscreen_persisted()));

    nav.navigate();
}

#define BOOL_OPTION(text, setting)                                                                 \
    nav.add_row(                                                                                   \
        text, EolSettings->setting##_persisted() ? "Yes" : "No",                                   \
        NAV_FUNC() { EolSettings->persist_##setting(!EolSettings->setting##_persisted()); });

#define INT_OPTION(text, setting, prompt, max_len)                                                 \
    nav.add_row(                                                                                   \
        text, std::to_string(EolSettings->setting##_persisted()), NAV_FUNC() {                     \
            auto val = menu_prompt_int(prompt, std::to_string(EolSettings->setting##_persisted()), \
                                       max_len);                                                   \
            if (!val) {                                                                            \
                return;                                                                            \
            }                                                                                      \
            EolSettings->persist_##setting(*val);                                                  \
        });

#define DOUBLE_OPTION(text, setting, prompt, form, max_len)                                        \
    nav.add_row(                                                                                   \
        text, std::format(form, EolSettings->setting##_persisted()), NAV_FUNC() {                  \
            auto val = menu_prompt_double(                                                         \
                prompt, std::format(form, EolSettings->setting##_persisted()), max_len);           \
            if (!val) {                                                                            \
                return;                                                                            \
            }                                                                                      \
            EolSettings->persist_##setting(*val);                                                  \
        });

template <typename Enum, std::size_t N>
const char* enum_name(Enum value, const std::array<std::pair<Enum, const char*>, N>& options) {
    for (const auto& [option, name] : options) {
        if (option == value) {
            return name;
        }
    }
    return "?";
}

template <typename Enum, std::size_t N, typename Persist>
void cycle_enum(Enum current, const std::array<std::pair<Enum, const char*>, N>& options,
                const Persist& persist) {
    for (std::size_t i = 0; i < N; ++i) {
        if (options[i].first == current) {
            persist(options[(i + 1) % N].first);
            return;
        }
    }
    persist(options[0].first);
}

#define ENUM_OPTION(text, setting, options)                                                        \
    nav.add_row(                                                                                   \
        text, enum_name(EolSettings->setting##_persisted(), options), NAV_FUNC() {                 \
            cycle_enum(EolSettings->setting##_persisted(), options,                                \
                       [](auto val) { EolSettings->persist_##setting(val); });                     \
        });

static void menu_cripples() {
    int choice = 0;
    while (true) {
        menu_nav nav("Cripples");
        nav.select_row(choice);
        nav.x_left = 0;
        nav.x_right = 390;

        BOOL_OPTION("No Brake:", cripple_no_brake);
        BOOL_OPTION("No Throttle:", cripple_no_throttle);
        BOOL_OPTION("Always Throttle:", cripple_always_throttle);
        BOOL_OPTION("No Turn:", cripple_no_turn);
        BOOL_OPTION("One Turn:", cripple_one_turn);
        BOOL_OPTION("No Volt:", cripple_no_volt);
        BOOL_OPTION("Drunk:", cripple_drunk);
        BOOL_OPTION("One Wheel:", cripple_one_wheel);
        BOOL_OPTION("Show One Wheel Status:", show_one_wheel_status);

        choice = nav.navigate();

        if (choice < 0) {
            return;
        }
    }
}

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

        nav.add_row("Player A:", State->player1, NAV_FUNC() { menu_player_choose(true, true); });

        nav.add_row("Player B:", State->player2, NAV_FUNC() { menu_player_choose(false, true); });

        nav.add_row(
            "Sound:", State->sound_on ? "Enabled" : "Disabled", NAV_FUNC() {
                if (State->sound_on) {
                    close_sound_device();
                    State->sound_on = 0;
                } else {
                    open_sound_device();
                    State->sound_on = 1;
                }
            });

        nav.add_row(
            "Animated Menus:", State->animated_menus ? "Yes" : "No",
            NAV_FUNC() { State->animated_menus = !State->animated_menus; });

        nav.add_row(
            "Video Detail:", State->high_quality ? "High" : "Low", NAV_FUNC() {
                State->high_quality = !State->high_quality;
                canvas::invalidate_canvases();
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

        nav.add_row("Cripples ...", NAV_FUNC() { menu_cripples(); });

        BOOL_OPTION("Pics In Background:", pictures_in_background);

        BOOL_OPTION("Override Ground:", default_ground);
        BOOL_OPTION("Override Sky:", default_sky);

        BOOL_OPTION("Centered Camera:", center_camera);
        BOOL_OPTION("Centered Minimap:", center_map);

// macro to keep the player A and player B default show_X settings in sync
#define SHOW_X_OPTION(name, setting)                                                               \
    nav.add_row(                                                                                   \
        name, EolSettings->setting##_player_a_persisted() ? "Yes" : "No", NAV_FUNC() {             \
            EolSettings->persist_##setting##_player_a(                                             \
                !EolSettings->setting##_player_a_persisted());                                     \
            EolSettings->persist_##setting##_player_b(                                             \
                EolSettings->setting##_player_a_persisted());                                      \
        });

        SHOW_X_OPTION("Show Timer", show_timer);
        SHOW_X_OPTION("Show Minimap", show_minimap);
        SHOW_X_OPTION("Show Replay Timer", show_replay_timer);
        SHOW_X_OPTION("Show Replay Minimap", show_replay_minimap);

#undef SHOW_X_OPTION

        static constexpr std::array MAP_ALIGNMENT{
            std::pair{MapAlignment::None, "None"},
            std::pair{MapAlignment::Left, "Left"},
            std::pair{MapAlignment::Middle, "Middle"},
            std::pair{MapAlignment::Right, "Right"},
        };
        ENUM_OPTION("Minimap Alignment:", map_alignment, MAP_ALIGNMENT);

        nav.add_row(
            "Minimap Size:",
            std::format("{}x{}", EolSettings->minimap_width_persisted(),
                        EolSettings->minimap_height_persisted()),
            NAV_FUNC() {
                int w = EolSettings->minimap_width_persisted();
                if (w == 140) {
                    EolSettings->persist_minimap_width(180);
                    EolSettings->persist_minimap_height(90);
                } else if (w == 180) {
                    EolSettings->persist_minimap_width(220);
                    EolSettings->persist_minimap_height(110);
                } else if (w == 220) {
                    EolSettings->persist_minimap_width(280);
                    EolSettings->persist_minimap_height(140);
                } else if (w == 280) {
                    EolSettings->persist_minimap_width(350);
                    EolSettings->persist_minimap_height(175);
                } else if (w == 350) {
                    EolSettings->persist_minimap_width(420);
                    EolSettings->persist_minimap_height(210);
                } else {
                    EolSettings->persist_minimap_width(140);
                    EolSettings->persist_minimap_height(70);
                }
            });

        nav.add_row(
            "Minimap Opacity:", std::format("{}%", EolSettings->minimap_opacity_persisted()),
            NAV_FUNC() {
                int opacity = EolSettings->minimap_opacity_persisted();
                if (opacity == 25) {
                    EolSettings->persist_minimap_opacity(50);
                } else if (opacity == 50) {
                    EolSettings->persist_minimap_opacity(75);
                } else if (opacity == 75) {
                    EolSettings->persist_minimap_opacity(100);
                } else {
                    EolSettings->persist_minimap_opacity(25);
                }
            });

        nav.add_row(
            "Resolution:",
            std::format("{}x{}", EolSettings->screen_width_persisted(),
                        EolSettings->screen_height_persisted()),
            NAV_FUNC() {
                if (EolSettings->fullscreen_persisted() == FullscreenMode::FullscreenDesktop) {
                    menu_dialog("Resolution is locked to desktop", "in Fullscreen (Desktop) mode.");
                    return;
                }
                menu_resolution();
            });

        DOUBLE_OPTION("Zoom:", zoom, "Enter Zoom", "{:.2f}", 5);

        DOUBLE_OPTION("Minimap Zoom:", minimap_zoom, "Enter Minimap Zoom", "{:.2f}", 5);

        BOOL_OPTION("Zoom Textures:", zoom_textures);

        BOOL_OPTION("Zoom Grass:", zoom_grass);

        static constexpr std::array RENDERER{
            std::pair{RendererType::Software, "Software"},
            std::pair{RendererType::OpenGL, "OpenGL"},
        };
        ENUM_OPTION("Renderer:", renderer, RENDERER);

        nav.add_row(
            "Fullscreen:", fullscreen_mode_label(EolSettings->fullscreen_persisted()),
            NAV_FUNC() { menu_fullscreen(); });

        nav.add_row(
            "Turn Time:",
            [] {
                if (EolSettings->turn_time_persisted() == 0.0) {
                    return std::string("Instant");
                }
                return std::format("{:.2f}s", EolSettings->turn_time_persisted());
            }(),
            NAV_FUNC() {
                double old_turn_time = EolSettings->turn_time_persisted();
                double new_turn_time = std::round((old_turn_time - 0.10) * 100.0) / 100.0;
                EolSettings->persist_turn_time(new_turn_time);
                if (old_turn_time == EolSettings->turn_time_persisted()) {
                    EolSettings->persist_turn_time(0.35);
                }
            });

        BOOL_OPTION("LCtrl search:", lctrl_search);

        nav.add_row(
            "Default LGR:", EolSettings->default_lgr_name_persisted(), NAV_FUNC() { menu_lgr(); });

        BOOL_OPTION("Fancyboost LGRs:", fancyboost);

        BOOL_OPTION("Show Apple Time:", show_last_apple_time);
        BOOL_OPTION("Show Lev Name In Rec:", show_lev_name_in_rec);
        BOOL_OPTION("Gravity Arrows:", show_gravity_arrows);

        BOOL_OPTION("Show FPS:", show_fps);
        BOOL_OPTION("Show UPS:", show_ups);

        nav.add_row(
            "FPS Limit:",
            EolSettings->fps_limit_enabled_persisted()
                ? std::to_string(EolSettings->fps_limit_persisted())
                : std::string("Off"),
            NAV_FUNC() {
                static constexpr int steps[] = {60, 100, 144, 240, 500};
                if (!EolSettings->fps_limit_enabled_persisted()) {
                    EolSettings->persist_fps_limit_enabled(true);
                    EolSettings->persist_fps_limit(steps[0]);
                    return;
                }
                int cur = EolSettings->fps_limit_persisted();
                for (size_t i = 0; i < std::size(steps); ++i) {
                    if (steps[i] == cur && i + 1 < std::size(steps)) {
                        EolSettings->persist_fps_limit(steps[i + 1]);
                        return;
                    }
                }
                EolSettings->persist_fps_limit_enabled(false);
                EolSettings->persist_fps_limit(steps[0]);
            });

        INT_OPTION("Record Replay FPS", recording_fps, "Enter Record Replay FPS", 4);

        BOOL_OPTION("Show Total Time:", show_total_time);
        BOOL_OPTION("Demo menu:", show_demo_menu);
        BOOL_OPTION("Help menu:", show_help_menu);
        BOOL_OPTION("About menu:", show_about_menu);
        BOOL_OPTION("Best Times menu:", show_best_times_menu);
        BOOL_OPTION("Skip Intro:", skip_intro);

        static constexpr std::array SHOW_CHAT{
            std::pair{ChatVisibility::Shown, "Shown"},
            std::pair{ChatVisibility::PublicHidden, "Public hidden"},
            std::pair{ChatVisibility::Hidden, "No"},
        };
        ENUM_OPTION("Show chat:", chat_visibility, SHOW_CHAT);

        BOOL_OPTION("Large Font:", large_font);

        static constexpr std::array SHOWN_TO{
            std::pair{ShownTo::Everyone, "Everyone"},
            std::pair{ShownTo::Team, "Team"},
            std::pair{ShownTo::Nobody, "Nobody"},
        };
        ENUM_OPTION("Shown to:", shown_to, SHOWN_TO);

        INT_OPTION("Num Chat Lines:", chat_lines, "Enter Number of Chat Lines", 2);

        BOOL_OPTION("Show others:", show_others);
        BOOL_OPTION("Show battle status:", show_battle_status);
        BOOL_OPTION("Show battle leader:", show_battle_leader);
        BOOL_OPTION("Show speedometer:", show_speedometer);

        static constexpr std::array TABLE_ALIGNMENT{
            std::pair{eol_table::Align::Left, "Left"},
            std::pair{eol_table::Align::Center, "Center"},
            std::pair{eol_table::Align::Right, "Right"},
        };
        ENUM_OPTION("Table Alignment:", table_alignment, TABLE_ALIGNMENT);

        choice = nav.navigate();

        if (choice < 0) {
            eol_settings::write_settings();
            State->save();
            return;
        }
    }
}

#undef BOOL_OPTION
