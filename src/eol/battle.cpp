#include "eol/eol.h"
#include "eol/status_messages.h"
#include "abc8.h"
#include "level.h"
#include "log.h"
#include "pic8.h"
#include "util/util.h"
#include <algorithm>
#include <format>

static std::string format_battle_result(BattleType type, uint32_t time, uint16_t apples) {
    using enum BattleType;
    if (type == FlagTag && time == 0 && apples == 0) {
        return "0:00";
    }
    if (type == Speed) {
        return std::format("{}.{:02}", time / 100, time % 100);
    }
    if (type == FinishCount) {
        return std::format("{} finish{}", time, time == 1 ? "" : "es");
    }
    if (time > 0) {
        char buf[32] = "";
        util::text::centiseconds_to_string(int(time), buf, true, true);
        return std::string(buf);
    }
    return std::format("{} apple{}", apples, apples == 1 ? "" : "s");
}

std::string eol::format_level(std::string_view level) {
    std::string with_ext = std::format("{}.lev", level);
    auto idx = get_internal_index(with_ext.c_str());

    if (idx.has_value()) {
        return std::format("internal {:02}", *idx);
    }
    return with_ext;
}

void eol::process(const battle_started& bs) {
    current_battle = bs.bat;
    battle_leaderboard_.clear();

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

void eol::toggle_show_battle_leader() const {
    EolSettings->set_show_battle_leader(!EolSettings->show_battle_leader());
    StatusMessages->add(EolSettings->show_battle_leader() ? "leader from battle status shown"
                                                          : "leader from battle status hidden");
}

void eol::upsert_leaderboard_entry(const battle_leaderboard_entry& entry, uint16_t rank) {
    std::erase_if(battle_leaderboard_, [&](const battle_leaderboard_entry& e) {
        return e.kuski_id == entry.kuski_id && e.kuski_id2 == entry.kuski_id2;
    });
    size_t idx = std::min<size_t>(rank, battle_leaderboard_.size());
    battle_leaderboard_.insert(battle_leaderboard_.begin() + idx, entry);
}

void eol::process(const battle_line_update& e) {
    upsert_leaderboard_entry({.kuski_id = e.kuski_id,
                              .kuski_id2 = e.kuski_id2,
                              .score = e.score,
                              .apple_count = e.apple_count},
                             e.rank);
}

void eol::process(const battle_time_sync& bts) {
    if (!current_battle) {
        LOG_ERROR("Received battle_time_sync message, but no battle is active");
        return;
    }

    current_battle->local_start_ms = bts.local_start_ms;
}

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

    if (!EolSettings->show_battle_leader() || battle_leaderboard_.empty()) {
        return;
    }
    const battle_leaderboard_entry& leader = battle_leaderboard_.front();
    if (leader.score == 0 && leader.apple_count == 0) {
        return;
    }

    const std::string result =
        format_battle_result(current_battle->type, leader.score, leader.apple_count);
    const std::string leader_line =
        leader.kuski_id2 != 0
            ? std::format("Battle leaders: {} & {} {}", lookup_nick(leader.kuski_id),
                          lookup_nick(leader.kuski_id2), result)
            : std::format("Battle leader: {} {}", lookup_nick(leader.kuski_id), result);

    font.write_centered(&dest, dest.get_width() / 2, y - font.line_height(), leader_line.c_str());
}
