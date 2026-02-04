#include "menu_controls.h"
#include <directinput/scancodes.h>
#include "eol_settings.h"
#include "keys.h"
#include "menu_nav.h"
#include "platform_impl.h"
#include "state.h"
#include <cstring>

const char* dik_to_string(DikScancode keycode) {
    switch (keycode) {
    case DIK_UNKNOWN:
        return "NONE";
    case DIK_1:
        return "1";
    case DIK_2:
        return "2";
    case DIK_3:
        return "3";
    case DIK_4:
        return "4";
    case DIK_5:
        return "5";
    case DIK_6:
        return "6";
    case DIK_7:
        return "7";
    case DIK_8:
        return "8";
    case DIK_9:
        return "9";
    case DIK_0:
        return "0";
    case DIK_MINUS:
        return "-";
    case DIK_EQUALS:
        return "=";
    case DIK_BACK:
        return "<-";
    case DIK_TAB:
        return "TAB";
    case DIK_Q:
        return "Q";
    case DIK_W:
        return "W";
    case DIK_E:
        return "E";
    case DIK_R:
        return "R";
    case DIK_T:
        return "T";
    case DIK_Y:
        return "Y";
    case DIK_U:
        return "U";
    case DIK_I:
        return "I";
    case DIK_O:
        return "O";
    case DIK_P:
        return "P";
    case DIK_LBRACKET:
        return "[";
    case DIK_RBRACKET:
        return "]";
    case DIK_RETURN:
        return "ENTER";
    case DIK_LCONTROL:
        return "LEFT CTRL";
    case DIK_A:
        return "A";
    case DIK_S:
        return "S";
    case DIK_D:
        return "D";
    case DIK_F:
        return "F";
    case DIK_G:
        return "G";
    case DIK_H:
        return "H";
    case DIK_J:
        return "J";
    case DIK_K:
        return "K";
    case DIK_L:
        return "L";
    case DIK_SEMICOLON:
        return ";";
    case DIK_APOSTROPHE:
        return "\"";
    case DIK_GRAVE:
        return "`";
    case DIK_LSHIFT:
        return "LEFT SHIFT";
    case DIK_BACKSLASH:
        return "\\";
    case DIK_Z:
        return "Z";
    case DIK_X:
        return "X";
    case DIK_C:
        return "C";
    case DIK_V:
        return "V";
    case DIK_B:
        return "B";
    case DIK_N:
        return "N";
    case DIK_M:
        return "M";
    case DIK_COMMA:
        return ",";
    case DIK_PERIOD:
        return ".";
    case DIK_SLASH:
        return "SLASH";
    case DIK_RSHIFT:
        return "RIGHT SHIFT";
    case DIK_MULTIPLY:
        return "PAD_*";
    case DIK_LMENU:
        return "LEFT ALT";
    case DIK_SPACE:
        return "SPACEBAR";
    case DIK_CAPITAL:
        return "CAPS LOCK";
    case DIK_F1:
        return "F1";
    case DIK_F2:
        return "F2";
    case DIK_F3:
        return "F3";
    case DIK_F4:
        return "F4";
    case DIK_F5:
        return "F5";
    case DIK_F6:
        return "F6";
    case DIK_F7:
        return "F7";
    case DIK_F8:
        return "F8";
    case DIK_F9:
        return "F9";
    case DIK_F10:
        return "F10";
    case DIK_NUMLOCK:
        return "NUM LOCK";
    case DIK_SCROLL:
        return "SCROLL LOCK";
    case DIK_NUMPAD7:
        return "PAD_HOME";
    case DIK_NUMPAD8:
        return "PAD_UP";
    case DIK_NUMPAD9:
        return "PAD_PGUP";
    case DIK_SUBTRACT:
        return "PAD_-";
    case DIK_NUMPAD4:
        return "PAD_LEFT";
    case DIK_NUMPAD5:
        return "PAD_5";
    case DIK_NUMPAD6:
        return "PAD_RIGHT";
    case DIK_ADD:
        return "PAD_+";
    case DIK_NUMPAD1:
        return "PAD_END";
    case DIK_NUMPAD2:
        return "PAD_DOWN";
    case DIK_NUMPAD3:
        return "PAD_PGDOWN";
    case DIK_NUMPAD0:
        return "PAD_INS";
    case DIK_DECIMAL:
        return "PAD_DEL";
    case DIK_F11:
        return "F11";
    case DIK_F12:
        return "F12";
    case DIK_F13:
        return "F13";
    case DIK_F14:
        return "F14";
    case DIK_F15:
        return "F15";
    case DIK_KANA:
        return "KANA";
    case DIK_CONVERT:
        return "CONVERT";
    case DIK_NOCONVERT:
        return "NOCONVERT";
    case DIK_YEN:
        return "YEN";
    case DIK_NUMPADEQUALS:
        return "PAD_=";
    case DIK_PREVTRACK:
        return "CIRCUMFLEX";
    case DIK_AT:
        return "AT";
    case DIK_COLON:
        return "COLON";
    case DIK_UNDERLINE:
        return "UNDERLINE";
    case DIK_KANJI:
        return "KANJI";
    case DIK_STOP:
        return "STOP";
    case DIK_AX:
        return "AX";
    case DIK_UNLABELED:
        return "UNLABELED";
    case DIK_NUMPADENTER:
        return "PAD_ENTER";
    case DIK_RCONTROL:
        return "RIGHT CTRL";
    case DIK_NUMPADCOMMA:
        return "COMMA";
    case DIK_DIVIDE:
        return "PAD_/";
    case DIK_SYSRQ:
        return "SYSRQ";
    case DIK_RMENU:
        return "RIGHT ALT";
    case DIK_HOME:
        return "HOME";
    case DIK_UP:
        return "UP ARROW";
    case DIK_PRIOR:
        return "PAGEUP";
    case DIK_LEFT:
        return "LEFT ARROW";
    case DIK_RIGHT:
        return "RIGHT ARROW";
    case DIK_END:
        return "END";
    case DIK_DOWN:
        return "DOWN ARROW";
    case DIK_NEXT:
        return "PAGE DOWN";
    case DIK_INSERT:
        return "INS";
    case DIK_DELETE:
        return "DEL";
    case DIK_LWIN:
        return "LEFT WIN";
    case DIK_RWIN:
        return "RIGHT WIN";
    case DIK_APPS:
        return "APPLICATION";
    }
    return NULL;
}

