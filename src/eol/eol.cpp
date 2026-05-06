#include "eol/eol.h"
#include "eol/status_messages.h"
#include "eol/console.h"
#include "eol_settings.h"
#include "log.h"
#include "platform/implementation.h"
#include "platform/utils.h"
#include <algorithm>
#include <cstring>
#include <chrono>
#include <format>

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
      players_online_table("Players online") {
    players_online_table.add_column(100, eol_table::Align::Left);
    players_online_table.add_column(100, eol_table::Align::Right);
}

void eol::process(const login& l) {
    if (l.success) {
        StatusMessages->add("login successful");
        id = l.id;
        id2 = l.id2;
    } else {
        StatusMessages->add("login unsuccessful");
    }
}

void eol::process(const new_kuski& nk) {
    auto pos = std::ranges::lower_bound(
        kuskis_, nk.k.nick, [](const char* a, const char* b) { return strcmpi(a, b) < 0; },
        [](const kuski& k) { return k.nick; });
    kuskis_.insert(pos, nk.k);
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

void eol::process(const battle_started& bs) {
    current_battle = bs.bat;

    if (current_battle->in_countdown) {
        StatusMessages->add("battle countdown started");
    } else {
        StatusMessages->add("battle running");
    }
}

void eol::process(const battle_countdown_ended&) {
    if (!current_battle) {
        LOG_ERROR("Received battle_countdown_ended message, but no battle is active");
        return;
    }

    current_battle->in_countdown = false;
    current_battle->local_start_ms = get_milliseconds();
    StatusMessages->add("battle running");
}

void eol::process(const battle_ended& be) {
    current_battle.reset();
    StatusMessages->add(be.aborted ? "battle aborted" : "battle over");
}

void eol::process(const battle_time_sync& bts) {
    if (!current_battle) {
        LOG_ERROR("Received battle_time_sync message, but no battle is active");
        return;
    }

    current_battle->local_start_ms = bts.local_start_ms;
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
        players_online_table.add_row({k->nick, k->level});
    }
}

void eol::process(const chat_message& msg) {
    std::string nick;
    if (msg.kuski_id == id) {
        nick = EolSettings->nick();
    } else {
        kuski* k = get_kuski(kuskis_, msg.kuski_id);
        if (!k) {
            return;
        }

        nick = k->nick;
    }

    auto timestamp = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::from_time_t(static_cast<std::time_t>(msg.unix_timestamp)));
    std::string line = std::format("{:%H:%M:%S} <{}> {}", timestamp, nick, msg.message);
    Console->add_line(line, console::LineType::Chat);
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

void eol::enter_level(const char* level_name, const level* lev) {
    for (kuski& k : kuskis_) {
        k.clear_spy_data();
    }

    struct enter_level el{.lev = lev, .name = level_name};
    proto.send(el);
}

void eol::exit_level(const char* level_name, double time, int apple_count, int level_apple_count,
                     bool dead) {
    struct exit_level fl{.name = level_name,
                         .time = time,
                         .apple_count = apple_count,
                         .level_apple_count = level_apple_count,
                         .dead = dead};
    proto.send(fl);
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

const struct spy_data* kuski::spy_data() const { return data ? &*data : nullptr; }

void kuski::add_spy_data(const struct spy_data& sd) { data = sd; }

void kuski::clear_spy_data() { data = std::nullopt; }
