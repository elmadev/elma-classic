#include "menu_intro.h"
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

void menu_intro() {
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

    // Load intro.pcx and hide the version
    Intro = new pic8("intro.pcx");
    Intro->fill_box(0, 410, Intro->get_width(), 450, Intro->gpixel(0, 409));
    spriteosit(Intro);
    pic8* static_intro_screen = new pic8(SCREEN_WIDTH, SCREEN_HEIGHT);
    static_intro_screen->fill_box(BLACK_PALETTE_ID);
    blit8(static_intro_screen, Intro, SCREEN_WIDTH / 2 - Intro->get_width() / 2,
          SCREEN_HEIGHT / 2 - Intro->get_height() / 2);

    // Display intro.pcx
    MenuPalette->set();
    bltfront(static_intro_screen);

    init_sound();

    // Load globals
    Pabc1 = new abc8("kisbetu1.abc"); // "small letter 1"
    Pabc1->set_spacing(1);
    Pabc2 = new abc8("kisbetu2.abc"); // "small letter 2"
    Pabc2->set_spacing(1);

    Rec1 = new recorder;
    Rec2 = new recorder;

    seteditorpal();

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

    menu_nav_entries_init();

    if (State->player_count == 0) {
        newjatekos(1, 0);
    } else {
        jatekosvalasztas(1, 0);
    }
    internal_error("menu_intro!");
}

void menu_exit() {
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
