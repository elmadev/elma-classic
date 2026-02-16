#include "eol/console.h"
#include "main.h"
#include <string>

console::console() {}

void console::add_line(std::string text) {
    lines_.push_back(std::move(text));
    if ((int)lines_.size() > MAX_LINES) {
        lines_.erase(lines_.begin());
    }
}

void console::clear() { lines_.clear(); }
