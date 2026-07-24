#include "eol/eol.h"
#include "eol/status_messages.h"
#include "level/level.h"
#include "level/object.h"
#include "log.h"
#include "pic/abc8.h"
#include "pic/pic8.h"
#include "platform/implementation.h"
#include "util/util.h"
#include <algorithm>
#include <filesystem>
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

void eol::set_battle_results_title(const char* label) {
    std::string new_title;
    if (current_battle->type == BattleType::Apple) {
        uint32_t apple_count = current_battle->level_apple_count;
        new_title = std::format("Apple battle {} in {} ({} apple{})", label,
                                format_level(current_battle->level_filename), apple_count,
                                apple_count == 1 ? "" : "s");
    } else {
        new_title =
            std::format("Battle {} in {}", label, format_level(current_battle->level_filename));
    }
    battle_results_table.set_title(new_title);
}

bool eol::in_apple_battle() const {
    return current_battle && current_battle->type == BattleType::Apple &&
           proto.playing_battle_level();
}

static bool has_countdown(BattleType t) {
    using enum BattleType;
    return t == OneLife || t == FirstFinish || t == Apple || t == FinishCount || t == FlagTag;
}

// Flag tag shows the countdown but lets you drive while waiting for the flag.
static bool freezes_during_countdown(BattleType t) {
    return has_countdown(t) && t != BattleType::FlagTag;
}

bool eol::bike_frozen_by_countdown() const {
    return current_battle && current_battle->in_countdown &&
           freezes_during_countdown(current_battle->type) && proto.playing_battle_level() &&
           get_milliseconds() < current_battle->local_start_ms;
}

void apple_battle_progress::clear() { std::ranges::fill(taken, false); }

void apple_battle_progress::record(int object_index) { taken[object_index] = true; }

void apple_battle_progress::apply(const level& lev) const {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* obj = lev.objects[i];
        if (!obj) {
            break;
        }
        if (obj->type == object::Type::Food && taken[i] &&
            obj->property == object::Property::None) {
            obj->active = false;
        }
    }
}

void eol::record_apple_for_apple_battle(int object_index) {
    if (in_apple_battle()) {
        online_apple_battle.record(object_index);
    }
}

void eol::process(const battle_started& bs) {
    current_battle = bs.bat;
    current_battle->level_exists = std::filesystem::exists(
        std::format("lev/{}.lev", (const char*)current_battle->level_filename));
    battle_leaderboard_.clear();
    online_apple_battle.clear();
    set_battle_results_title("standings");
    sync_battle_results_table();

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
    current_battle->local_start_ms = std::min(current_battle->local_start_ms, get_milliseconds());
    StatusMessages->add("battle running");
}

void eol::process(const restore_apple_battle_progress& e) {
    static_assert(std::is_same_v<decltype(e.apples_taken), decltype(online_apple_battle.taken)>);
    std::ranges::copy(e.apples_taken, online_apple_battle.taken);

    auto apple_count = std::ranges::count(e.apples_taken, true);
    StatusMessages->add(std::format("apple battle progress restored ({} apple{} taken)",
                                    apple_count, apple_count == 1 ? "" : "s"));
}

void eol::process(const battle_ended& be) {
    set_battle_results_title("results");
    current_battle.reset();
    online_apple_battle.clear();
    StatusMessages->add(be.aborted ? "battle aborted" : "battle over");
}

void eol::toggle_battle_status() const {
    EolSettings->set_show_battle_status(!EolSettings->show_battle_status());
    StatusMessages->add(EolSettings->show_battle_status() ? "battle status line shown"
                                                          : "battle status line hidden");
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
    sync_battle_results_table();
}

void eol::sync_battle_results_table() {
    battle_results_table.clear_rows();
    if (!current_battle) {
        return;
    }

    for (size_t i = 0; i < battle_leaderboard_.size(); i++) {
        const battle_leaderboard_entry& entry = battle_leaderboard_[i];
        std::string nick = entry.kuski_id2 != 0
                               ? std::format("{}. {} & {}", i + 1, lookup_nick(entry.kuski_id),
                                             lookup_nick(entry.kuski_id2))
                               : std::format("{}. {}", i + 1, lookup_nick(entry.kuski_id));
        std::string result =
            format_battle_result(current_battle->type, entry.score, entry.apple_count);
        battle_results_table.add_row({std::move(nick), std::move(result)});
    }
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
    case Apple:
        return "apple ";
    case Speed:
        return "speed ";
    }
    return "";
}

static std::string format_battle_type(BattleType t) {
    std::string out = std::format("{}battle", battle_type_prefix(t));
    out.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(out.front())));
    return out;
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

    if ((current_battle->attributes & BattleAttributes::Uploaded) &&
        !current_battle->download_requested) {
        out += current_battle->level_exists ? " (F4 to rewrite)" : " (F4 to download)";
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

void eol::render_battle_countdown(pic8& dest, abc8& large_font, abc8& data_font) const {
    if (!current_battle || !has_countdown(current_battle->type) ||
        current_battle->countdown_seconds == 0 || !proto.playing_battle_level()) {
        return;
    }

    const long long remaining_ms = current_battle->local_start_ms - get_milliseconds();

    std::string text;
    if (current_battle->in_countdown && remaining_ms > 0) {
        text = std::format("{}", std::max(1LL, (remaining_ms + 999) / 1000));
    } else if (remaining_ms > -1000) {
        // Show for the first second after the battle started
        text = current_battle->type == BattleType::FlagTag ? "0" : "GOOOOO!!!";
    } else {
        return;
    }

    const int y = eol_table::table_y_offset(dest, data_font) - 106;
    large_font.write_centered(&dest, dest.get_width() / 2, y, text.c_str());
}

void eol::process(const battle_queue_update& e) {
    battle_queue_ = e.entries;
    sync_battle_queue_table();
}

void eol::sync_battle_queue_table() {
    battle_queue_table.clear_rows();
    for (const battle_queue_entry& entry : battle_queue_) {
        std::string_view nick = lookup_nick(entry.designer_id);
        std::string duration =
            std::format("{} min{}", entry.duration_minutes, entry.duration_minutes == 1 ? "" : "s");
        battle_queue_table.add_row(
            {std::string(nick), std::move(duration), format_battle_type(entry.battle_type)});
    }
}
