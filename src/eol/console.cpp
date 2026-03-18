#include "eol/console.h"
#include "abc8.h"
#include "eol_settings.h"
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
}
