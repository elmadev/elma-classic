#include "eol/console.h"
#include "abc8.h"
#include "eol_settings.h"
#include "keys.h"
#include "platform_impl.h"
#include "platform_utils.h"
#include <directinput/scancodes.h>
#include <string>
#include <ranges>

console* Console = nullptr;

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

void console::handle_input() {
    if (!input_active) {
        return;
    }

    if (was_key_just_pressed(DIK_RETURN)) {
        if (!input_buffer.empty()) {
            add_line(input_buffer, LineType::Chat);
        }
        deactivate_input();
        return;
    }

    if (was_key_just_pressed(DIK_ESCAPE)) {
        deactivate_input();
        return;
    }

    if (was_key_down(DIK_BACK)) {
        if (cursor_pos > 0) {
            input_buffer.erase(cursor_pos - 1, 1);
            cursor_pos--;
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
