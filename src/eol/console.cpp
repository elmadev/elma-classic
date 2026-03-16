#include "eol/console.h"
#include "abc8.h"
#include "eol_settings.h"
#include "keys.h"
#include "platform_impl.h"
#include "platform_utils.h"
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

#define REGISTER_SETTINGS_BOOL(field)                                                              \
    register_command(#field, [this](std::string_view text) {                                       \
        if (text.empty()) {                                                                        \
            EolSettings->set_##field(!EolSettings->field());                                       \
        } else {                                                                                   \
            if (auto val = parse_bool(text)) {                                                     \
                EolSettings->set_##field(*val);                                                    \
            } else {                                                                               \
                add_line(std::format("invalid value: {}", text), LineType::System);                \
            }                                                                                      \
        }                                                                                          \
    });

void console::register_console_commands() {
    register_command("clear", [this](std::string_view) { clear(); });
    register_command("dev", [this](std::string_view) { mode = Mode::Console; });
    register_command("chat", [this](std::string_view) { mode = Mode::Chat; });
    REGISTER_SETTINGS_BOOL(show_last_apple_time);
    REGISTER_SETTINGS_BOOL(show_gravity_arrows);
}

void console::add_line(std::string text, LineType type) {
    lines.emplace_back(std::move(text), type);
    if (lines.size() > MAX_LINES) {
        lines.erase(lines.begin());
    }
}

void console::clear() { lines.clear(); }

bool console::is_input_active() const { return input_active; }

void console::activate_input() {
    input_active = true;
    input_buffer.clear();
    cursor_pos = 0;
}

void console::deactivate_input() {
    input_active = false;
    input_buffer.clear();
    cursor_pos = 0;
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
        if (is_ascii_character(c) && input_buffer.size() < MAX_INPUT_LENGTH) {
            input_buffer.insert(input_buffer.begin() + cursor_pos, c);
            cursor_pos++;
        }
    }
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

    // Drain text input buffer for printable characters
    char c;
    while ((c = pop_text_input()) != 0) {
        if (is_ascii_character(c) && input_buffer.size() < MAX_INPUT_LENGTH) {
            input_buffer.insert(input_buffer.begin() + cursor_pos, c);
            cursor_pos++;
        }
    }
}

void console::register_command(std::string_view name,
                               std::function<void(std::string_view args)> callback) {
    commands[std::string(name)] = {std::move(callback)};
}

void console::submit_input() {
    if (input_buffer.empty()) {
        return;
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
        add_line(input_buffer, LineType::Chat);
    }
}

void console::render(pic8& screen, abc8& font) {
    auto filter = [this](const auto& line) {
        if (mode == Mode::Chat) {
            return line.type != LineType::System;
        }

        return true;
    };
    auto view = lines | std::views::reverse | std::views::filter(filter) |
                std::views::take(EolSettings->chat_lines());

    int line_height = font.line_height();
    int y = MARGIN_Y + line_height + 8;
    for (const console_line& line : view) {
        font.write(&screen, MARGIN_X, y, line.text.c_str());
        y += line_height;
    }

    if (input_active) {
        font.write(&screen, MARGIN_X, MARGIN_Y, input_buffer.c_str());

        bool cursor_visible = (get_milliseconds() / 500) % 2 == 0;
        if (cursor_visible) {
            std::string before_cursor = input_buffer.substr(0, cursor_pos);
            int cursor_x = MARGIN_X + font.len(before_cursor.c_str());
            font.write(&screen, cursor_x, MARGIN_Y, "_");
        }
    }
}
