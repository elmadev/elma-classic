#include "eol/console.h"
#include "abc8.h"
#include "keys.h"
#include "main.h"
#include "platform_impl.h"
#include <algorithm>
#include <directinput/scancodes.h>
#include <string>

console* Console = nullptr;

console::console() {
    register_command("clear", "Clear chat history", [this](std::string_view) { clear(); });
}

void console::add_line(std::string text) {
    lines_.push_back(std::move(text));
    if ((int)lines_.size() > MAX_LINES) {
        lines_.erase(lines_.begin());
    }
}

void console::clear() { lines_.clear(); }

bool console::is_input_active() const { return input_active_; }

void console::activate_input() {
    input_active_ = true;
    input_buffer_.clear();
    cursor_pos_ = 0;
}

void console::deactivate_input() {
    input_active_ = false;
    input_buffer_.clear();
    cursor_pos_ = 0;
}

void console::paste_text(std::string_view text) {
    if (!input_active_) {
        return;
    }

    for (char c : text) {
        if (c >= 32 && c < 127 && (int)input_buffer_.size() < MAX_INPUT_LENGTH) {
            input_buffer_.insert(input_buffer_.begin() + cursor_pos_, c);
            cursor_pos_++;
        }
    }
}

void console::handle_input() {
    if (!input_active_) {
        return;
    }

    if (was_key_just_pressed(DIK_RETURN)) {
        if (!input_buffer_.empty()) {
            submit_input();
        }
        deactivate_input();
        return;
    }

    if (was_key_just_pressed(DIK_ESCAPE) || was_key_just_pressed(DIK_F9)) {
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
        if (cursor_pos_ > 0) {
            input_buffer_.erase(cursor_pos_ - 1, 1);
            cursor_pos_--;
        }
    }

    if (was_key_down(DIK_DELETE)) {
        if (cursor_pos_ < (int)input_buffer_.size()) {
            input_buffer_.erase(cursor_pos_, 1);
        }
    }

    if (was_key_down(DIK_LEFT)) {
        if (cursor_pos_ > 0) {
            cursor_pos_--;
        }
    }

    if (was_key_down(DIK_RIGHT)) {
        if (cursor_pos_ < (int)input_buffer_.size()) {
            cursor_pos_++;
        }
    }

    // Drain text input buffer for printable characters
    char c;
    while ((c = pop_text_input()) != 0) {
        if (c >= 32 && c < 127 && (int)input_buffer_.size() < MAX_INPUT_LENGTH) {
            input_buffer_.insert(input_buffer_.begin() + cursor_pos_, c);
            cursor_pos_++;
        }
    }
}

void console::register_command(std::string_view name, std::string_view description,
                            std::function<void(std::string_view args)> callback) {
    commands_[std::string(name)] = {std::string(description), std::move(callback)};
}

void console::submit_input() {
    if (input_buffer_.empty()) {
        return;
    }

    if (input_buffer_[0] == '!') {
        std::string_view input(input_buffer_);
        input.remove_prefix(1);

        // Split into command name and args
        auto space = input.find(' ');
        std::string cmd_name(input.substr(0, space));
        std::string_view args;
        if (space != std::string_view::npos) {
            args = input.substr(space + 1);
        }

        auto it = commands_.find(cmd_name);
        if (it != commands_.end()) {
            it->second.callback(args);
        } else {
            add_line("Unknown command: !" + cmd_name);
        }
    } else {
        add_line(input_buffer_);
        if (on_chat_message) {
            on_chat_message(input_buffer_);
        }
    }
}

void console::render(pic8* surface, abc8* font) {
    int visible_count = std::min((int)lines_.size(), VISIBLE_LINES);
    int start = (int)lines_.size() - visible_count;

    // Backbuffer is flipped: y=0 is bottom of display.
    // Newest line at lowest y, older lines above.
    // Always reserve bottom area for input prompt with extra gap.
    int base_y = MARGIN_Y + LINE_HEIGHT + 8;

    for (int i = 0; i < visible_count; i++) {
        int line_index = start + (visible_count - 1 - i);
        int y = base_y + i * LINE_HEIGHT;
        font->write(surface, MARGIN_X, y, lines_[line_index].c_str());
    }

    if (input_active_) {
        font->write(surface, MARGIN_X, MARGIN_Y, input_buffer_.c_str());

        bool cursor_visible = (get_milliseconds() / 500) % 2 == 0;
        if (cursor_visible) {
            std::string before_cursor = input_buffer_.substr(0, cursor_pos_);
            int cursor_x = MARGIN_X + font->len(before_cursor.c_str());
            font->write(surface, cursor_x, MARGIN_Y, "_");
        }
    }
}
