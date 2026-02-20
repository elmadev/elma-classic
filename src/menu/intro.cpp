#include "menu/intro.h"
#include "abc8.h"
#include "menu/ball_collision.h"
#include "EDITUJ.H"
#include "eol_settings.h"
#include "KIRAJZOL.H"
#include "M_PIC.H"
#include "main.h"
#include "menu/main.h"
#include "menu/pic.h"
#include "menu/player.h"
#include "physics_init.h"
#include "pic8.h"
#include "platform/implementation.h"
#include "menu/rec_list.h"
#include "recorder.h"
#include "state.h"
#include "qopen.h"
#include <directinput/scancodes.h>

static void show_intro_screen() {
    pic8* intro_screen = lockbackbuffer_pic(false);
    intro_screen->fill_box(BLACK_PALETTE_ID);
    blit8(intro_screen, Intro, SCREEN_WIDTH / 2 - Intro->get_width() / 2,
          SCREEN_HEIGHT / 2 - Intro->get_height() / 2);
    unlockbackbuffer_pic();
}

void menu_intro() {
    init_qopen();

    init_menu_pictures();

    State = new state;
    if (!State) {
        external_error("memory");
    }

    merge_states();
    eol_settings::sync_controls_to_state(State);

    init_physics_data();

    rec_list::build_cache();

    // test_player();

    // Load intro.pcx and hide the version
    Intro = new pic8("intro.pcx");
    Intro->fill_box(0, 410, Intro->get_width(), 450, Intro->gpixel(0, 409));
    Intro->add_transparency();

    // Display intro.pcx
    MenuPalette->set();
    show_intro_screen();

    init_sound();

    // Load globals
    Pabc1 = new abc8("kisbetu1.abc"); // "small letter 1"
    Pabc1->set_spacing(1);
    Pabc2 = new abc8("kisbetu2.abc"); // "small letter 2"
    Pabc2->set_spacing(1);

    init_renderer();

    Rec1 = new recorder;
    Rec2 = new recorder;

    seteditorpal();

    // Initialize stopwatch, just in case
    stopwatch_reset();

    // Await for key input before scrolling intro.pcx
    while (true) {
        handle_events();
        if (get_any_key_just_pressed()) {
            break;
        }
        show_intro_screen();
    }

    if (State->player_count == 0) {
        if (!menu_player_create(true)) {
            menu_exit();
        }
    } else {
        if (!menu_player_choose(true, false)) {
            menu_exit();
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
