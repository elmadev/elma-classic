#include "eol/eol.h"
#include "eol/status_messages.h"
#include "eol/console.h"
#include "abc8.h"
#include "eol_settings.h"
#include "level.h"
#include "log.h"
#include "pic8.h"
#include "platform/implementation.h"
#include "platform/utils.h"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <format>
#include <string>
#include <string_view>

static kuski* get_kuski(std::vector<kuski>& kuskis, unsigned int id) {
    for (kuski& k : kuskis) {
        if (k.id == id) {
            return &k;
        }
    }

    return nullptr;
}

static std::string format_level(std::string_view level) {
    std::string with_ext = std::format("{}.lev", level);
    auto idx = get_internal_index(with_ext.c_str());

    if (idx.has_value()) {
        return std::format("internal {:02}", *idx);
    }
    return with_ext;
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
        players_online_table.add_row({k->nick, strlen(k->level) > 0 ? format_level(k->level) : ""});
    }
}

std::string_view eol::lookup_nick(unsigned int kuski_id) const {
    auto match = std::ranges::find(all_kuskis(), kuski_id, &kuski::id);
    return match != all_kuskis().end() ? std::string_view{match->nick} : "?";
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

static std::string format_hms(int seconds) {
    seconds = std::max(seconds, 0);
    int hours = seconds / 3600;
    int mins = (seconds / 60) % 60;
    int secs = seconds % 60;

    if (hours > 0) {
        return std::format("{}:{:02}:{:02}", hours, mins, secs);
    }

    return std::format("{}:{:02}", mins, secs);
}

static std::string_view battle_type_prefix(BattleType t) {
    using enum BattleType;

    switch (t) {
    case Normal:
        return "";
    case OneLife:
        return "one-life ";
    case FirstFinish:
        return "first finish ";
    case Slowness:
        return "slowness ";
    case Survivor:
        return "survivor ";
    case LastCounts:
        return "last counts ";
    case FinishCount:
        return "finish-count ";
    case HourTT:
        return "1 hour TT ";
    case FlagTag:
        return "flag tag ";
    case AppleCollect:
        return "apple ";
    case Speed:
        return "speed ";
    }
    return "";
}

static std::string format_type_with_cripples(BattleType t, BattleAttributes::Kind attrs) {
    using namespace BattleAttributes;

    constexpr std::pair<Kind, std::string_view> cripples[] = {
        {NoVolt, "no-volt "},
        {NoTurn, "no-turn "},
        {OneTurn, "one-turn "},
        {NoBrake, "no-brake "},
        {NoThrottle, "no-throttle "},
        {AlwaysThrottle, "always-throttle "},
        {OneWheel, "one-wheel "},
        {Drunk, "drunk "},
        {Multi, "multi "},
    };

    std::string out{battle_type_prefix(t)};

    for (auto [flag, text] : cripples) {
        if (attrs & flag) {
            out += text;
        }
    }

    out += "battle";

    out.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(out.front())));

    return out;
}

void eol::render_battle_status(pic8& dest, abc8& font) const {
    if (!EolSettings->show_battle_status() || !current_battle) {
        return;
    }

    const std::string type_text =
        format_type_with_cripples(current_battle->type, current_battle->attributes);
    const std::string level_text = format_level(current_battle->level_filename);
    const std::string duration_text = format_hms(current_battle->duration * 60);
    const std::string_view designer = lookup_nick(current_battle->designer_id);

    const long long target_ms =
        current_battle->in_countdown
            ? current_battle->local_start_ms
            : current_battle->local_start_ms + current_battle->duration * 60000LL;
    const long long left_ms = target_ms - get_milliseconds();
    const std::string time_text = format_hms(static_cast<int>((left_ms + 999) / 1000));

    std::string out;
    if (current_battle->in_countdown && current_battle->type == BattleType::FlagTag) {
        out = std::format("{} in {} by {} - flag will be given in {} ({})", type_text, level_text,
                          designer, time_text, duration_text);
    } else if (current_battle->in_countdown) {
        out = std::format("{} in {} by {} starts in {} ({})", type_text, level_text, designer,
                          time_text, duration_text);
    } else if (current_battle->type == BattleType::HourTT) {
        out = std::format("{} by {} ends in {}", type_text, designer, time_text);
    } else {
        out = std::format("{} in {} by {} ends in {} / {}", type_text, level_text, designer,
                          time_text, duration_text);
    }

    const int y = 15 + font.line_height() * (1 + EolSettings->chat_lines());
    font.write_centered(&dest, dest.get_width() / 2, y, out.c_str());
}
