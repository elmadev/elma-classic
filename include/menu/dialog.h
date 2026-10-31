#ifndef MENU_DIALOG_H
#define MENU_DIALOG_H

#include "platform/scancode.h"
#include <optional>
#include <string_view>

DikScancode menu_dialog(const char* text1, const char* text2 = nullptr, const char* text3 = nullptr,
                        const char* text4 = nullptr, const char* text5 = nullptr,
                        const char* text6 = nullptr);

std::optional<std::string> menu_prompt_string(std::string_view prompt,
                                              std::string_view default_value, int max_length);

std::optional<std::string> menu_prompt_filename(std::string_view prompt,
                                                std::string_view default_value, int max_length);

std::optional<int> menu_prompt_int(std::string_view prompt, std::string_view default_value,
                                   int max_length);

std::optional<double> menu_prompt_double(std::string_view prompt, std::string_view default_value,
                                         int max_length);

#endif