// A list of pointers to where the keys are stored (somewhere in a state class object)
typedef DikScancode* key_pointers[NAV_ENTRIES_RIGHT_MAX_LENGTH + 1];

static key_pointers UniversalKeys; // +/- and Screenshot
static key_pointers Player1Keys;
static key_pointers Player2Keys;
static key_pointers ReplayKeys;

constexpr int UNIVERSAL_KEYS_START = 4;
constexpr int UNIVERSAL_KEYS_END = UNIVERSAL_KEYS_START + 4;
constexpr int PLAYER_KEYS_START = 0;
constexpr int PLAYER_KEYS_END = PLAYER_KEYS_START + 10;
constexpr int REPLAY_KEYS_START = 0;
constexpr int REPLAY_KEYS_END = REPLAY_KEYS_START + 6;

// Setup the menu to display one control key
static void load_control(key_pointers keys, int offset, const char* label, int* key) {
    strcpy(NavEntriesLeft[offset], label);
    const char* key_text = dik_to_string(*key);
    char tmp[20] = "";
    if (!key_text) {
        sprintf(tmp, "Key code: %d", *key);
        key_text = tmp;
    }
    strcpy(NavEntriesRight[offset], key_text);
    keys[offset] = key;
}

// Await keypress to choose a new key for one control
static void prompt_control(int length, key_pointers keys, int index) {
    menu_nav nav;
    nav.selected_index = index;
    nav.x_left = 60;
    nav.x_right = 400;
    nav.y_entries = 86;
    nav.dy = 40;
    strcpy(nav.title, "Customize controls");
    strcpy(NavEntriesRight[index], "_");
    nav.setup(length, true);

    // Render only!
    nav.navigate(nullptr, 0, true);
    while (true) {
        handle_events();
        for (DikScancode keycode = 1; keycode < MaxKeycode; keycode++) {
            if (is_key_down(DIK_ESCAPE)) {
                return;
            }
            if (keycode == DIK_RETURN || keycode == DIK_ESCAPE) {
                continue;
            }
            if (!is_key_down(keycode)) {
                continue;
            }
            // Disallow multiple controls being mapped to the same key
            for (int i = UNIVERSAL_KEYS_START; i < UNIVERSAL_KEYS_END; i++) {
                if (*UniversalKeys[i] == keycode) {
                    *UniversalKeys[i] = 0;
                }
            }
            for (int i = PLAYER_KEYS_START; i < PLAYER_KEYS_END; i++) {
                if (*Player1Keys[i] == keycode) {
                    *Player1Keys[i] = 0;
                }
                if (*Player2Keys[i] == keycode) {
                    *Player2Keys[i] = 0;
                }
            }
            *keys[index] = keycode;
            return;
        }
        nav.render();
    }
}

// Setup the menu to display the universal controls
static void load_universal_controls() {
    strcpy(NavEntriesLeft[0], "Reset all controls to default");
    NavEntriesRight[0][0] = 0;
    strcpy(NavEntriesLeft[1], "Customize Player A");
    NavEntriesRight[1][0] = 0;
    strcpy(NavEntriesLeft[2], "Customize Player B");
    NavEntriesRight[2][0] = 0;
    strcpy(NavEntriesLeft[3], "Customize Replay VCR");
    NavEntriesRight[3][0] = 0;
    int i = UNIVERSAL_KEYS_START;
    load_control(UniversalKeys, i++, "Inc. Screen Size", &State->key_increase_screen_size);
    load_control(UniversalKeys, i++, "Dec. Screen Size", &State->key_decrease_screen_size);
    load_control(UniversalKeys, i++, "Make a Screenshot", &State->key_screenshot);
    load_control(UniversalKeys, i++, "Escape Alias", &State->key_escape_alias);
}

