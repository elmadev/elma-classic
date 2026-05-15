#include "eol/eol.h"
#include "eol/status_messages.h"
#include "eol/console.h"
#include "eol_settings.h"
#include "level.h"
#include "menu/external.h"
#include "pic8.h"
#include "platform/implementation.h"
#include "platform/utils.h"
#include "util/util.h"
#include <algorithm>
#include <cstring>
#include <ctime>
#include <format>
#include <fstream>
#include <string>
#include <string_view>

static tm local_tm(uint64_t unix_timestamp) {
    time_t t = static_cast<time_t>(unix_timestamp);
    tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    return tm;
}

static kuski* get_kuski(std::vector<kuski>& kuskis, unsigned int id) {
    for (kuski& k : kuskis) {
        if (k.id == id) {
            return &k;
        }
    }

    return nullptr;
}

eol::eol()
    : proto(*this),
      cur_table(nullptr),
      players_online_table("Players online"),
      battle_results_table("Battle results"),
      battle_queue_table("Battle queue") {
    players_online_table.add_column(100, eol_table::Align::Left);
    players_online_table.add_column(100, eol_table::Align::Right);
    battle_results_table.add_column(100, eol_table::Align::Left);
    battle_results_table.add_column(100, eol_table::Align::Right);
    battle_queue_table.add_column(100, eol_table::Align::Left);
    battle_queue_table.add_column(60, eol_table::Align::Right);
    battle_queue_table.add_column(130, eol_table::Align::Right);
}

void eol::process(const login& l) {
    if (l.success) {
        if (id != l.id || id2 != l.id2) {
            StatusMessages->add("login successful");
            id = l.id;
            id2 = l.id2;

            kuski self{};
            self.id = id;
            strncpy(self.nick, EolSettings->nick().c_str(), sizeof(self.nick) - 1);
            self.is_player = true;
            self.is_online = true;
            process(new_kuski{self});

            if (id2 != 0 && id2 != id) {
                self.id = id2;
                self.is_player = false;
                process(new_kuski{self});
            }
        }
    } else {
        StatusMessages->add("login unsuccessful");
    }
}

pic8* eol::load_shirt(std::string_view nick) {
    constexpr int SHIRT_BMP_WIDTH = 149;
    constexpr int SHIRT_BMP_HEIGHT = 101;

    char path[4 + sizeof(kuski::nick) + 4 + 1] = {};
    std::format_to_n(path, sizeof(path) - 1, "bmp/{}.bmp", nick);

    pic8* pic_shirt = pic8::from_bmp(path);
    if (pic_shirt && pic_shirt->get_width() == SHIRT_BMP_WIDTH &&
        pic_shirt->get_height() == SHIRT_BMP_HEIGHT) {
        return pic_shirt;
    }

    return nullptr;
}

void eol::process(const new_kuski& nk) {
    auto pos = std::ranges::lower_bound(
        kuskis_, nk.k.nick, [](const char* a, const char* b) { return strcmpi(a, b) < 0; },
        [](const kuski& k) { return k.nick; });
    kuski k = nk.k;
    if (k.is_player && k.is_online) {
        k.shirt = load_shirt(k.nick);
    }
    kuskis_.insert(pos, k);
    sync_players_online_table();
}

void eol::process(const kuski_logout& kl) {
    for (kuski& k : kuskis_) {
        if (k.id == kl.id || k.id == kl.id2) {
            k.is_online = false;
        }
    }
    sync_players_online_table();
}

void eol::process(const kuski_set_level& l) {
    kuski* k = get_kuski(kuskis_, l.id);
    if (!k) {
        return;
    }

    strncpy(k->level, (const char*)l.level, MAX_FILENAME_LEN);
    sync_players_online_table();
}

void eol::sync_players_online_table() {
    players_online_table.clear_rows();
    std::vector<const kuski*> by_id;
    by_id.reserve(kuskis_.size());
    for (const kuski& k : kuskis()) {
        by_id.push_back(&k);
    }
    // "players online" table is ordered by login time, so sort by id
    std::ranges::sort(by_id, {}, &kuski::id);
    for (const kuski* k : by_id) {
        players_online_table.add_row({k->nick, strlen(k->level) > 0 ? format_level(k->level) : ""});
    }
}

std::string_view eol::lookup_nick(unsigned int kuski_id) const {
    auto match = std::ranges::find(all_kuskis(), kuski_id, &kuski::id);
    return match != all_kuskis().end() ? std::string_view{match->nick} : "?";
}

void eol::process(const chat_message& msg) {
    std::string_view nick = lookup_nick(msg.kuski_id);
    tm tm = local_tm(msg.unix_timestamp);
    std::string line = std::format("{:02}:{:02}:{:02} <{}> {}", tm.tm_hour, tm.tm_min, tm.tm_sec,
                                   nick, (const char*)msg.message);
    Console->add_line(line, console::LineType::Chat);
}

void eol::process(const private_message& msg) {
    std::string_view from_nick = lookup_nick(msg.from_kuski_id);
    std::string_view to_nick = lookup_nick(msg.to_kuski_id);
    tm tm = local_tm(msg.unix_timestamp);
    std::string line = std::format("{:02}:{:02}:{:02} <{}>-><{}> {}", tm.tm_hour, tm.tm_min,
                                   tm.tm_sec, from_nick, to_nick, (const char*)msg.message);
    Console->add_line(line, console::LineType::Pm);
}

