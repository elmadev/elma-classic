#include "menu/controls.h"
#include "eol/settings.h"
#include "game/state.h"
#include "main.h"
#include "menu/nav.h"
#include "platform/implementation.h"
#include "platform/scancode.h"
#include <format>
#include <functional>
#include <optional>
#include <ranges>
#include <vector>

struct control {
    bool combo;
    std::function<combo_scancode()> getter;
    std::function<void(combo_scancode)> setter;

    control()
        : getter([]() { return DIK_NONE; }) {}
    control(bool combo, std::function<combo_scancode()> getter,
            std::function<void(combo_scancode)> setter)
        : combo(combo),
          getter(std::move(getter)),
          setter(std::move(setter)) {}
};

using controls = std::vector<control>;
constexpr int UNIVERSAL_KEYS_START = 5;
static controls UniversalKeys;
static controls Player1Keys;
static controls Player2Keys;
static controls ReplayKeys;
static controls FunctionKeys;

static void load_control(menu_nav* nav, controls& keys, std::string label, bool combo,
                         const std::function<combo_scancode()>& getter,
                         const std::function<void(combo_scancode)>& setter) {
    control& ctrl = keys.emplace_back(combo, getter, setter);
    if (!nav) {
        return;
    }
    nav->add_row(std::move(label), dik_to_string(ctrl.getter()));
    if (keys.size() != nav->row_count()) {
        internal_error("load_control controls desynced from menu_nav!");
    }
}

static void load_control_state(menu_nav* nav, controls& keys, std::string label, DikScancode* key) {
    load_control(
        nav, keys, std::move(label), false, [key]() { return combo_scancode(*key); },
        [key](combo_scancode new_key) { *key = new_key.key; });
}

