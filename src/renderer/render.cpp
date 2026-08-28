#include "renderer/render.h"
#include "editor/editor.h"
#include "eol/console.h"
#include "eol/eol.h"
#include "eol/settings.h"
#include "eol/status_messages.h"
#include "game/driver.h"
#include "game/fps.h"
#include "game/game.h"
#include "level/level.h"
#include "main.h"
#include "physics/flagtag.h"
#include "physics/init.h"
#include "physics/pacer.h"
#include "pic/abc8.h"
#include "pic/anim.h"
#include "pic/lgr.h"
#include "pic/pic8.h"
#include "pic/surface.h"
#include "platform/implementation.h"
#include "renderer/affine.h"
#include "renderer/canvas.h"
#include "renderer/object_overlay.h"
#include "renderer/timer.h"
#include "util/util.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <format>
#include <string>
#include <vector>

static bool GameBackgroundRender = false;

static abc8* SmallFont = nullptr;
static abc8* MediumFont = nullptr;
static abc8* LargeFont = nullptr;

// Percentage of the screen used to render the game (QFRAME drawn on the edge)
static double VisibleFraction = 1.0;
constexpr double VISIBLE_FRACTION_SCALING_FACTOR = 1.1;

void reset_game_background() { GameBackgroundRender = true; }
static bool is_opengl_render() {
    return !std::getenv("CPURENDER") && EolSettings->renderer() == RendererType::OpenGL;
}

void increase_view_size() {
    if (is_opengl_render()) {
        GlZoom /= 1.1;
        return;
    }
    VisibleFraction *= VISIBLE_FRACTION_SCALING_FACTOR;
    if (VisibleFraction >= 0.999) {
        VisibleFraction = 1.0;
    }
    reset_game_background();
}

void decrease_view_size() {
    if (is_opengl_render()) {
        GlZoom *= 1.1;
        return;
    }
    VisibleFraction /= VISIBLE_FRACTION_SCALING_FACTOR;
    VisibleFraction = std::max(VisibleFraction, 0.7);
    reset_game_background();
}

// Position of the view of player1 and player2 in pixels on the screen
static int GameViewLeft;
static int GameViewBottom1;
static int GameViewRight;
static int GameViewTop1;
static int GameViewBottom2;
static int GameViewTop2;
int GameViewWidth;
int GameViewHeight;

// In meters from the bottom-left corner of screen
static double CameraY;
static double CameraX;
static double CameraDx;

// In pixels from the bottom-left corner of screen
double AffinePicScreenLeft;
double AffinePicScreenRight;
double AffinePicScreenBottom;
double AffinePicScreenTop;

// In pixels from the bottom-left corner of screen
int MinimapWidth;
int MinimapHeight;
int MinimapX;
int MinimapDx;

pic8* shirt = nullptr;

void init_renderer() {
    shirt = eol::load_shirt(EolSettings->nick());
    init_gravity_arrows();

    SmallFont = new abc8("small.abc", 1, 12);
    MediumFont = new abc8("medium.abc", 1, 18);
    LargeFont = new abc8("large.abc", 2, 90);

    Console = new console();
    Console->register_console_commands();
    Console->set_font(SmallFont);

    StatusMessages = new status_messages();
}

