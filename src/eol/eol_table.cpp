#include "eol/eol_table.h"
#include "abc8.h"
#include "pic8.h"
#include <algorithm>

constexpr int TITLE_GAP = 22;
constexpr int TITLE_OFFSET = 4;
constexpr int GROUP_GAP = 40;

void eol_table::add_column(int width, Align alignment) { columns.push_back({width, alignment}); }

void eol_table::clear_columns() { columns.clear(); }

void eol_table::add_row(std::vector<std::string> values) { rows.push_back(std::move(values)); }

void eol_table::clear_rows() { rows.clear(); }

void eol_table::render(pic8& dest, abc8& title_font, abc8& data_font, Align alignment) const {
    int y_top = (dest.get_height() * 5) / 6 + data_font.line_height() * 3;

    int x_center;
    switch (alignment) {
    case Align::Left:
        x_center = dest.get_width() / 4;
        break;
    case Align::Center:
        x_center = dest.get_width() / 2;
        break;
    case Align::Right:
        x_center = (dest.get_width() * 3) / 4;
        break;
    }

    // Render title centered
    if (!title.empty()) {
        title_font.write_centered(&dest, x_center, y_top - TITLE_OFFSET, title.c_str());
    }

    int total_rows = static_cast<int>(rows.size());
    if (total_rows <= 0 || columns.empty()) {
        return;
    }

    // Calculate max rows that fit vertically
    int screen_max_rows = std::max((y_top - TITLE_GAP) / data_font.line_height(), 1);

    // Total width of one column group
    int total_col_width = 0;
    for (const auto& col : columns) {
        total_col_width += col.width;
    }

    // Column group layout: each group is total_col_width wide with GROUP_GAP between them.
    // Left-aligned tables grow rightward, right-aligned grow leftward, center grows both ways.
    int group_stride = total_col_width + GROUP_GAP;
    int screen_max_groups = (dest.get_width() + GROUP_GAP) / group_stride;
    int needed_groups = (total_rows + screen_max_rows - 1) / screen_max_rows;
    int num_groups = std::min(needed_groups, screen_max_groups);
    int all_groups_width = num_groups * group_stride - GROUP_GAP;

    int groups_base_x;
    switch (alignment) {
    case Align::Left:
        groups_base_x = x_center - total_col_width / 2;
        break;
    case Align::Center:
        groups_base_x = x_center - all_groups_width / 2;
        break;
    case Align::Right:
        groups_base_x = x_center + total_col_width / 2 - all_groups_width;
        break;
    }

    // draw rows top to bottom, in groups left to right
    for (int i = 0; i < total_rows; i++) {
        int group = i / screen_max_rows;
        int row_in_group = i % screen_max_rows;

        int y = y_top - data_font.line_height() * row_in_group - TITLE_GAP;
        int group_x = groups_base_x + group * group_stride;

        const auto& row = rows[i];
        int col_x = group_x;
        for (int c = 0; c < static_cast<int>(columns.size()); c++) {
            const char* text = (c < static_cast<int>(row.size())) ? row[c].c_str() : "";
            int w = columns[c].width;

            switch (columns[c].alignment) {
            case Align::Left:
                data_font.write(&dest, col_x, y, text);
                break;
            case Align::Center:
                data_font.write_centered(&dest, col_x + w / 2, y, text);
                break;
            case Align::Right:
                data_font.write_right_align(&dest, col_x + w, y, text);
                break;
            }

            col_x += w;
        }
    }
}
