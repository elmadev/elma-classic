#ifndef MENU_NAV_H
#define MENU_NAV_H

#include "keys.h"
#include "menu_pic.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

enum class SearchPattern { None, Sorted, Internals };

#define NAV_FUNC(...)                                                                              \
    [__VA_ARGS__]([[maybe_unused]] int choice, [[maybe_unused]] const std::string& left,           \
                  [[maybe_unused]] const std::string& right)
using nav_func = std::function<void(int, std::string&, std::string&)>;

struct nav_row {
    std::string text_left;
    std::string text_right;
    nav_func handler;
};

enum class OverlayAlignment {
    Left,
    Centered,
};

struct nav_overlay {
    std::string text;
    int x;
    int y;
    OverlayAlignment alignment;
};

class menu_nav {
    std::vector<nav_row> entries;
    std::vector<nav_overlay> overlays;
    std::unique_ptr<menu_pic> menu;
    std::string search_input;
    int selected_index;

  public:
    int x_left;
    int y_entries;
    int dy;
    int x_right;
    int y_title;
    bool enable_esc;
    std::string title;
    SearchPattern search_pattern;
    bool search_skip_one;
    int search_skip;

    menu_nav(std::string title);

    void add_row(std::string left, std::string right, nav_func handler = nullptr);
    void add_row(const std::string& left, const nav_func& handler = nullptr) {
        add_row(left, "", handler);
    }
    void add_overlay(std::string text, int x, int y,
                     OverlayAlignment alignment = OverlayAlignment::Left);
    void sort_rows();
    void select_row(int index) { selected_index = index; }
    void select_row(const std::string& left);

    int navigate(bool render_only = false);
    void render();
    size_t row_count() { return entries.size(); }
    std::string& entry_left(int index) { return entries[index].text_left; }
    std::string& entry_right(int index) { return entries[index].text_right; }

  private:
    int prompt_choice(bool render_only);
    int calculate_visible_entries();
    bool search_handler(Keycode code);
};

#endif
