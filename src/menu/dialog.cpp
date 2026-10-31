#include "menu/dialog.h"
#include "main.h"
#include "menu/pic.h"
#include "pic/abc8.h"
#include "pic/surface.h"
#include "platform/implementation.h"
#include "platform/scancode.h"
#include "platform/text_input.h"
#include "util/util.h"
#include <functional>
#include <string>

// Display the menu with the provided text, then return the key pressed by the user.
DikScancode menu_dialog(const char* text1, const char* text2, const char* text3, const char* text4,
                        const char* text5, const char* text6) {
    // Count the number of entries
    constexpr int MENU_DIALOG_MAX_LENGTH = 6;
    const char* text_array[MENU_DIALOG_MAX_LENGTH + 1] = {text1, text2, text3,  text4,
                                                          text5, text6, nullptr};
    int text_count = 0;
    while (text_array[text_count] && text_count < MENU_DIALOG_MAX_LENGTH) {
        text_count += 1;
    }
    if (text_count <= 0) {
        internal_error("menu_dialog text_count <= 0!");
    }

    // Render and prompt for key input
    menu_pic menu;
    int dy = 40;
    int y0 = 240 - (text_count - 1) / 2 * dy;
    for (int i = 0; i < text_count; i++) {
        menu.add_line_centered(text_array[i], 320, y0 + i * dy);
    }

    return menu.loop_until_any_key();
}

using validate_func = std::function<bool(unsigned char input)>;

// Prompt for input and return the entered text, or nullopt if cancelled.
static std::optional<std::string> menu_prompt(std::string_view prompt,
                                              std::string_view default_value, int max_length,
                                              const validate_func& validator) {
    // Split the prompt along \n
    std::vector<std::string> prompt_lines;
    std::size_t start = 0;
    while (true) {
        const std::size_t end = prompt.find('\n', start);
        const std::size_t length =
            end == std::string_view::npos ? std::string_view::npos : end - start;
        prompt_lines.emplace_back(prompt.substr(start, length));
        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }
    int lines = prompt_lines.size() + 1;

    // Interface to input
    menu_pic menu;
    std::string input(default_value);
    std::size_t i = input.length();
    empty_keypress_buffer();
    bool rerender = true;
    while (true) {
        handle_events();
        if (was_key_just_pressed(DIK_ESCAPE)) {
            return std::nullopt;
        }
        if (was_key_just_pressed(DIK_RETURN)) {
            if (input.empty()) {
                return std::nullopt;
            }
            return input;
        }
        if (was_key_down(DIK_BACK)) {
            if (i > 0) {
                input.erase(--i, 1);
                rerender = true;
            }
        }
        if (was_key_down(DIK_DELETE)) {
            if (i < input.length()) {
                input.erase(i, 1);
                rerender = true;
            }
        }
        if (was_key_down(DIK_LEFT)) {
            if (i > 0) {
                i--;
            }
            rerender = true;
        }
        if (was_key_down(DIK_RIGHT)) {
            if (i < input.length()) {
                i++;
            }
            rerender = true;
        }

        if (was_left_mouse_just_clicked()) {
            // Correct position based on screen size
            int mouse_x;
            int mouse_y;
            get_mouse_position(&mouse_x, &mouse_y);
            mouse_x = mouse_x + 320 - SCREEN_WIDTH / 2;
            mouse_y = mouse_y + 240 - SCREEN_HEIGHT / 2;

            // Clickable box
            int y_start = 150 + lines * 30;
            int y_end = y_start + MenuFont->line_height();
            int len = MenuFont->len(input.c_str());
            int x_start = 320 - len / 2;
            int x_end = 320 + len / 2;
            if (mouse_y >= y_start && mouse_y <= y_end && mouse_x >= x_start && mouse_x <= x_end) {
                // Seek the correct cursor position
                i = 0;
                while (i < input.length()) {
                    int char_width = MenuFont->len(input.substr(i, 1).c_str());
                    if (x_start + char_width >= mouse_x) {
                        if (mouse_x - x_start > char_width / 2) {
                            // Clicked on the right half of the character
                            i++;
                        }
                        break;
                    }
                    x_start += char_width;
                    i++;
                }
                rerender = true;
            }
        }

        while (has_text_input()) {
            char c = pop_text_input();
            if (input.size() < max_length && MenuFont->has_char(c) && validator(c)) {
                input.insert(input.begin() + i++, c);
                rerender = true;
            }
        }
        if (rerender) {
            rerender = false;
            menu.clear();

            int y = 210 - 30 * lines;
            for (int j = 0; j < prompt_lines.size(); j++) {
                menu.add_line_centered(prompt_lines[j], 320, y + 60 * j);
            }

            int len = MenuFont->len(input.c_str());
            int underscore_offset = MenuFont->len(input.substr(0, i).c_str());
            menu.add_line_centered(input, 320, y + 60 * lines - 60);
            menu.add_line("_", 320 - len / 2 + underscore_offset, y + 60 * (lines - 1));
        }
        menu.render();
    }
}

std::optional<std::string> menu_prompt_string(std::string_view prompt,
                                              std::string_view default_value, int max_length) {
    return menu_prompt(prompt, default_value, max_length, util::text::is_ascii_char);
}

std::optional<std::string> menu_prompt_filename(std::string_view prompt,
                                                std::string_view default_value, int max_length) {
    return menu_prompt(prompt, default_value, max_length, util::text::is_filename_char);
}

std::optional<int> menu_prompt_int(std::string_view prompt, std::string_view default_value,
                                   int max_length) {
    std::optional<std::string> ret =
        menu_prompt(prompt, default_value, max_length, util::text::is_ascii_digit);
    if (!ret) {
        return std::nullopt;
    }
    return std::strtol(ret->data(), nullptr, 10);
}

std::optional<double> menu_prompt_double(std::string_view prompt, std::string_view default_value,
                                         int max_length) {
    auto validator = [](unsigned char c) {
        if (util::text::is_ascii_digit(c)) {
            return true;
        }
        if (c == '.') {
            return true;
        }
        return false;
    };
    std::optional<std::string> ret = menu_prompt(prompt, default_value, max_length, validator);
    if (!ret) {
        return std::nullopt;
    }
    return std::strtod(ret->data(), nullptr);
}
