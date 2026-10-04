#include "menu/replay.h"
#include "editor/editor.h"
#include "eol/settings.h"
#include "game/game.h"
#include "game/ghost_list.h"
#include "game/level_load.h"
#include "game/recorder.h"
#include "level/level.h"
#include "main.h"
#include "menu/best_times.h"
#include "menu/dialog.h"
#include "menu/nav.h"
#include "menu/pic.h"
#include "menu/play.h"
#include "menu/rec_list.h"
#include "platform/implementation.h"
#include "util/util.h"
#include <algorithm>
#include <cstring>
#include <format>
#include <numeric>
#include <string>
#include <vector>

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

static bool validate_replay_level(int level_id, const std::string& filename) {
    if (!level_file_exists(Rec1->level_filename)) {
        menu_dialog("Cannot find the lev file that corresponds", "to the record file!",
                    filename.c_str(), Rec1->level_filename);
        return false;
    }
    if (!load_level_play(Rec1->level_filename)) {
        return false;
    }

    if (Level->level_id != level_id) {
        menu_dialog("The level file has changed since the", "saving of the record file!",
                    filename.c_str(), Rec1->level_filename);
        return false;
    }

    return true;
}

static bool load_replay(const std::string& filename) {
    MenuPalette->set();
    loading_screen();

    int level_id = recorder::load_rec_file(filename.c_str(), false);
    return validate_replay_level(level_id, filename);
}

// Selected replays are marked left of the names, clear of the helmet cursor
constexpr int MARK_X = 110;
constexpr const char MARK[] = "X";

static bool shift_held() { return is_key_down(DIK_LSHIFT) || is_key_down(DIK_RSHIFT); }

static bool any_marked(menu_nav& nav) {
    for (size_t i = 0; i < nav.row_count(); i++) {
        if (!nav.entry_right((int)i).empty()) {
            return true;
        }
    }
    return false;
}

static void clear_marks(menu_nav& nav) {
    for (size_t i = 0; i < nav.row_count(); i++) {
        nav.entry_right((int)i).clear();
    }
}

// Play `focused` with every marked row drawn as a ghost. `skip_row` is the
// followed replay's own row, or -1.
static void ghost_play(menu_nav& nav, const std::string& focused, int skip_row) {
    MenuPalette->set();
    loading_screen();

    int level_id = recorder::load_rec_file(focused.c_str(), false);

    for (size_t i = 0; i < nav.row_count(); i++) {
        if ((int)i == skip_row || nav.entry_right((int)i).empty()) {
            continue;
        }
        std::string filename = nav.entry_left((int)i) + ".rec";
        if (!Ghosts.add("rec/" + filename, level_id)) {
            menu_dialog(filename.c_str(), "is not a replay of this level!");
            return;
        }
    }

    if (!validate_replay_level(level_id, focused)) {
        return;
    }

    replay_from_file(Rec1->level_filename);
}

static void replay_play(const std::string& filename) {
    if (load_replay(filename)) {
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
        if (load_replay(filename)) {
            Rec1->rewind();
            Rec2->rewind();
            render_replay(Rec1->level_filename);
        }
    }
}

static void replay_randomizer(std::vector<std::string>& filenames) {
    int count = static_cast<int>(filenames.size());
    std::vector<int> indices(count);
    std::iota(indices.begin(), indices.end(), 0);
    int last_played = -1;
    int second_last_played = -1;
    while (!indices.empty()) {
        int index = util::random::uint32() % count;
        while ((index == last_played && count > 1) || (index == second_last_played && count > 2)) {
            index = util::random::uint32() % count;
        }
        second_last_played = last_played;
        last_played = index;

        bool loaded = load_replay(filenames[indices[index]]);
        if (loaded) {
            Rec1->rewind();
            Rec2->rewind();
            if (replay_loop(Rec1->level_filename, false)) {
                return;
            }
        } else {
            indices.erase(indices.begin() + index);
            count = static_cast<int>(indices.size());
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

static void add_replay_rows(menu_nav& nav, const std::vector<std::string>& replay_names,
                            const nav_func& handler) {
    for (const std::string& filename : replay_names) {
        constexpr int EXT_LEN = 4;
        nav.add_row(filename.substr(0, filename.size() - EXT_LEN), handler);
    }

    nav.search_pattern = SearchPattern::Sorted;
    nav.max_search_len = MAX_REPLAY_NAME_LEN;
    nav.sort_rows();
    nav.x_right = MARK_X;
}

// The first replay picked is followed. Shift+Enter toggles a ghost, Enter adds
// one and plays, ESC starts over and leaves once nothing is picked.
void menu_merge_replays() {
    menu_nav nav("Merge replays");
    int followed = -1;

    add_replay_rows(nav, rec_list::get_replays(),
                    [&nav, &followed](int choice, const std::string&, const std::string&) {
                        if (followed < 0) {
                            followed = choice;
                            nav.entry_right(choice) = MARK;
                            return;
                        }
                        if (choice == followed) {
                            return;
                        }
                        std::string& mark = nav.entry_right(choice);
                        if (shift_held()) {
                            mark = mark.empty() ? MARK : "";
                            return;
                        }
                        mark = MARK;
                        ghost_play(nav, nav.entry_left(followed) + ".rec", followed);
                    });

    if (nav.row_count() == 0) {
        return;
    }

    while (true) {
        MenuPalette->set();
        if (nav.navigate() >= 0) {
            continue;
        }
        if (followed < 0) {
            Ghosts.clear();
            return;
        }
        followed = -1;
        clear_marks(nav);
    }
}

// Returns false if the user cancelled with ESC before it finished,
// in which case the cache is still building and callers must not touch it.
static bool wait_for_cache() {
    if (!rec_list::is_cache_ready()) {
        loading_screen();
        while (!rec_list::is_cache_ready()) {
            handle_events();
            if (is_key_down(DIK_ESCAPE)) {
                return false;
            }
        }
    }
    return true;
}

void menu_replay_level(int level_id) {
    if (!wait_for_cache()) {
        return;
    }

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

// Like menu_merge_replays(), but `merge_file` is followed
void menu_merge_level(int level_id, const std::string& merge_file) {
    if (!wait_for_cache()) {
        return;
    }

    std::vector<std::string> replay_names = rec_list::replays_for_level(level_id);
    std::erase(replay_names, std::string(LAST_REC_FILENAME));

    if (replay_names.empty()) {
        return;
    }

    menu_nav nav("Merge with");

    add_replay_rows(nav, replay_names,
                    [&nav, &merge_file](int choice, const std::string&, const std::string&) {
                        std::string& mark = nav.entry_right(choice);
                        if (shift_held()) {
                            mark = mark.empty() ? MARK : "";
                            return;
                        }
                        mark = MARK;
                        ghost_play(nav, merge_file, -1);
                    });

    while (true) {
        MenuPalette->set();
        if (nav.navigate() >= 0) {
            continue;
        }
        if (!any_marked(nav)) {
            Ghosts.clear();
            return;
        }
        clear_marks(nav);
    }
}