// Determine the position of the view of player1 +- player2 on the screen
static void calculate_viewpoints(bool splitscreen) {
    // Determine viewpoint of player1 and player2
    GameViewWidth = (int)(SCREEN_WIDTH * VisibleFraction);
    GameViewHeight = (int)(SCREEN_HEIGHT * VisibleFraction);
    GameViewLeft = (SCREEN_WIDTH - GameViewWidth) / 2;
    GameViewBottom1 = (SCREEN_HEIGHT - GameViewHeight) / 2;
    GameViewRight = GameViewLeft + GameViewWidth - 1;
    GameViewTop1 = GameViewBottom1 + GameViewHeight - 1;
    if (splitscreen) {
        GameViewHeight = (SCREEN_HEIGHT / 2) - 6;
        GameViewBottom1 = (SCREEN_HEIGHT / 2) + 6;
        GameViewTop1 = SCREEN_HEIGHT - 1;
        GameViewBottom2 = 0;
        GameViewTop2 = (SCREEN_HEIGHT / 2) - 7;
    }
#ifdef DEBUG
    if (GameViewRight >= SCREEN_WIDTH) {
        internal_error("calculate_viewpoints GameViewRight >= SCREEN_WIDTH!");
    }
    if (GameViewTop1 >= SCREEN_HEIGHT) {
        internal_error("calculate_viewpoints GameViewTop1 >= SCREEN_HEIGHT!");
    }
#endif

    // Determine boundary for the purposes of drawing affine pictures
    AffinePicScreenLeft = 2.0;
    AffinePicScreenBottom = 2.0;
    AffinePicScreenRight = GameViewWidth - 3.0;
    AffinePicScreenTop = GameViewHeight - 3.0;

    // Determine camera position in meters from the bottom-left corner
    if (EolSettings->center_camera()) {
        CameraX = (GameViewWidth / MetersToPixels) * 0.50;
    } else {
        CameraX = (SCREEN_WIDTH / MetersToPixels) * 0.15 * EolSettings->zoom();
    }
    CameraDx = GameViewWidth / MetersToPixels - 2.0 * CameraX;
    CameraY = GameViewHeight / MetersToPixels / 2.0;

    // Determine minimap position and size
    MinimapWidth =
        (int)(EolSettings->minimap_width() * sqrt(double(GameViewHeight) / double(SCREEN_HEIGHT)));
    MinimapHeight =
        (int)(EolSettings->minimap_height() * sqrt(double(GameViewHeight) / double(SCREEN_HEIGHT)));
    MinimapX = (int)(40.0 * (VisibleFraction - 0.6) / 0.4);
    MinimapDx = GameViewWidth - 2 * MinimapX - MinimapWidth;
}

static void handle_screenshot(pic8* pic) {
    if (VideoRecordingMode) {
        std::string filename = std::format("snp{:05}.pcx", VideoFrameIndex);
        std::filesystem::path path = std::filesystem::path(VideoOutputDirectory) / filename;
        pic->vertical_flip();
        pic->save(path.string().c_str(), Lgr->palette_data);
        pic->vertical_flip();
        return;
    }

    if (ScreenshotRequested) {
        ScreenshotRequested = false;
        platform_save_screenshot();
    }
}

// Local (2-player) flag tag: does this player's bike currently show the flag?
static bool local_flag_tag_has_flag(bool player1, double time) {
    if (Single || !FlagTag) {
        return false;
    }
    if ((player1 && FlagTagAHasFlag) || (!player1 && !FlagTagAHasFlag)) {
        return true;
    }
    // Other player has flag, but this player just lost the flag
    // Blink the flag away while immunity applies
    return FlagTagImmunity && (int)(time * 30.0) % 2 != 0;
}

static bool bike_in_view(const motorst* mot, vect2 center) {
    double distance = (mot->bike.r - center).length();
    return distance < (std::max(SCREEN_WIDTH, SCREEN_HEIGHT) * 27.0 / 32.0) * PixelsToMeters;
}

// Render the bottom-right info panel: rows[0] is the bottom row, each later row stacks above it
// (the backbuffer is upside-down, so larger y is higher on screen).
void render_info_panel(pic8* pic, const std::vector<info_panel_row>& rows) {
    constexpr int RIGHT_MARGIN = 10;
    constexpr int BOTTOM_MARGIN = 10;
    constexpr int LABEL_OFFSET = 180;
    constexpr int EXTRA_SPACE_PER_CHAR = 6;

    int max_value_length = 0;
    for (const info_panel_row& row : rows) {
        max_value_length = std::max(max_value_length, (int)row.value.size());
    }
    int label_offset = LABEL_OFFSET;
    if (max_value_length > 10) {
        label_offset += (max_value_length - 10) * EXTRA_SPACE_PER_CHAR;
    }

    int value_x = GameViewWidth - RIGHT_MARGIN;
    int label_x = GameViewWidth - label_offset;
    for (size_t i = 0; i < rows.size(); i++) {
        int y = BOTTOM_MARGIN + (int)i * SmallFont->line_height();
        SmallFont->write(pic, label_x, y, rows[i].label.c_str());
        SmallFont->write_right_align(pic, value_x, y, rows[i].value.c_str());
    }
}

