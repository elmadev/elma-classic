#include "editor/editor.h"
#include "eol/settings.h"
#include "game/state.h"
#include "level/level.h"
#include "log.h"
#include "main.h"
#include "menu/play.h"
#include <cstring>

constexpr const char LEVEL_NAME[] = "test.lev";

topten_set null_set;

struct top_ten_entry {
    int time;
    player_name name1;
    player_name name2;
    std::string message;
};

void update_and_test(const top_ten_entry& entry,
                     const std::span<const top_ten_entry* const>& result) {
    // Update top ten
    LOG_INFO("----------");
    LOG_INFO("{}/{} {}", entry.name1, entry.name2, entry.message);
    strcpy(State->player1, entry.name1);
    strcpy(State->player2, entry.name2);
    char msg[256];
    update_top_ten(entry.time, msg, 0, LEVEL_NAME);

    // Make sure time message matches expected
    LOG_INFO("{} == {}", msg, entry.message);
    ELMA_ASSERT(strcmp(msg, entry.message.c_str()) == 0);

    // Make sure top ten table matches expected result
    ELMA_ASSERT((int)result.size() == Level->toptens.single.times_count);
    for (std::size_t i = 0; i < result.size(); i++) {
        const auto& r = *result[i];
        LOG_INFO("{}/{} {} == {}/{} {}", Level->toptens.single.names1[i],
                 Level->toptens.single.names2[i], Level->toptens.single.times[i], r.name1, r.name2,
                 r.time);
        ELMA_ASSERT(strcmp(Level->toptens.single.names1[i], r.name1) == 0);
        ELMA_ASSERT(strcmp(Level->toptens.single.names2[i], r.name2) == 0);
        ELMA_ASSERT(Level->toptens.single.times[i] == r.time);
    }

    // Quick check for overflow in the next struct
    ELMA_ASSERT(std::memcmp(&Level->toptens.multi, &null_set, sizeof(Level->toptens.multi)) == 0);
}

// Test the sorting behaviour
// Test the text messages
void test_menu_play_update_top_ten() {
    std::filesystem::create_directory("lev");
    EolSettings = new eol_settings();
    State = new state;
    Level = new level();
    Level->save(LEVEL_NAME, true);
    delete Level;
    Level = new level(LEVEL_NAME);

    top_ten_entry a{100, "a1", "a2", "00:01:00     Best Time!"};
    top_ten_entry b{50, "b1", "b2", "00:00:50     Best Time!"};
    top_ten_entry c{1000000000, "c1", "c2", "59:59:99"};
    top_ten_entry d{100, "d1", "d2", "00:01:00     You Made the Top Ten"};
    top_ten_entry e{-1, "e1", "e2", "You Failed to Finish!"};
    top_ten_entry f{75, "f1", "f2", "00:00:75     You Made the Top Ten"};
    top_ten_entry g{75, "g1", "g2", "00:00:75     You Made the Top Ten"};
    top_ten_entry h{75, "h1", "h2", "00:00:75     You Made the Top Ten"};
    top_ten_entry i{75, "i1", "i2", "00:00:75     You Made the Top Ten"};
    top_ten_entry j{75, "j1", "j2", "00:00:75     You Made the Top Ten"};
    top_ten_entry k{75, "k1", "k2", "00:00:75     You Made the Top Ten"};
    top_ten_entry l{75, "l1", "l2", "00:00:75     You Made the Top Ten"};
    top_ten_entry m{750, "m1", "m2", "00:07:50"};
    top_ten_entry n{100, "n1", "n2", "00:01:00"};
    top_ten_entry o{1, "o1", "o2", "00:00:01     Best Time!"};

    update_and_test(a, std::array{&a});
    update_and_test(b, std::array{&b, &a});
    update_and_test(c, std::array{&b, &a, &c});
    update_and_test(d, std::array{&b, &a, &d, &c});
    update_and_test(e, std::array{&b, &a, &d, &c});
    update_and_test(f, std::array{&b, &f, &a, &d, &c});
    update_and_test(g, std::array{&b, &f, &g, &a, &d, &c});
    update_and_test(h, std::array{&b, &f, &g, &h, &a, &d, &c});
    update_and_test(i, std::array{&b, &f, &g, &h, &i, &a, &d, &c});
    update_and_test(j, std::array{&b, &f, &g, &h, &i, &j, &a, &d, &c});
    update_and_test(k, std::array{&b, &f, &g, &h, &i, &j, &k, &a, &d, &c});
    update_and_test(l, std::array{&b, &f, &g, &h, &i, &j, &k, &l, &a, &d});
    update_and_test(m, std::array{&b, &f, &g, &h, &i, &j, &k, &l, &a, &d});
    update_and_test(n, std::array{&b, &f, &g, &h, &i, &j, &k, &l, &a, &d});
    update_and_test(o, std::array{&o, &b, &f, &g, &h, &i, &j, &k, &l, &a});
}

// Test the numbering scheme used in replay autosave
// https://stackoverflow.com/questions/181596/how-to-convert-a-column-number-e-g-127-into-an-excel-column-e-g-aa/182924
extern std::string format_rec_name(const std::string& level_name, char player, int time, int i);
void test_menu_play_format_rec_name() {
    // 3-5 digits if < 10 minutes
    // test numbering scheme "", "b", "c", ..., "z", "aa", "ab", ...
    // truncate the digits if we run out of space due to numbering scheme being >= "aa"
    ELMA_ASSERT(format_rec_name("LEVELNAM", '_', 5, 1) == "LEVELNAM_005.rec");
    ELMA_ASSERT(format_rec_name("LEVELNAM", '_', 500, 2) == "LEVELNAM_500b.rec");
    ELMA_ASSERT(format_rec_name("LEVELNAM", '_', 5500, 26) == "LEVELNAM_5500z.rec");
    ELMA_ASSERT(format_rec_name("LEVELNAM", '_', 5500, 27) == "LEVELNAM_5500aa.rec");
    ELMA_ASSERT(format_rec_name("LEVELNAM", '_', 5500, 28) == "LEVELNAM_5500ab.rec");
    ELMA_ASSERT(format_rec_name("LEVELNAM", '_', 5500, 703) == "LEVELNAM_550aaa.rec");
    ELMA_ASSERT(format_rec_name("LEVELNAM", '_', 6000 + 1500, 1) == "LEVELNAM_11500.rec");
    ELMA_ASSERT(format_rec_name("LEVELNAM", '_', 6000 + 1500, 2) == "LEVELNAM_11500b.rec");
    ELMA_ASSERT(format_rec_name("LEVELNAM", '_', 6000 + 1500, 27) == "LEVELNAM_1150aa.rec");
    // Remove centiseconds if >= 10 minutes
    ELMA_ASSERT(format_rec_name("LEVELNAM", '_', 10 * 6000, 1) == "LEVELNAM_1000.rec");
    ELMA_ASSERT(format_rec_name("LEVELNAM", '_', 10 * 6000 + 3500, 1) == "LEVELNAM_1035.rec");
    ELMA_ASSERT(format_rec_name("LEVELNAM", '_', 61 * 6000, 1) == "LEVELNAM_10100.rec");
}
