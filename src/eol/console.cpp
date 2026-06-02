#include "eol/console.h"
#include "pic/abc8.h"
#include "eol/eol.h"
#include "eol/status_messages.h"
#include "eol/settings.h"
#include "keys.h"
#include "log.h"
#include "platform/implementation.h"
#include "platform/utils.h"
#include "util/util.h"
#include <directinput/scancodes.h>
#include <format>
#include <optional>
#include <string>
#include <ranges>

console* Console = nullptr;

static std::optional<bool> parse_bool(std::string_view text) {
    if (text.empty()) {
        return {};
    }

    if (strcmpi(text.data(), "yes") == 0 || strcmpi(text.data(), "true") == 0) {
        return true;
    }

    if (strcmpi(text.data(), "no") == 0 || strcmpi(text.data(), "false") == 0) {
        return false;
    }

    if (text.size() != 1) {
        return {};
    }

    switch (text[0]) {
    case 'y':
    case 'Y':
    case '1':
        return true;
    case 'n':
    case 'N':
    case '0':
        return false;
    default:
        break;
    }

    return {};
}

#define REGISTER_SETTINGS_STR(field)                                                               \
    register_command(#field,                                                                       \
                     [](std::string_view text) { EolSettings->set_##field(std::string(text)); });

#define REGISTER_SETTINGS_BOOL(field)                                                              \
    register_command(#field, [this](std::string_view text) {                                       \
        if (text.empty()) {                                                                        \
            EolSettings->set_##field(!EolSettings->field());                                       \
            StatusMessages->add(                                                                   \
                std::format("{}: {}", #field, EolSettings->field() ? "on" : "off"));               \
        } else {                                                                                   \
            if (auto val = parse_bool(text)) {                                                     \
                EolSettings->set_##field(*val);                                                    \
                StatusMessages->add(std::format("{}: {}", #field, *val ? "on" : "off"));           \
            } else {                                                                               \
                add_line(std::format("invalid value: {}", text), LineType::System);                \
            }                                                                                      \
        }                                                                                          \
    });

void console::register_console_commands() {
    register_command("clear", [this](std::string_view) { clear(); });
    register_command("dev", [this](std::string_view) { mode = Mode::Console; });
    register_command("chat", [this](std::string_view) { mode = Mode::Chat; });
    register_command("log", [this](std::string_view text) {
        if (text.empty()) {
            show_log_lines = !show_log_lines;
        } else if (auto val = parse_bool(text)) {
            show_log_lines = *val;
        } else {
            add_line(std::format("invalid value: {}", text), LineType::System);
            return;
        }
        StatusMessages->add(std::format("log: {}", show_log_lines ? "on" : "off"));
    });
    REGISTER_SETTINGS_STR(default_lgr_name);
    REGISTER_SETTINGS_BOOL(show_last_apple_time);
    REGISTER_SETTINGS_BOOL(show_gravity_arrows);
    REGISTER_SETTINGS_BOOL(cripple_no_brake);
    register_alias("nobrake", "cripple_no_brake");
    register_alias("nb", "cripple_no_brake");
    REGISTER_SETTINGS_BOOL(cripple_no_throttle);
    register_alias("nothrottle", "cripple_no_throttle");
    register_alias("ng", "cripple_no_throttle");
    register_alias("nth", "cripple_no_throttle");
    register_alias("nogas", "cripple_no_throttle");
    REGISTER_SETTINGS_BOOL(cripple_always_throttle);
    register_alias("alwaysthrottle", "cripple_always_throttle");
    register_alias("at", "cripple_always_throttle");
    register_alias("ag", "cripple_always_throttle");
    register_alias("ath", "cripple_always_throttle");
    register_alias("alwaysgas", "cripple_always_throttle");
    REGISTER_SETTINGS_BOOL(cripple_no_turn);
    register_alias("noturn", "cripple_no_turn");
    register_alias("nt", "cripple_no_turn");
    REGISTER_SETTINGS_BOOL(cripple_no_volt);
    register_alias("novolt", "cripple_no_volt");
    register_alias("nv", "cripple_no_volt");
    REGISTER_SETTINGS_BOOL(cripple_one_turn);
    register_alias("oneturn", "cripple_one_turn");
    register_alias("ot", "cripple_one_turn");
    REGISTER_SETTINGS_BOOL(cripple_drunk);
    register_alias("drunk", "cripple_drunk");
    register_alias("dr", "cripple_drunk");
    register_command("download", [](std::string_view text) { EolClient->download_level(text); });
    register_alias("dl", "download");
    register_command("download_battle",
                     [](std::string_view /*text*/) { EolClient->download_battle_level(); });
    register_alias("dlb", "download");
}

void console::add_line(std::string text, LineType type) {
    // Drop glyphs the font can't render
    if (font) {
        std::erase_if(text, [this](char c) { return !font->has_char((unsigned char)c); });
    }

    // If we're currently rendering, defer adding the line until after rendering is done to avoid
    // modifying the lines vector while it's being iterated over.
    if (rendering) {
        deferred_lines.emplace_back(std::move(text), type);
        return;
    }

    lines.emplace_back(std::move(text), type);
    if (lines.size() > MAX_LINES) {
        lines.erase(lines.begin());
    }
    if (scroll_offset > 0 && should_show(lines.back())) {
        // Maintain scroll offset when new lines are added so the chat doesn't scroll
        scroll_offset = std::min(scroll_offset + 1, max_scroll_offset());
    }
}

void console::set_font(abc8* new_font) { font = new_font; }

void console::clear() {
    lines.clear();
    scroll_offset = 0;
}

bool console::is_input_active() const { return input_active; }

void console::activate_input() {
    input_active = true;
    input_buffer.clear();
    cursor_pos = 0;
    scroll_offset = 0;
}

void console::deactivate_input() {
    input_active = false;
    input_buffer.clear();
    cursor_pos = 0;
    scroll_offset = 0;
    if (clear_label_on_submit) {
        clear_label_mode();
    }
}

void console::toggle_active() {
    empty_keypress_buffer();
    if (input_active) {
        deactivate_input();
    } else {
        activate_input();
    }
}

void console::paste_text(std::string_view text) {
    if (!input_active) {
        return;
    }

    for (char c : text) {
        if (util::text::is_ascii_char(c) && input_buffer.size() < MAX_INPUT_LENGTH) {
            input_buffer.insert(input_buffer.begin() + cursor_pos, c);
            cursor_pos++;
        }
    }
}

bool console::should_show(const console_line& line) const {
    if (line.type == LineType::Log) {
        return show_log_lines;
    }
    if (mode == Mode::Chat) {
        return line.type != LineType::System;
    }
    return true;
}

int console::max_scroll_offset() const {
    auto filtered = lines | std::views::filter([this](const auto& l) { return should_show(l); });
    return std::max(0, (int)std::ranges::distance(filtered) - EolSettings->chat_lines());
}

void console::handle_input() {
    if (!input_active) {
        return;
    }

    if (was_key_just_pressed(DIK_RETURN)) {
        if (!input_buffer.empty()) {
            submit_input();
        }
        deactivate_input();
        return;
    }

    if (was_key_just_pressed(DIK_ESCAPE)) {
        deactivate_input();
        return;
    }

    if (was_key_just_pressed(DIK_V) && is_shortcut_modifier_down()) {
        std::string clipboard = get_clipboard_text();
        if (!clipboard.empty()) {
            paste_text(clipboard);
        }
        return;
    }

    if (was_key_down(DIK_BACK)) {
        if (cursor_pos > 0) {
            input_buffer.erase(cursor_pos - 1, 1);
            cursor_pos--;
        }
    }

    if (was_key_down(DIK_DELETE)) {
        if (cursor_pos < (int)input_buffer.size()) {
            input_buffer.erase(cursor_pos, 1);
        }
    }

    if (was_key_down(DIK_LEFT)) {
        if (cursor_pos > 0) {
            cursor_pos--;
        }
    }

    if (was_key_down(DIK_RIGHT)) {
        if (cursor_pos < (int)input_buffer.size()) {
            cursor_pos++;
        }
    }

    if (was_key_down(DIK_HOME)) {
        cursor_pos = 0;
    }

    if (was_key_down(DIK_END)) {
        cursor_pos = (int)input_buffer.size();
    }

    if (was_key_down(DIK_UP)) {
        scroll_offset = std::min(scroll_offset + 1, max_scroll_offset());
    }

    if (was_key_down(DIK_DOWN)) {
        scroll_offset = std::max(scroll_offset - 1, 0);
    }

    if (was_key_down(DIK_PRIOR)) {
        scroll_offset =
            std::min(scroll_offset + EolSettings->chat_lines() - 1, max_scroll_offset());
    }

    if (was_key_down(DIK_NEXT)) {
        scroll_offset = std::max(scroll_offset - EolSettings->chat_lines() + 1, 0);
    }

    // Drain text input buffer for printable characters
    char c;
    while ((c = pop_text_input()) != 0) {
        if (util::text::is_ascii_char(c) && input_buffer.size() < MAX_INPUT_LENGTH) {
            input_buffer.insert(input_buffer.begin() + cursor_pos, c);
            cursor_pos++;
        }
    }
}

void console::register_command(std::string_view name,
                               std::function<void(std::string_view args)> callback) {
    commands[std::string(name)] = {std::move(callback)};
}

void console::register_alias(std::string_view alias, const std::string& cmd) {
    register_command(alias, [this, cmd](std::string_view text) {
        auto it = commands.find(cmd);
        if (it != commands.end()) {
            it->second.callback(text);
        } else {
            add_line(std::format("Unknown command: !{}", cmd), LineType::System);
        }
    });
}

void console::submit_input() {
    if (input_buffer.empty()) {
        return;
    }

    if (!input_label_alias.empty()) {
        if (!label_allow_commands || input_buffer[0] != '!') {
            input_buffer = input_label_alias + input_buffer;
        }
        if (clear_label_on_submit) {
            clear_label_mode();
        }
    }

    bool commands_need_prefix = mode == Mode::Chat;
    if (input_buffer[0] == '!' || !commands_need_prefix) {
        add_line(input_buffer, LineType::System);

        std::string_view input(input_buffer);
        if (input_buffer[0] == '!') {
            input.remove_prefix(1);
        }

        // Split into command name and args
        auto space = input.find(' ');
        std::string cmd_name(input.substr(0, space));
        std::string_view args;
        if (space != std::string_view::npos) {
            args = input.substr(space + 1);
        }

        auto it = commands.find(cmd_name);
        if (it != commands.end()) {
            it->second.callback(args);
        } else {
            add_line("Unknown command: !" + cmd_name, LineType::System);
        }
    } else {
        EolClient->send_chat(input_buffer);
    }
}

void console::label_mode(std::string label, std::string label_alias, bool clear_label,
                         bool allow_commands) {
    input_label = std::move(label);
    input_label_alias = std::move(label_alias);
    clear_label_on_submit = clear_label;
    label_allow_commands = allow_commands;
}

void console::clear_label_mode() {
    input_label = "";
    input_label_alias = "";
    clear_label_on_submit = false;
    label_allow_commands = false;
}

void console::render(pic8& screen) {
    if (!font) {
        LOG_ERROR("Cannot render console: font not set");
        return;
    }
    rendering = true;

    auto view = lines | std::views::reverse |
                std::views::filter([this](const auto& l) { return should_show(l); }) |
                std::views::drop(scroll_offset) | std::views::take(EolSettings->chat_lines());

    int line_height = font->line_height();
    int y = MARGIN_Y + line_height + 8;
    for (const console_line& line : view) {
        font->write(&screen, MARGIN_X, y, line.text.c_str());
        y += line_height;
    }

    if (input_active) {
        int input_x = MARGIN_X;
        if (!input_label.empty()) {
            font->write(&screen, input_x, MARGIN_Y, input_label.c_str());
            input_x += font->len(input_label.c_str());
        }
        font->write(&screen, input_x, MARGIN_Y, input_buffer.c_str());

        bool cursor_visible = (get_milliseconds() / 500) % 2 == 0;
        if (cursor_visible) {
            std::string before_cursor = input_buffer.substr(0, cursor_pos);
            int cursor_x = input_x + font->len(before_cursor.c_str());
            font->write(&screen, cursor_x, MARGIN_Y, "_");
        }
    }

    rendering = false;

    for (auto& line : deferred_lines) {
        add_line(std::move(line.text), line.type);
    }

    deferred_lines.clear();
}
