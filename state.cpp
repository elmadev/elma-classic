#include "state.h"
#include "best_times.h"
#include "eol_settings.h"
#include "level.h"
#include "main.h"
#include "menu_dialog.h"
#include <cstring>
#include <directinput/scancodes.h>
#include <filesystem>

constexpr int STATE_VERSION = 200;

constexpr int STATE_MAGICNUMBER_SHAREWARE = 123432112;
constexpr int STATE_MAGICNUMBER_REGISTERED = 123432221;

constexpr char STATE_FILENAME[] = "state.dat";

static void read_encrypted(void* buffer, int length, FILE* h, const char* filename) {
    if (fread(buffer, 1, length, h) != length) {
        external_error("Corrupt file, please delete it!", filename);
    }
    unsigned char* pc = (unsigned char*)buffer;
    short a = 23;
    short b = 9782;
    short c = 3391;

    for (int i = 0; i < length; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
}

static void write_encrypted(void* buffer, int length, FILE* h) {
    unsigned char* pc = (unsigned char*)buffer;
    short a = 23;
    short b = 9782;
    short c = 3391;

    for (int i = 0; i < length; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
    if (fwrite(buffer, 1, length, h) != length) {
        internal_error("Unable to write to state.dat file!");
    }
    a = 23;
    b = 9782;
    c = 3391;

    for (int i = 0; i < length; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
}

state::state(const char* filename) {
    memset(toptens, 0, sizeof(toptens));
    memset(players, 0, sizeof(players));
    player_count = 0;
    player1[0] = 0;
    player2[0] = 0;
    sound_on = 1;
    compatibility_mode = 0;
    single = 1;
    flag_tag = 0;
    player1_bike1 = 1;
    high_quality = 1;
    animated_objects = 1;
    animated_menus = 1;
    editor_filename[0] = 0;
    external_filename[0] = 0;
    reset_keys();

    if (!filename) {
        filename = STATE_FILENAME;
    }

    // Recreate state.dat if it doesn't exist
    if (!std::filesystem::exists(filename)) {
        save();
        return;
    }

    FILE* h = fopen(filename, "rb");
    if (!h) {
        internal_error("Cannot open state file!: ", filename);
    }

    int version = 0;
    read_encrypted(&version, sizeof(version), h, filename);
    if (version != STATE_VERSION) {
        external_error("File version is incorrect!", "Please rename it!", filename);
    }
    read_encrypted(toptens, sizeof(toptens), h, filename);
    read_encrypted(players, sizeof(players), h, filename);
    read_encrypted(&player_count, sizeof(player_count), h, filename);
    read_encrypted(player1, sizeof(player1), h, filename);
    read_encrypted(player2, sizeof(player2), h, filename);
    read_encrypted(&sound_on, sizeof(sound_on), h, filename);
    read_encrypted(&compatibility_mode, sizeof(compatibility_mode), h, filename);
    read_encrypted(&single, sizeof(single), h, filename);
    read_encrypted(&flag_tag, sizeof(flag_tag), h, filename);
    read_encrypted(&player1_bike1, sizeof(player1_bike1), h, filename);
    read_encrypted(&high_quality, sizeof(high_quality), h, filename);
    read_encrypted(&animated_objects, sizeof(animated_objects), h, filename);
    read_encrypted(&animated_menus, sizeof(animated_menus), h, filename);
    read_encrypted(&keys1, sizeof(player_state_keys), h, filename);
    read_encrypted(&keys2, sizeof(player_state_keys), h, filename);
    read_encrypted(&key_increase_screen_size, sizeof(key_increase_screen_size), h, filename);
    read_encrypted(&key_decrease_screen_size, sizeof(key_decrease_screen_size), h, filename);
    read_encrypted(&key_screenshot, sizeof(key_decrease_screen_size), h, filename);
    read_encrypted(&editor_filename, sizeof(editor_filename), h, filename);
    read_encrypted(&external_filename, sizeof(external_filename), h, filename);

    int magic_number = 0;
    if (fread(&magic_number, 1, sizeof(magic_number), h) != sizeof(magic_number)) {
        external_error("Corrupt file, please rename it!", filename);
    }
    if (magic_number != STATE_MAGICNUMBER_SHAREWARE &&
        magic_number != STATE_MAGICNUMBER_REGISTERED) {
        external_error("Corrupt file, please rename it!", filename);
    }
    fclose(h);
}

// Reload top-ten data from top ten. Not clear why this is necessary.
void state::reload_toptens() {
    const char* filename = STATE_FILENAME;
    FILE* h = fopen(filename, "rb");
    while (!h) {
        menu_dialog("The state.dat file cannot be opened!",
                    "Please make sure state.dat is accessible,", "then press enter!");
        h = fopen(filename, "rb");
    }

    int version = 0;
    read_encrypted(&version, 4, h, filename);
    if (version != STATE_VERSION) {
        external_error("File version is incorrect!", "Please rename it!", filename);
    }

    read_encrypted(toptens, sizeof(toptens), h, filename);

    fclose(h);
}

void state::save() {
    FILE* h = fopen(STATE_FILENAME, "wb");
    if (!h) {
        external_error("Could not open for write file!: ", STATE_FILENAME);
    }

    int version = STATE_VERSION;
    write_encrypted(&version, sizeof(version), h);
    write_encrypted(toptens, sizeof(toptens), h);
    write_encrypted(players, sizeof(players), h);
    write_encrypted(&player_count, sizeof(player_count), h);
    write_encrypted(player1, sizeof(player1), h);
    write_encrypted(player2, sizeof(player2), h);
    write_encrypted(&sound_on, sizeof(sound_on), h);
    write_encrypted(&compatibility_mode, sizeof(compatibility_mode), h);
    write_encrypted(&single, sizeof(single), h);
    write_encrypted(&flag_tag, sizeof(flag_tag), h);
    write_encrypted(&player1_bike1, sizeof(player1_bike1), h);
    write_encrypted(&high_quality, sizeof(high_quality), h);
    write_encrypted(&animated_objects, sizeof(animated_objects), h);
    write_encrypted(&animated_menus, sizeof(animated_menus), h);
    write_encrypted(&keys1, sizeof(player_state_keys), h);
    write_encrypted(&keys2, sizeof(player_state_keys), h);
    write_encrypted(&key_increase_screen_size, sizeof(key_increase_screen_size), h);
    write_encrypted(&key_decrease_screen_size, sizeof(key_decrease_screen_size), h);
    write_encrypted(&key_screenshot, sizeof(key_decrease_screen_size), h);
    write_encrypted(&editor_filename, sizeof(editor_filename), h);
    write_encrypted(&external_filename, sizeof(external_filename), h);

    int magic_number = STATE_MAGICNUMBER_REGISTERED;
    if (fwrite(&magic_number, 1, sizeof(magic_number), h) != sizeof(magic_number)) {
        internal_error("Cannot write to state file: ", STATE_FILENAME);
    }

    fclose(h);
}

static void write_stats_topten(FILE* h, topten* tten, bool single) {
    for (int i = 0; i < tten->times_count; i++) {
        char time_text[40];
        centiseconds_to_string(tten->times[i], time_text, true);
        fprintf(h, "    ");
        fprintf(h, "%s", time_text);

        for (int alignment = 0; alignment < (12 - strlen(time_text)); alignment++) {
            fprintf(h, " ");
        }

        fprintf(h, "%s", tten->names1[i]);
        if (!single) {
            fprintf(h, ", %s", tten->names2[i]);
        }
        fprintf(h, "\n");
    }
}

// Default time if uncompleted level: 10 minutes
constexpr int STATS_MAX_TIME = 100 * 60 * 10;

// Print total time of all players combined
void state::write_stats_anonymous_total_time(FILE* h, bool single, const char* text1,
                                             const char* text2, const char* text3) {
    int total_time = 0;
    for (int i = 0; i < INTERNAL_LEVEL_COUNT - 1; i++) {
        int best_time = 100000000;
        // Both single and multi can take the single best time
        topten* tten_single = &toptens[i].single;
        if (tten_single->times_count > 0) {
            best_time = tten_single->times[0];
        }
        if (!single) {
            // Only multi can take the multi best time
            topten* tten_multi = &toptens[i].multi;
            if (tten_multi->times_count > 0 && best_time > tten_multi->times[0]) {
                best_time = tten_multi->times[0];
            }
        }
        if (best_time >= STATS_MAX_TIME) {
            total_time += STATS_MAX_TIME;
        } else {
            total_time += best_time;
        }
    }

    fprintf(h, "%s\n%s\n%s\n", text1, text2, text3);

    char time_text[40];
    centiseconds_to_string(total_time, time_text, true);
    fprintf(h, "%s", time_text);
    fprintf(h, "\n\n");
}

// Print total time of one player
void state::write_stats_player_total_time(FILE* h, const char* player_name, bool single) {
    int total_time = 0;
    for (int i = 0; i < INTERNAL_LEVEL_COUNT - 1; i++) {
        int best_time = 100000000;

        // Both single and multi can take the single best time
        topten* tten_single = &toptens[i].single;
        for (int j = 0; j < tten_single->times_count; j++) {
            if (strcmp(player_name, tten_single->names1[j]) == 0) {
                best_time = tten_single->times[j];
                break;
            }
        }

        if (!single) {
            // Only multi can take the multi best time
            topten* tten_multi = &toptens[i].multi;
            for (int j = 0; j < tten_multi->times_count; j++) {
                if (strcmp(player_name, tten_multi->names1[j]) == 0 ||
                    strcmp(player_name, tten_multi->names2[j]) == 0) {
                    if (best_time > tten_multi->times[j]) {
                        best_time = tten_multi->times[j];
                    }
                    break;
                }
            }
        }

        if (best_time >= STATS_MAX_TIME) {
            total_time += STATS_MAX_TIME;
        } else {
            total_time += best_time;
        }
    }
    char time_text[40];
    centiseconds_to_string(total_time, time_text, true);
    fprintf(h, "%s", time_text);
    for (int alignment = 0; alignment < (12 - strlen(time_text)); alignment++) {
        fprintf(h, " ");
    }
    fprintf(h, "%s\n", player_name);
}

void state::write_stats() {
    FILE* h = fopen("stats.txt", "w");
    if (!h) {
        external_error("Could not open STATS.TXT for writing!");
    }

    fprintf(h, "This text file is generated automatically each time you quit the\n");
    fprintf(h, "ELMA.EXE program. If you modify this file, you will loose the\n");
    fprintf(h, "changes next time you run the game. This is only an output file, the\n");
    fprintf(h, "best times are stored in the STATE.DAT binary file.\n");
    fprintf(h, "Registered version " ELMA_VERSION "\n");
    fprintf(h, "\n");

    // Singleplayer times:
    fprintf(h, "Single player times:\n");
    fprintf(h, "\n");
    for (int i = 0; i < INTERNAL_LEVEL_COUNT - 1; i++) {
        fprintf(h, "Level %d, %s:\n", i + 1, get_internal_level_name(i));
        topten* tten = &toptens[i].single;
        write_stats_topten(h, tten, true);
        fprintf(h, "\n");
    }
    fprintf(h, "\n");

    // Multiplayer times:
    fprintf(h, "Multiplayer times:\n");
    fprintf(h, "\n");
    for (int i = 0; i < INTERNAL_LEVEL_COUNT - 1; i++) {
        fprintf(h, "Level %d, %s:\n", i + 1, get_internal_level_name(i));
        topten* tten = &toptens[i].multi;
        write_stats_topten(h, tten, false);
        fprintf(h, "\n");
    }

    // Single player total times:
    fprintf(h, "The following are the single player total times for individual players.\n");
    fprintf(h, "If a player doesn't have a time in the top ten for a level, this\n");
    fprintf(h, "will add ten minutes to the total time.\n");

    for (int i = 0; i < player_count; i++) {
        write_stats_player_total_time(h, State->players[i].name, true);
    }
    fprintf(h, "\n");

    // Multi player total times:
    fprintf(h, "The following are the combined total times for individual players. For each\n");
    fprintf(h, "level the best time is choosen of either the player's single player best\n");
    fprintf(h, "time, or the best multiplayer time where the player was one of the two\n");
    fprintf(h, "players.\n");
    fprintf(h, "If a player doesn't have such a time for a level, this will add ten\n");
    fprintf(h, "minutes to the total time.\n");
    for (int i = 0; i < player_count; i++) {
        write_stats_player_total_time(h, State->players[i].name, false);
    }
    fprintf(h, "\n");

    // Anonymous total times:
    write_stats_anonymous_total_time(
        h, true, "The following is the anonymous total time of the best single player",
        "times. If there is no single player time for a level, this will",
        "add ten minutes to the total time.");
    write_stats_anonymous_total_time(
        h, false, "The following is the anonymous combined total time of the best",
        "single or multiplayer times. If there is no single or multiplayer",
        "time for a level, this will add ten minutes to the total time.");

    fclose(h);
}

void state::reset_keys() {
    keys1.gas = DIK_UP;
    keys1.brake = DIK_DOWN;
    keys1.right_volt = DIK_RIGHT;
    keys1.left_volt = DIK_LEFT;
    keys1.turn = DIK_SPACE;
    keys1.toggle_minimap = DIK_V;
    keys1.toggle_timer = DIK_T;
    keys1.toggle_visibility = DIK_1;
    keys1.alovolt = EolSettings->alovolt_key_player_a_default();
    keys1.brake_alias = EolSettings->brake_alias_key_player_a_default();

    keys2.gas = DIK_NUMPAD5;
    keys2.brake = DIK_NUMPAD2;
    keys2.right_volt = DIK_NUMPAD3;
    keys2.left_volt = DIK_NUMPAD1;
    keys2.turn = DIK_NUMPAD0;
    keys2.toggle_minimap = DIK_B;
    keys2.toggle_timer = DIK_Y;
    keys2.toggle_visibility = DIK_2;
    keys1.alovolt = EolSettings->alovolt_key_player_b_default();
    keys2.brake_alias = EolSettings->brake_alias_key_player_b_default();

    key_increase_screen_size = DIK_EQUALS;
    key_decrease_screen_size = DIK_MINUS;
    key_screenshot = DIK_I;
    key_escape_alias = EolSettings->escape_alias_key_default();
}

player* state::get_player(const char* player_name) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        player* cur_player = &State->players[i];
        if (strcmp(cur_player->name, player_name) == 0) {
            return cur_player;
        }
    }
    internal_error("get_player cannot find name!");
    return nullptr;
}