static std::vector<info_panel_row> get_info_rows(bool bottom_player, GameLoop loop,
                                                 camera current_camera, driver& driv) {

    // Build the bottom-right info panel rows.
    // rows are rendered in the order they were added (last added on top)
    std::vector<info_panel_row> info_rows;

    if (loop == GameLoop::Game) {
        if (current_camera.mode != CameraMode::MapViewer) {
            if (EolSettings->show_speedometer()) {
                info_rows.push_back({"max speed", driv.stats.format_max_speed()});
                info_rows.push_back({"speed", driv.stats.format_speed()});
            }

            if (EolSettings->show_one_wheel_status()) {
                info_rows.push_back({"one wheel", driv.mot->one_wheel_failed ? "no" : "yes"});
            }
        }

        if (bottom_player) {
            // FPS
            if (EolSettings->show_fps()) {
                info_rows.push_back({"FPS", fps::format_fps() + pacer::format_fps_limit()});
            }

            // UPS
            if (EolSettings->show_ups() && current_camera.mode == CameraMode::Normal) {
                info_rows.push_back({"UPS", fps::format_ups()});
            }
        }
    }

    // Apple count/time
    if (driv.mot->apple_count && EolSettings->show_last_apple_time()) {
        char apple_time[32];
        util::text::centiseconds_to_string(driv.mot->last_apple_time, apple_time, true, true);
        info_rows.push_back(
            {std::format("last apple ({})", driv.mot->apple_count - driv.mot->apple_bug_count),
             apple_time});
    }

    return info_rows;
}

// Render the view for one player
void GameRenderer::render_view(bool player1, bool bottom_player, int left, int bottom, int right,
                               int top) {

    auto driv = player1 ? driv1 : driv2;
    auto other_driv = player1 ? driv2 : driv1;

    // Give advance notice of the timers since OpenGL is a diva
    // (should come before subview)
    if (driv.hud->timer) {
        double flagtag_time = -1.0;
        if (!Single && FlagTag) {
            flagtag_time = player1 ? FlagTimeA : FlagTimeB;
        }
    }

    // Calculate frame of reference
    vect2 bike_center = driv.mot->bike.r;
    if (current_camera.mode == CameraMode::MapViewer) {
        bike_center = vect2(current_camera.x, current_camera.y);
    }

    const kuski* spy_kuski = EolClient->spy_kuski();
    if (spy_kuski) {
        bike_center = spy_kuski->spy_data()->mot.bike.r;
    }

    bottomleft_corner.x =
        bike_center.x - (CameraX + driv.meta.camera_turning.turn_phase * CameraDx);
    bottomleft_corner.y = bike_center.y - CameraY;

    center.x = bottomleft_corner.x + (SCREEN_WIDTH / 2.0) * PixelsToMeters;
    center.y = bottomleft_corner.y + (SCREEN_HEIGHT / 2.0) * PixelsToMeters;

    // Set part of screen to draw on
    subview(left, bottom, right, top);

    // Draw the background
    render_back(player1);

    // Draw the objects
    render_objects();

    // Select the correct bike for each player
    bike_pics* bike1 = &Lgr->bike1;
    bike_pics* bike2 = &Lgr->bike2;
    if ((State->player1_bike1 && !player1) || (!State->player1_bike1 && player1)) {
        bike1 = &Lgr->bike2;
        bike2 = &Lgr->bike1;
    }

    if (EolSettings->show_others()) {
        for (const kuski& ku : EolClient->kuskis()) {
            if (&ku == spy_kuski) {
                continue;
            }
            const spy_data* k = ku.spy_data();
            if (!k) {
                continue;
            }

            if (bike_in_view(&k->mot, center)) {
                render_bike(EolClient->kuski_has_flag(ku.id), &k->mot, &k->metadata, bike2,
                            ku.shirt);
            }
        }
    }

    if (spy_kuski) {
        const spy_data* k = spy_kuski->spy_data();
        if (k && bike_in_view(&k->mot, center)) {
            render_bike(EolClient->kuski_has_flag(spy_kuski->id), &k->mot, &k->metadata, bike2,
                        spy_kuski->shirt);
        }
    }

    if (current_camera.mode == CameraMode::Normal) {
        if (!Single) {
            // Draw the other bike if it's on-screen
            if (bike_in_view(other_driv.mot, center)) {
                render_bike(local_flag_tag_has_flag(!player1, time), other_driv.mot,
                            &other_driv.meta, bike2, nullptr);
            }
        }

        // Draw the current player's bike
        render_bike(local_flag_tag_has_flag(player1, time) || EolClient->own_bike_has_flag(),
                    driv.mot, &driv.meta, bike1, shirt);
    }

    // Draw the foreground
    if (!EolSettings->pictures_in_background()) {
        render_front(player1);
    }

    // Draw the minimap
    if (driv.hud->minimap) {
        auto other = Single ? nullptr : other_driv.mot;
        dispatch_minimap(player1, driv.meta.camera_turning.turn_phase, bike_center, other);
    }

    // Draw the timers
    if (driv.hud->timer) {
        double flagtag_time = -1.0;
        if (!Single && FlagTag) {
            flagtag_time = player1 ? FlagTimeA : FlagTimeB;
        }
        render_timers(BestTime, flagtag_time, GameViewWidth, GameViewHeight);
    }

    /*
     * Pic rendering stuff
     */

    auto info_rows = get_info_rows(bottom_player, loop, current_camera, driv);

    auto pic = get_backbuffer_pic();
    render_info_panel(pic, info_rows);

    if (!EolClient->play_offline() && !EolClient->connected()) {
        MediumFont->write_right_align(
            pic, GameViewWidth - 10, GameViewHeight - MediumFont->line_height() * 2,
            std::format("Lost connection ({} to reconnect)", dik_to_string(State->key_reconnect))
                .c_str());
    }
}

