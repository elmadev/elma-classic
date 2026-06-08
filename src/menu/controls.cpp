#include "menu/controls.h"
#include "eol/settings.h"
#include "game/state.h"
#include "main.h"
#include "menu/nav.h"
#include "platform/implementation.h"
#include "platform/scancode.h"
#include "platform/text_input.h"
#include <cstring>
#include <format>
#include <ranges>
#include <vector>

std::string dik_to_string(DikScancode keycode) {
    switch (keycode) {
    case DIK_NONE:
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
        return "L CTRL";
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
        return "L SHIFT";
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
        return "R SHIFT";
    case DIK_MULTIPLY:
        return "PAD_*";
    case DIK_LMENU:
        return "L ALT";
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
        return "R CTRL";
    case DIK_NUMPADCOMMA:
        return "COMMA";
    case DIK_DIVIDE:
        return "PAD_/";
    case DIK_SYSRQ:
        return "SYSRQ";
    case DIK_RMENU:
        return "R ALT";
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
        return "L WIN";
    case DIK_RWIN:
        return "R WIN";
    case DIK_APPS:
        return "APPLICATION";
    }
    return std::format("Key code: {}", keycode);
}

std::string dik_to_string(const combo_scancode& keycode) {
    std::string modifier = (keycode.modifier ? dik_to_string(keycode.modifier) + " + " : "");
    std::string key = dik_to_string(keycode.key);
    return modifier + key;
}

// A list of pointers to where the keys are stored (somewhere in a state class object)
struct scancode_pointer {
    DikScancode* dik;
    combo_scancode* combo;
};
using key_pointers = std::vector<scancode_pointer>;

constexpr int UNIVERSAL_KEYS_START = 5;
static key_pointers UniversalKeys; // +/- and Screenshot
static key_pointers Player1Keys;
static key_pointers Player2Keys;
static key_pointers ReplayKeys;
static key_pointers FunctionKeys;

// Setup the menu to display one control key
template <typename Scancode>
static void load_control(menu_nav* nav, key_pointers& keys, std::string label, Scancode* key) {
    if constexpr (std::is_same_v<Scancode, DikScancode>) {
        keys.emplace_back(scancode_pointer{key, nullptr});
    } else {
        keys.emplace_back(scancode_pointer{nullptr, key});
    }
    if (!nav) {
        return;
    }
    nav->add_row(std::move(label), dik_to_string(*key));
    if (keys.size() != nav->row_count()) {
        internal_error("load_control key_pointers desynced from menu_nav!");
    }
}

// Disallow multiple controls being mapped to the same key
static void deduplicate_controls(DikScancode keycode) {
    auto clear_matches = [keycode](const key_pointers& keys) {
        for (const scancode_pointer& key : keys) {
            if (key.dik && *key.dik == keycode) {
                *key.dik = DIK_NONE;
            }
        }
    };
    clear_matches(UniversalKeys);
    clear_matches(Player1Keys);
    clear_matches(Player2Keys);
}

static void prompt_control_dik(menu_nav& nav, DikScancode* key) {
    // Render only!
    nav.navigate(true);
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
            deduplicate_controls(keycode);
            *key = keycode;
            return;
        }
        nav.render();
    }
}

static bool is_valid_modifier(DikScancode key) {
    return std::ranges::find(MODIFIERS, key) != std::ranges::end(MODIFIERS);
}

static void prompt_control_combo(menu_nav& nav, combo_scancode* key) {
    // Render only!
    nav.navigate(true);
    while (true) {
        handle_events();
        for (DikScancode keycode = 1; keycode < MaxKeycode; keycode++) {
            if (is_key_down(DIK_ESCAPE)) {
                return;
            }
            if (keycode == DIK_RETURN || keycode == DIK_ESCAPE) {
                continue;
            }
            if (is_valid_modifier(keycode)) {
                continue;
            }
            if (!is_key_down(keycode)) {
                continue;
            }
            *key = keycode;
            auto modifier = std::ranges::find_if(MODIFIERS, is_key_down);
            if (modifier != MODIFIERS.end()) {
                key->modifier = *modifier;
            }
            return;
        }
        nav.render();
    }
}

// Await keypress to choose a new key for one control
static void prompt_control(menu_nav& nav, key_pointers& keys, int index) {
    nav.entry_right(index) = "_";
    if (keys[index].dik) {
        prompt_control_dik(nav, keys[index].dik);
    } else {
        prompt_control_combo(nav, keys[index].combo);
    }
}

// Setup the menu to display the universal controls
static void load_universal_controls(menu_nav* nav) {
    UniversalKeys.resize(UNIVERSAL_KEYS_START);
    load_control(nav, UniversalKeys, "Inc. Screen Size", &State->key_increase_screen_size);
    load_control(nav, UniversalKeys, "Dec. Screen Size", &State->key_decrease_screen_size);
    load_control(nav, UniversalKeys, "Make a Screenshot", &State->key_screenshot);
    load_control(nav, UniversalKeys, "Escape Alias", &State->key_escape_alias);
}

// Setup the menu to display the replay controls
static void load_replay_controls(menu_nav* nav) {
    ReplayKeys.resize(0);
    load_control(nav, ReplayKeys, "Fast forward 2x", &State->key_replay_fast_2x);
    load_control(nav, ReplayKeys, "Fast forward 4x", &State->key_replay_fast_4x);
    load_control(nav, ReplayKeys, "Fast forward 8x", &State->key_replay_fast_8x);
    load_control(nav, ReplayKeys, "Slow motion 2x", &State->key_replay_slow_2x);
    load_control(nav, ReplayKeys, "Slow motion 4x", &State->key_replay_slow_4x);
    load_control(nav, ReplayKeys, "Pause", &State->key_replay_pause);
    load_control(nav, ReplayKeys, "Rewind", &State->key_replay_rewind);
}

