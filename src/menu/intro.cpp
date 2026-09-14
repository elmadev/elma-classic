#include "menu/intro.h"
#include "eol/settings.h"
#include "game/state.h"
#include "main.h"
#include "menu/ball_collision.h"
#include "menu/main.h"
#include "menu/pic.h"
#include "menu/player.h"
#include "menu/rec_list.h"
#include "pic/pic8.h"
#include "pic/surface.h"
#include "platform/implementation.h"
#include "platform/scancode.h"

static void show_intro_screen() {
    pic8* intro_screen = lock_backbuffer_pic(false);
    intro_screen->fill_box(BLACK_PALETTE_ID);
    blit8(intro_screen, Intro, SCREEN_WIDTH / 2 - Intro->get_width() / 2,
          SCREEN_HEIGHT / 2 - Intro->get_height() / 2);
    unlock_backbuffer_pic();
}

void menu_intro() {
    rec_list::build_cache();

    // test_player();

    // Load intro.pcx and hide the version
    if (!EolSettings->skip_intro()) {
        Intro = new pic8("intro.pcx");
        Intro->fill_box(0, 410, Intro->get_width(), 450, Intro->gpixel(0, 409));
        Intro->add_transparency();
    }

    // Display intro.pcx
    MenuPalette->set();
    if (!EolSettings->skip_intro()) {
        show_intro_screen();
    }

    init_sound();

    // Await for key input before scrolling intro.pcx
    if (!EolSettings->skip_intro()) {
        while (true) {
            handle_events();
            if (get_any_key_just_pressed()) {
                break;
            }
            show_intro_screen();
        }
    }

    if (State->player_count == 0) {
        if (!menu_player_create(true)) {
            menu_exit();
        }
    } else {
        if (!EolSettings->skip_intro()) {
            if (!menu_player_choose(true, false)) {
                menu_exit();
            }
        }
    }

    menu_main();

    internal_error("menu_intro!");
}

void menu_exit() {
    WallsDisabled = true;
    menu_pic* menu = new menu_pic;
    menu->add_line_centered("Thank you for registering the game!", 320, 220);
    menu->add_line_centered("Please do not distribute!", 320, 300);

    menu->loop();
    quit();
}