// Setup the menu to display the replay controls
static void load_replay_controls(key_pointers keys) {
    int i = REPLAY_KEYS_START;
    load_control(keys, i++, "Fast forward 2x", &State->key_replay_fast_2x);
    load_control(keys, i++, "Fast forward 4x", &State->key_replay_fast_4x);
    load_control(keys, i++, "Fast forward 8x", &State->key_replay_fast_8x);
    load_control(keys, i++, "Slow motion 2x", &State->key_replay_slow_2x);
    load_control(keys, i++, "Slow motion 4x", &State->key_replay_slow_4x);
    load_control(keys, i++, "Pause", &State->key_replay_pause);
}

// Setup the menu to display one player's controls
static void load_player_controls(key_pointers keys, player_keys* player_controls) {
    int i = PLAYER_KEYS_START;
    load_control(keys, i++, "Throttle", &player_controls->gas);
    load_control(keys, i++, "Brake", &player_controls->brake);
    load_control(keys, i++, "Brake Alias", &player_controls->brake_alias);
    load_control(keys, i++, "Rotate left", &player_controls->left_volt);
    load_control(keys, i++, "Rotate right", &player_controls->right_volt);
    load_control(keys, i++, "Alovolt", &player_controls->alovolt);
    load_control(keys, i++, "Change direction", &player_controls->turn);
    load_control(keys, i++, "Toggle Navigator", &player_controls->toggle_minimap);
    load_control(keys, i++, "Toggle Time", &player_controls->toggle_timer);
    load_control(keys, i++, "Toggle Show/Hide", &player_controls->toggle_visibility);
}

// Menu to change controls for one player
static void menu_customize_player(key_pointers keys, player_keys* player_controls,
                                  const char* player_letter) {
    int choice = 0;
    while (true) {
        menu_nav nav;
        nav.selected_index = choice;
        nav.x_left = 60;
        nav.x_right = 400;
        nav.y_entries = 86;
        nav.dy = 40;
        strcpy(nav.title, "Customize Player ");
        strcat(nav.title, player_letter);

        load_player_controls(keys, player_controls);

        nav.setup(PLAYER_KEYS_END, true);

        choice = nav.navigate();
        if (choice < 0) {
            return;
        }
        if (choice >= 0 && choice < PLAYER_KEYS_END) {
            prompt_control(PLAYER_KEYS_END, keys, choice);
        }
    }
}

// Menu to change replay controls
static void menu_customize_replay(key_pointers keys) {
    int choice = 0;
    while (true) {
        menu_nav nav;
        nav.selected_index = choice;
        nav.x_left = 60;
        nav.x_right = 400;
        nav.y_entries = 86;
        nav.dy = 40;
        strcpy(nav.title, "Customize Replay VCR");

        load_replay_controls(keys);

        nav.setup(REPLAY_KEYS_END, true);

        choice = nav.navigate();
        if (choice < 0) {
            return;
        }
        if (choice >= 0 && choice < REPLAY_KEYS_END) {
            prompt_control(REPLAY_KEYS_END, keys, choice);
        }
    }
}

// Menu to customize universal controls or select a player
void menu_customize_controls() {
    // Initialize these pointers so we can check/modify the values in prompt_control
    load_player_controls(Player1Keys, &State->keys1);
    load_player_controls(Player2Keys, &State->keys2);

    int choice = 0;
    while (true) {
        menu_nav nav;
        nav.selected_index = choice;
        nav.x_left = 60;
        nav.x_right = 400;
        nav.y_entries = 86;
        nav.dy = 40;
        strcpy(nav.title, "Customize controls");

        load_universal_controls();

        nav.setup(UNIVERSAL_KEYS_END, true);

        choice = nav.navigate();
        if (choice < 0) {
            eol_settings::sync_controls_from_state(State);
            return;
        }
        if (choice == 0) {
            // Reset all controls to default
            State->reset_keys();
            load_player_controls(Player1Keys, &State->keys1);
            load_player_controls(Player2Keys, &State->keys2);
            load_replay_controls(ReplayKeys);
            load_universal_controls();
        }
        if (choice == 1) {
            // Customize Player A
            menu_customize_player(Player1Keys, &State->keys1, "A");
        }
        if (choice == 2) {
            // Customize Player B
            menu_customize_player(Player2Keys, &State->keys2, "B");
        }
        if (choice == 3) {
            menu_customize_replay(ReplayKeys);
        }
        if (choice >= UNIVERSAL_KEYS_START && choice < UNIVERSAL_KEYS_END) {
            // Modify universal control
            prompt_control(UNIVERSAL_KEYS_END, UniversalKeys, choice);
        }
    }
}
