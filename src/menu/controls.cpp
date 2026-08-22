#include "menu/controls.h"
#include "eol/settings.h"
#include "game/state.h"
#include "main.h"
#include "menu/nav.h"
#include "platform/implementation.h"
#include "platform/scancode.h"
#include <format>
#include <ranges>
#include <vector>

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
    load_control(nav, UniversalKeys, "Toggle Video Detail", &State->key_high_quality);
    load_control(nav, UniversalKeys, "Toggle Ground/Sky", &State->key_default_ground_sky);
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
    load_control(nav, FunctionKeys, "Finished Times", &State->key_finished_times);
    load_control(nav, FunctionKeys, "Finished Times Filter",
                 &State->key_cycle_finished_times_filter);
    load_control(nav, FunctionKeys, "Clear Finished Times", &State->key_clear_finished_times);
    load_control(nav, FunctionKeys, "Chat", &State->key_chat);
    load_control(nav, FunctionKeys, "Show Chat", &State->key_show_chat);
    load_control(nav, FunctionKeys, "Team Chat", &State->key_team_chat);
    load_control(nav, FunctionKeys, "PM Next Kuski", &State->key_pm_next_kuski);
    load_control(nav, FunctionKeys, "PM Prev Kuski", &State->key_pm_prev_kuski);
    load_control(nav, FunctionKeys, "PM Clear", &State->key_pm_clear_kuski);
    load_control(nav, FunctionKeys, "Hide Battle Status", &State->key_battle_status);
    load_control(nav, FunctionKeys, "Hide Battle Leader", &State->key_battle_leader);
    load_control(nav, FunctionKeys, "Show Speedometer", &State->key_speedometer);
    load_control(nav, FunctionKeys, "Reconnect", &State->key_reconnect);
    load_control(nav, FunctionKeys, "Disconnect", &State->key_disconnect);
    load_control(nav, FunctionKeys, "Show Last Apple Time", &State->key_toggle_last_apple_time);
    load_control(nav, FunctionKeys, "Show One-Wheel Status", &State->key_toggle_one_wheel_status);
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
