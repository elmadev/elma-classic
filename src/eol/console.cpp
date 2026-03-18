#include "eol/console.h"
#include <string>

void console::add_line(std::string text, LineType type) {
    lines.emplace_back(std::move(text), type);
    if (lines.size() > MAX_LINES) {
        lines.erase(lines.begin());
    }
}

void console::clear() { lines.clear(); }