GameRenderer::GameRenderer(double time, driver& driv1, driver& driv2, camera& current_camera,
                           GameLoop loop)
    : time(time),
      driv1(driv1),
      driv2(driv2),
      current_camera(current_camera),
      loop(loop) {
    // Determine who we are going to draw (player 1, player 2 or both)
    draw_player1 = driv1.draw_view;
    draw_player2 = driv2.draw_view;
    if (Single || current_camera.mode == CameraMode::MapViewer) {
        draw_player1 = true;
        draw_player2 = false;
    }
    if (!draw_player1 && !draw_player2) {
        internal_error("render_game nobody visible!");
    }
    splitscreen = draw_player1 && draw_player2;
}

void GameRenderer::render() {

    start_frame();

    // If we need to recalculate the screen position, redraw the background qframe
    if (GameBackgroundRender) {
        GameBackgroundRender = false;
        calculate_viewpoints(splitscreen);
    }

    render_qframe(GameBackgroundRender);

    // Draw 1 or 2 players
    if (splitscreen) {
        render_view(true, false, GameViewLeft, GameViewBottom1, GameViewRight, GameViewTop1);
        render_view(false, true, GameViewLeft, GameViewBottom2, GameViewRight, GameViewTop2);
    } else {
        render_view(draw_player1, true, GameViewLeft, GameViewBottom1, GameViewRight, GameViewTop1);
    }

    // Draw EOL overlays
    auto pic = get_backbuffer_pic();
    Console->render(*pic);
    StatusMessages->render(*pic, *SmallFont);
    EolClient->render_table(*pic, *MediumFont, *SmallFont);
    EolClient->render_battle_status(*pic, *SmallFont);
    EolClient->render_battle_leader(*pic, *SmallFont);
    EolClient->render_battle_countdown(*pic, *LargeFont, *SmallFont);

    end_frame();

    // Conditionally save screenshot
    handle_screenshot(pic);
}

// Render the entire minimap
void GameRenderer::dispatch_minimap(bool player1, double camera_turn_phase, vect2 bike_center,
                                    motorst* other_motor) {
    // Calculate minimap size and minimap frame of reference
    double minimap_width = MinimapWidth * MinimapScaleFactor * PixelsToMeters;
    double minimap_height = MinimapHeight * MinimapScaleFactor * PixelsToMeters;

    double camera_x = EolSettings->center_map() ? 0.5 : 0.2;
    double camera_dx = 1.0 - 2.0 * camera_x;
    vect2 camera_pos(minimap_width * (camera_x + camera_turn_phase * camera_dx),
                     minimap_height / 2);
    vect2 bottomleft_corner = bike_center - camera_pos;

    double align;
    switch (EolSettings->map_alignment()) {
    case MapAlignment::None:
        align = camera_turn_phase;
        break;
    case MapAlignment::Left:
        align = 0.0;
        break;
    case MapAlignment::Middle:
        align = 0.5;
        break;
    case MapAlignment::Right:
        align = 1.0;
        break;
    }

    const int minimap_x1 = std::max(1, (int)(MinimapX + align * MinimapDx));
    const int minimap_x2 = minimap_x1 + MinimapWidth - 1;
    const int minimap_y1 = 1;
    const int minimap_y2 = minimap_y1 + MinimapHeight - 1;

    render_minimap(player1, other_motor, minimap_x1, minimap_y1, minimap_x2, minimap_y2,
                   bottomleft_corner, camera_pos);
}

void render_game(double time, driver& driv1, driver& driv2, camera& current_camera, GameLoop loop) {
    reload_graphic_assets();

    fps::update();

    auto renderer = createPicRenderer(time, driv1, driv2, current_camera, loop);

    renderer->render();
}