// Setup the menu to display one player's controls
static void load_player_controls(menu_nav* nav, key_pointers& keys, player_keys* player_controls) {
    keys.resize(0);
    load_control(nav, keys, "Throttle", &player_controls->gas);
    load_control(nav, keys, "Brake", &player_controls->brake);
    load_control(nav, keys, "Brake Alias", &player_controls->brake_alias);
    load_control(nav, keys, "One Frame Brake", &player_controls->one_frame_brake);
    load_control(nav, keys, "Rotate left", &player_controls->left_volt);
    load_control(nav, keys, "Rotate right", &player_controls->right_volt);
    load_control(nav, keys, "Alovolt", &player_controls->alovolt);
    load_control(nav, keys, "Change direction", &player_controls->turn);
    load_control(nav, keys, "Toggle Minimap", &player_controls->toggle_minimap);
    load_control(nav, keys, "Toggle Time", &player_controls->toggle_timer);
    load_control(nav, keys, "Toggle Show/Hide", &player_controls->toggle_visibility);
}

// Setup the menu to display EOL function key controls
static void load_function_controls(menu_nav* nav) {
    FunctionKeys.resize(0);
    load_control(nav, FunctionKeys, "Show Others", &State->key_show_others);
    load_control(nav, FunctionKeys, "Spy Next Kuski", &State->key_spy_next_kuski);
    load_control(nav, FunctionKeys, "Spy Prev Kuski", &State->key_spy_prev_kuski);
    load_control(nav, FunctionKeys, "Battle Queue", &State->key_battle_queue);
    load_control(nav, FunctionKeys, "Download Battle Level", &State->key_download_battle_level);
    load_control(nav, FunctionKeys, "Download Level", &State->key_download_level);
    load_control(nav, FunctionKeys, "Players Online", &State->key_players_online);
    load_control(nav, FunctionKeys, "Battle Results", &State->key_battle_results);
    load_control(nav, FunctionKeys, "Chat", &State->key_chat);
    load_control(nav, FunctionKeys, "Hide Battle Status", &State->key_battle_status);
    load_control(nav, FunctionKeys, "Hide Battle Leader", &State->key_battle_leader);
}

// Menu to change controls for one player
static void menu_customize_player(key_pointers& keys, player_keys* player_controls,
                                  char player_letter) {
    int choice = 0;
    while (true) {
        menu_nav nav(std::format("Customize Player {}", player_letter));
        nav.select_row(choice);
        nav.x_left = 60;
        nav.x_right = 400;
        nav.y_entries = 86;
        nav.dy = 40;

        load_player_controls(&nav, keys, player_controls);

        choice = nav.navigate();
        if (choice < 0) {
            return;
        }
        prompt_control(nav, keys, choice);
    }
}

// Menu to change replay controls
static void menu_customize_replay() {
    int choice = 0;
    while (true) {
        menu_nav nav("Customize Replay VCR");
        nav.select_row(choice);
        nav.x_left = 60;
        nav.x_right = 400;
        nav.y_entries = 86;
        nav.dy = 40;

        load_replay_controls(&nav);

        choice = nav.navigate();
        if (choice < 0) {
            return;
        }
        prompt_control(nav, ReplayKeys, choice);
    }
}

// Menu to change EOL function keys
static void menu_customize_function() {
    int choice = 0;
    while (true) {
        menu_nav nav("Customize Function Keys");
        nav.select_row(choice);
        nav.x_left = 20;
        nav.x_right = 400;
        nav.y_entries = 86;
        nav.dy = 40;

        load_function_controls(&nav);

        choice = nav.navigate();
        if (choice < 0) {
            return;
        }
        prompt_control(nav, FunctionKeys, choice);
    }
}

// Menu to customize universal controls or select a player
void menu_customize_controls() {
    // Initialize these pointers so we can check/modify the values in prompt_control
    load_player_controls(nullptr, Player1Keys, &State->keys1);
    load_player_controls(nullptr, Player2Keys, &State->keys2);

    int choice = 0;
    while (true) {
        menu_nav nav("Customize controls");
        nav.select_row(choice);
        nav.x_left = 60;
        nav.x_right = 400;
        nav.y_entries = 86;
        nav.dy = 40;

        nav.add_row("Reset all controls to default", NAV_FUNC() { State->reset_keys(); });

        nav.add_row(
            "Customize Player A",
            NAV_FUNC() { menu_customize_player(Player1Keys, &State->keys1, 'A'); });

        nav.add_row(
            "Customize Player B",
            NAV_FUNC() { menu_customize_player(Player2Keys, &State->keys2, 'B'); });

        nav.add_row("Customize Replay VCR", NAV_FUNC() { menu_customize_replay(); });

        nav.add_row("Customize Function Keys", NAV_FUNC() { menu_customize_function(); });

        load_universal_controls(&nav);

        choice = nav.navigate();
        if (choice < 0) {
            eol_settings::sync_controls_from_state(State);
            return;
        }
        if (choice >= UNIVERSAL_KEYS_START) {
            prompt_control(nav, UniversalKeys, choice);
        }
    }
}