#define LOAD_SETTING_DIK(nav, keys, label, setting)                                                \
    do {                                                                                           \
        static_assert(std::is_same_v<decltype(EolSettings->setting()), DikScancode>);              \
        load_control(                                                                              \
            nav, keys, label, false, []() { return combo_scancode(EolSettings->setting()); },      \
            [](combo_scancode new_key) { EolSettings->persist_##setting(new_key.key); });          \
    } while (0)

#define LOAD_SETTING_COMBO(nav, keys, label, setting)                                              \
    do {                                                                                           \
        static_assert(std::is_same_v<decltype(EolSettings->setting()), combo_scancode>);           \
        load_control(                                                                              \
            nav, keys, label, true, []() { return EolSettings->setting(); },                       \
            [](combo_scancode new_key) { EolSettings->persist_##setting(new_key); });              \
    } while (0)

// Disallow multiple controls being mapped to the same key
static void deduplicate_controls(combo_scancode keycode) {
    auto clear_matches = [keycode](const controls& keys) {
        for (const control& key : keys) {
            if (key.getter() == keycode) {
                key.setter(DIK_NONE);
            }
        }
    };
    clear_matches(UniversalKeys);
    clear_matches(Player1Keys);
    clear_matches(Player2Keys);
}

static std::optional<DikScancode> prompt_control_dik(menu_nav& nav) {
    // Render only!
    nav.navigate(true);
    while (true) {
        handle_events();
        if (is_key_down(DIK_ESCAPE)) {
            return std::nullopt;
        }
        for (DikScancode keycode = 1; keycode < MaxKeycode; keycode++) {
            if (keycode == DIK_RETURN || keycode == DIK_ESCAPE) {
                continue;
            }
            if (!is_key_down(keycode)) {
                continue;
            }
            deduplicate_controls(keycode);
            return keycode;
        }
        nav.render();
    }
}

static bool is_valid_modifier(DikScancode key) {
    return std::ranges::find(MODIFIERS, key) != std::ranges::end(MODIFIERS);
}

static std::optional<combo_scancode> prompt_control_combo(menu_nav& nav) {
    // Render only!
    nav.navigate(true);
    while (true) {
        handle_events();
        if (is_key_down(DIK_ESCAPE)) {
            return std::nullopt;
        }
        for (DikScancode keycode = 1; keycode < MaxKeycode; keycode++) {
            if (keycode == DIK_RETURN || keycode == DIK_ESCAPE) {
                continue;
            }
            if (is_valid_modifier(keycode)) {
                continue;
            }
            if (!is_key_down(keycode)) {
                continue;
            }
            combo_scancode key = keycode;
            auto modifier = std::ranges::find_if(MODIFIERS, is_key_down);
            if (modifier != MODIFIERS.end()) {
                key.modifier = *modifier;
            }
            return key;
        }
        nav.render();
    }
}

// Await keypress to choose a new key for one control
static void prompt_control(menu_nav& nav, controls& keys, int index) {
    nav.entry_right(index) = "_";
    control& ctrl = keys[index];
    if (ctrl.combo) {
        std::optional<combo_scancode> new_key = prompt_control_combo(nav);
        if (new_key) {
            ctrl.setter(*new_key);
        }
    } else {
        std::optional<DikScancode> new_key = prompt_control_dik(nav);
        if (new_key) {
            ctrl.setter(*new_key);
        }
    }
}

// Setup the menu to display the universal controls
static void load_universal_controls(menu_nav* nav) {
    UniversalKeys.resize(UNIVERSAL_KEYS_START);
    load_control_state(nav, UniversalKeys, "Inc. Screen Size", &State->key_increase_screen_size);
    load_control_state(nav, UniversalKeys, "Dec. Screen Size", &State->key_decrease_screen_size);
    load_control_state(nav, UniversalKeys, "Make a Screenshot", &State->key_screenshot);
    LOAD_SETTING_DIK(nav, UniversalKeys, "Escape Alias", escape_alias_key);
    LOAD_SETTING_COMBO(nav, UniversalKeys, "Toggle Video Detail", high_quality_key);
    LOAD_SETTING_COMBO(nav, UniversalKeys, "Toggle Ground/Sky", default_ground_sky_key);
    LOAD_SETTING_COMBO(nav, UniversalKeys, "Zoom Out", zoom_out_key);
    LOAD_SETTING_COMBO(nav, UniversalKeys, "Zoom In", zoom_in_key);
}

// Setup the menu to display the replay controls
static void load_replay_controls(menu_nav* nav) {
    ReplayKeys.resize(0);
    LOAD_SETTING_DIK(nav, ReplayKeys, "Fast forward 2x", replay_fast_2x_key);
    LOAD_SETTING_DIK(nav, ReplayKeys, "Fast forward 4x", replay_fast_4x_key);
    LOAD_SETTING_DIK(nav, ReplayKeys, "Fast forward 8x", replay_fast_8x_key);
    LOAD_SETTING_DIK(nav, ReplayKeys, "Slow motion 2x", replay_slow_2x_key);
    LOAD_SETTING_DIK(nav, ReplayKeys, "Slow motion 4x", replay_slow_4x_key);
    LOAD_SETTING_DIK(nav, ReplayKeys, "Pause", replay_pause_key);
    LOAD_SETTING_DIK(nav, ReplayKeys, "Rewind", replay_rewind_key);
}

// Setup the menu to display one player's controls
static void load_player_controls(menu_nav* nav, controls& keys, player_keys* player_controls) {
    keys.resize(0);
    load_control_state(nav, keys, "Throttle", &player_controls->gas);
    load_control_state(nav, keys, "Brake", &player_controls->brake);
    load_control_state(nav, keys, "Brake Alias", &player_controls->brake_alias);
    load_control_state(nav, keys, "One Frame Brake", &player_controls->one_frame_brake);
    load_control_state(nav, keys, "Rotate left", &player_controls->left_volt);
    load_control_state(nav, keys, "Rotate right", &player_controls->right_volt);
    load_control_state(nav, keys, "Alovolt", &player_controls->alovolt);
    load_control_state(nav, keys, "Change direction", &player_controls->turn);
    load_control_state(nav, keys, "Toggle Minimap", &player_controls->toggle_minimap);
    load_control_state(nav, keys, "Toggle Time", &player_controls->toggle_timer);
    load_control_state(nav, keys, "Toggle Show/Hide", &player_controls->toggle_visibility);
}

// Setup the menu to display EOL function key controls
static void load_function_controls(menu_nav* nav) {
    FunctionKeys.resize(0);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Show Others", show_others_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Shown To", shown_to_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Spy Next Kuski", spy_next_kuski_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Spy Prev Kuski", spy_prev_kuski_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Battle Queue", battle_queue_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Download Battle Level", download_battle_level_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Download Level", download_level_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Players Online", players_online_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Best Times", best_times_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Battle Results", battle_results_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Finished Times", finished_times_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Finished Times Filter", cycle_finished_times_filter_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Clear Finished Times", clear_finished_times_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Chat", chat_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Show Chat", show_chat_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Team Chat", team_chat_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "PM Next Kuski", pm_next_kuski_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "PM Prev Kuski", pm_prev_kuski_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "PM Clear", pm_clear_kuski_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Hide Battle Status", battle_status_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Hide Battle Leader", battle_leader_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Show Speedometer", speedometer_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Download Battle Rec", download_battle_rec_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Reconnect", reconnect_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Disconnect", disconnect_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Show Last Apple Time", toggle_last_apple_time_key);
    LOAD_SETTING_COMBO(nav, FunctionKeys, "Show One-Wheel Status", toggle_one_wheel_status_key);
}

// Menu to change controls for one player
static void menu_customize_player(controls& keys, player_keys* player_controls,
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