void eol::process(const team_message& msg) {
    std::string_view nick = lookup_nick(msg.from_kuski_id);
    tm tm = local_tm(msg.unix_timestamp);
    std::string line = std::format("{:02}:{:02}:{:02} [Team] <{}> {}", tm.tm_hour, tm.tm_min,
                                   tm.tm_sec, nick, (const char*)msg.message);
    Console->add_line(line, console::LineType::Team);
}

void eol::process(const spy_data& sd) {
    kuski* k = get_kuski(kuskis_, sd.kuski_id);
    if (k) {
        k->add_spy_data(sd);
    }
}

void eol::process(const clear_spy_data& sd) {
    kuski* k = get_kuski(kuskis_, sd.kuski_id);
    if (k) {
        k->clear_spy_data();
    }
}

void eol::process(const level_download& ld) {
    const char* level_name = (const char*)ld.level;
    switch (ld.result) {
    case DownloadResult::Success: {
        for (char c : ld.level) {
            if (c && !util::text::is_filename_char(c)) {
                StatusMessages->add(std::format("error: level {}.lev failed download", level_name));
                return;
            }
        }
        std::string path = std::format("lev/{}.lev", level_name);
        std::ofstream file(path, std::ios::binary);
        file.write((const char*)ld.data.data(), ld.data.size());
        invalidate_external_levels();
        StatusMessages->add(std::format("level {}.lev downloaded", level_name));
        break;
    }
    case DownloadResult::Fail:
        StatusMessages->add(std::format("error: level {}.lev failed download", level_name));
        break;
    case DownloadResult::NotFound:
        StatusMessages->add(std::format("level {}.lev not saved in the database", level_name));
        break;
    }
}

void eol::download_level(std::string_view name) {
    level_download_request req{};
    int size = std::min(name.size(), sizeof(req.level) - 1);
    strncpy((char*)req.level, name.data(), size);
    proto.send(req);
}

void eol::download_battle_level() {
    if (!current_battle) {
        return;
    }
    // Internal battles, 1H TT, etc don't have level files.
    if (!(current_battle->attributes & BattleAttributes::Uploaded)) {
        return;
    }

    proto.send(battle_level_download_request{});
}

void eol::enter_level(const char* level_name, const level* lev) {
    for (kuski& k : kuskis_) {
        k.clear_spy_data();
    }

    struct enter_level el{.lev = lev, .name = level_name};
    proto.send(el);
}

void eol::exit_level(const char* level_name, double time, int apple_count, int level_apple_count,
                     bool dead) {
    spy_kuski_id.reset();
    struct exit_level fl{.name = level_name,
                         .time = time,
                         .apple_count = apple_count,
                         .level_apple_count = level_apple_count,
                         .dead = dead};
    proto.send(fl);
}

void eol::send_chat(std::string_view message) {
    constexpr size_t MAX_LEN = MAX_MESSAGE_LEN;
    constexpr size_t MAX_INPUT_LEN = MAX_LEN * 3;
    constexpr size_t SPLIT_MARGIN = 20;
    if (message.empty() || message.size() > MAX_INPUT_LEN) {
        return;
    }
    while (message.size() > MAX_LEN) {
        size_t split = message.rfind(' ', MAX_LEN);
        if (split == std::string_view::npos || split < MAX_LEN - SPLIT_MARGIN) {
            split = MAX_LEN;
        }
        struct send_chat sc{.kuski_id = id, .message = message.substr(0, split)};
        proto.send(sc);
        message.remove_prefix(split);
        if (!message.empty() && message.front() == ' ') {
            message.remove_prefix(1);
        }
    }
    struct send_chat sc{.kuski_id = id, .message = message};
    proto.send(sc);
}

void eol::set_table(TableType table) {
    eol_table* new_table = nullptr;
    switch (table) {
    case TableType::None:
        cur_table = nullptr;
        break;
    case TableType::PlayersOnline:
        new_table = &players_online_table;
        break;
    case TableType::BattleResults:
        new_table = &battle_results_table;
        break;
    case TableType::BattleQueue:
        new_table = &battle_queue_table;
        break;
    }

    if (cur_table != new_table) {
        cur_table = new_table;
    } else {
        cur_table = nullptr;
        table = TableType::None;
    }

    proto.send(show_table{.table = table});
}

void eol::render_table(pic8& dest, abc8& title_font, abc8& data_font) const {
    if (!cur_table) {
        return;
    }

    cur_table->render(dest, title_font, data_font, eol_table::Align::Center);
}

const kuski* eol::spy_kuski() {
    if (!spy_kuski_id) {
        return nullptr;
    }

    const kuski* k = get_kuski(kuskis_, *spy_kuski_id);
    return k->spy_data() ? k : nullptr;
}

template <typename Range>
static void set_spy_kuski(std::optional<unsigned int>& spy_kuski_id, Range&& range) {
    bool found_current = !spy_kuski_id;

    for (const kuski& k : range) {
        if (spy_kuski_id && *spy_kuski_id == k.id) {
            found_current = true;
            continue;
        }

        if (!k.spy_data()) {
            continue;
        }

        if (found_current) {
            spy_kuski_id = k.id;
            StatusMessages->add(std::format("now observing {}", k.nick));
            return;
        }
    }

    if (spy_kuski_id) {
        spy_kuski_id.reset();
        StatusMessages->add("not observing anyone anymore");
    }
}

void eol::spy_next_kuski() { set_spy_kuski(spy_kuski_id, kuskis()); }

void eol::spy_prev_kuski() { set_spy_kuski(spy_kuski_id, std::views::reverse(kuskis())); }

const struct spy_data* kuski::spy_data() const { return data ? &*data : nullptr; }

void kuski::add_spy_data(const struct spy_data& sd) { data = sd; }

void kuski::clear_spy_data() { data.reset(); }
