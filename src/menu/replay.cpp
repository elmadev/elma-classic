#include "menu/replay.h"

#include "best_times.h"
#include "EDITUJ.H"
#include "eol_settings.h"
#include "fs_utils.h"
#include "menu/rec_list.h"
#include "LEJATSZO.H"
#include "level.h"
#include "level_load.h"
#include "menu_dialog.h"
#include "menu_nav.h"
#include "menu_pic.h"
#include "menu_play.h"
#include "recorder.h"
#include "platform_impl.h"
#include "util/util.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <format>
#include <string>
#include <vector>

enum class LoadReplayResult { Success, Fail, Abort };

static void replay_time(const std::string& filename) {
    MenuPalette->set();
    loading_screen();

    recorder::load_rec_file(filename.c_str(), false);

    int time = Rec1->frame_count();
    if (MultiplayerRec && Rec2->frame_count() > time) {
        time = Rec2->frame_count();
    }

    time = (int)(time * 3.3333333333333);
    time -= 2;
    time = std::max(time, 1);

    char time_str[25];
    util::text::centiseconds_to_string(time, time_str);
    strcat(time_str, "    +- 0.01 sec");
    menu_dialog(filename.c_str(), "The time of this replay file is:", time_str);
}

static LoadReplayResult validate_replay_level(int level_id, const std::string& filename) {
    if (access_level_file(Rec1->level_filename) != 0) {
        DikScancode key =
            menu_dialog("Cannot find the lev file that corresponds", "to the record file!",
                        filename.c_str(), Rec1->level_filename);
        return key == DIK_ESCAPE ? LoadReplayResult::Abort : LoadReplayResult::Fail;
    }
    load_level_play(Rec1->level_filename);

    if (Ptop->level_id != level_id) {
        DikScancode key =
            menu_dialog("The level file has changed since the", "saving of the record file!",
                        filename.c_str(), Rec1->level_filename);
        return key == DIK_ESCAPE ? LoadReplayResult::Abort : LoadReplayResult::Fail;
    }

    return LoadReplayResult::Success;
}

static LoadReplayResult load_replay(const std::string& filename) {
    MenuPalette->set();
    loading_screen();

    int level_id = recorder::load_rec_file(filename.c_str(), false);
    return validate_replay_level(level_id, filename);
}

static void merge_play(const std::string& file1, const std::string& file2) {
    MenuPalette->set();
    loading_screen();

    recorder::merge_result result = recorder::load_merge(file1, file2);

    if (result.rec1_was_multi || result.rec2_was_multi) {
        menu_dialog("Note: Only player 1 used from multiplayer replays.");
    }

    if (result.level_id_mismatch) {
        DikScancode key = menu_dialog("Warning: Replays are from different levels!",
                                      "Press Enter to continue, ESC to cancel.");
        if (key == DIK_ESCAPE) {
            return;
        }
    }

    LoadReplayResult loaded = validate_replay_level(result.level_id, file1);
    if (loaded != LoadReplayResult::Success) {
        return;
    }

    replay_from_file(Rec1->level_filename);
}

static void replay_play(const std::string& filename) {
    if (load_replay(filename) == LoadReplayResult::Success) {
        replay_from_file(Rec1->level_filename);
    }
}

static void replay_render(const std::string& filename) {
    setup_render_directory(filename);
    std::string msg = std::format("Recording at {} FPS to {}", EolSettings->recording_fps(),
                                  VideoOutputDirectory);
    DikScancode c = menu_dialog("Render replay to video frames?", msg.c_str(),
                                "Press Enter to continue, ESC to cancel");
    if (c == DIK_RETURN) {
        if (load_replay(filename) == LoadReplayResult::Success) {
            Rec1->rewind();
            Rec2->rewind();
            render_replay(Rec1->level_filename);
        }
    }
}

static void replay_randomizer(std::vector<std::string>& filenames) {
    int count = static_cast<int>(filenames.size());
    int last_played = -1;
    int second_last_played = -1;
    while (true) {
        int index = util::random::uint32() % count;
        while ((index == last_played && count > 1) || (index == second_last_played && count > 2)) {
            index = util::random::uint32() % count;
        }
        second_last_played = last_played;
        last_played = index;

        LoadReplayResult loaded = load_replay(filenames[index]);
        if (loaded == LoadReplayResult::Success) {
            Rec1->rewind();
            Rec2->rewind();
            if (lejatszo_r(Rec1->level_filename, 0)) {
                return;
            }
        } else if (loaded == LoadReplayResult::Abort) {
            return;
        }
    }
}

static void replay_row_handler(const std::string& filename) {
    if (is_key_down(DIK_F1)) {
        replay_render(filename);
    } else if (is_key_down(DIK_LCONTROL) && is_key_down(DIK_LMENU)) {
        replay_time(filename);
    } else {
        replay_play(filename);
    }
}

void menu_replay_all() {
    std::vector<std::string> replay_names = rec_list::get_replays();

    menu_nav nav("Select replay file!");
    nav.add_row("Randomizer", NAV_FUNC(&replay_names) { replay_randomizer(replay_names); });

    for (const std::string& filename : replay_names) {
        constexpr int EXT_LEN = 4;
        std::string short_name = filename.substr(0, filename.size() - EXT_LEN);
        nav.add_row(short_name, NAV_FUNC(filename) { replay_row_handler(filename); });
    }

    nav.search_pattern = SearchPattern::Sorted;
    nav.search_skip = 1;
    nav.max_search_len = MAX_REPLAY_NAME_LEN;
    nav.sort_rows();

    if (nav.row_count() <= 1) {
        return;
    }

    while (true) {
        MenuPalette->set();
        int choice = nav.navigate();
        if (choice < 0) {
            return;
        }
    }
}

void menu_merge_replays() {
    std::string picked_file;
    std::vector<std::string> replay_names = rec_list::get_replays();

    menu_nav nav("Select first replay");

    for (const std::string& filename : replay_names) {
        constexpr int EXT_LEN = 4;
        std::string short_name = filename.substr(0, filename.size() - EXT_LEN);
        nav.add_row(short_name, NAV_FUNC(&picked_file, filename) { picked_file = filename; });
    }

    nav.search_pattern = SearchPattern::Sorted;
    nav.max_search_len = MAX_REPLAY_NAME_LEN;
    nav.sort_rows();

    if (nav.row_count() == 0) {
        return;
    }

    while (true) {
        nav.title = "Select first replay";
        MenuPalette->set();
        if (nav.navigate() < 0) {
            return;
        }
        std::string file1 = picked_file;

        nav.title = "Select second replay";
        MenuPalette->set();
        if (nav.navigate() < 0) {
            continue;
        }

        merge_play(file1, picked_file);
    }
}

void menu_replay_level(int level_id) {
    std::vector<std::string> replay_names = rec_list::replays_for_level(level_id);
    std::erase(replay_names, std::string(LAST_REC_FILENAME));

    if (replay_names.empty()) {
        return;
    }

    menu_nav nav("Level Replays");

    for (const std::string& filename : replay_names) {
        constexpr int EXT_LEN = 4;
        std::string short_name = filename.substr(0, filename.size() - EXT_LEN);
        nav.add_row(short_name, NAV_FUNC(filename) { replay_row_handler(filename); });
    }

    nav.search_pattern = SearchPattern::Sorted;
    nav.max_search_len = MAX_REPLAY_NAME_LEN;
    nav.sort_rows();

    while (true) {
        MenuPalette->set();
        int choice = nav.navigate();
        if (choice < 0) {
            return;
        }
    }
}
