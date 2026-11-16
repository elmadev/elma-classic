#include "eol/checkpoint_table.h"
#include "eol/settings.h"
#include "pic/abc8.h"
#include "pic/surface.h"
#include "renderer/render.h"

box checkpoint_table::click_box() const {
    box bx;

    // Horizontal position
    HorizontalAlign horizontal_alignment = EolSettings->checkpoint_table_horizontal_pos();
    int dx = EolSettings->checkpoint_table_dx();
    switch (horizontal_alignment) {
    case HorizontalAlign::Left:
        bx.x1 = dx;
        bx.x2 = dx + width();
        break;
    case HorizontalAlign::Center:
        bx.x1 = SCREEN_WIDTH / 2 + dx - width() / 2;
        bx.x2 = SCREEN_WIDTH / 2 + dx + width() / 2;
        break;
    case HorizontalAlign::Right:
        bx.x1 = SCREEN_WIDTH + dx - width();
        bx.x2 = SCREEN_WIDTH + dx;
        break;
    }

    // Vertical position
    VerticalAlign vertical_alignment = EolSettings->checkpoint_table_vertical_pos();
    int dy = EolSettings->checkpoint_table_dy();
    int height = EolSettings->checkpoint_table_height();
    switch (vertical_alignment) {
    case VerticalAlign::Top:
        bx.y1 = SCREEN_HEIGHT + dy - height;
        bx.y2 = SCREEN_HEIGHT + dy;
        break;
    case VerticalAlign::Center:
        bx.y1 = SCREEN_HEIGHT / 2 + dy - height / 2;
        bx.y2 = SCREEN_HEIGHT / 2 + dy + height / 2;
        break;
    case VerticalAlign::Bottom:
        bx.y1 = dy;
        bx.y2 = dy + height;
        break;
    }

    return bx;
}

void checkpoint_table::set_box(const box& new_box) {
    int x = (new_box.x1 + new_box.x2) / 2;
    int y = (new_box.y1 + new_box.y2) / 2;

    // Horizontal position
    if (x < SCREEN_WIDTH / 3) {
        EolSettings->persist_checkpoint_table_horizontal_pos(HorizontalAlign::Left);
        EolSettings->persist_checkpoint_table_dx(std::max(0, new_box.x1));
    } else if (x < SCREEN_WIDTH * 2 / 3) {
        EolSettings->persist_checkpoint_table_horizontal_pos(HorizontalAlign::Center);
        EolSettings->persist_checkpoint_table_dx(x - SCREEN_WIDTH / 2);
    } else {
        EolSettings->persist_checkpoint_table_horizontal_pos(HorizontalAlign::Right);
        EolSettings->persist_checkpoint_table_dx(std::min(0, new_box.x2 - SCREEN_WIDTH));
    }

    // Vertical position
    if (y < SCREEN_HEIGHT / 3) {
        EolSettings->persist_checkpoint_table_vertical_pos(VerticalAlign::Bottom);
        EolSettings->persist_checkpoint_table_dy(std::max(0, new_box.y1));
    } else if (y < SCREEN_HEIGHT * 2 / 3) {
        EolSettings->persist_checkpoint_table_vertical_pos(VerticalAlign::Center);
        EolSettings->persist_checkpoint_table_dy(y - SCREEN_HEIGHT / 2);
    } else {
        EolSettings->persist_checkpoint_table_vertical_pos(VerticalAlign::Top);
        EolSettings->persist_checkpoint_table_dy(std::min(0, new_box.y2 - SCREEN_HEIGHT));
    }

    EolSettings->persist_checkpoint_table_height(std::max(24, new_box.y2 - new_box.y1));
}

void checkpoint_table::reset_box() {
    EolSettings->persist_checkpoint_table_horizontal_pos(
        EolSettings->checkpoint_table_horizontal_pos_default());
    EolSettings->persist_checkpoint_table_dx(EolSettings->checkpoint_table_dx_default());
    EolSettings->persist_checkpoint_table_vertical_pos(
        EolSettings->checkpoint_table_vertical_pos_default());
    EolSettings->persist_checkpoint_table_dy(EolSettings->checkpoint_table_dy_default());
    EolSettings->persist_checkpoint_table_height(EolSettings->checkpoint_table_height_default());
}

void checkpoint_table::middle_clicked(const game_mouse& /*pos*/) {}

checkpoint_table::checkpoint_table()
    : box_overlay_clickable(false, true) {}

constexpr int COL_MARGIN = 10;

void checkpoint_table::add_line(checkpoint_line line) {
    abc8* font = EolSettings->large_font() ? MediumFont : SmallFont;
    name_width = std::max(name_width, COL_MARGIN + font->len(line.name.c_str()));
    time_width = std::max(time_width, COL_MARGIN + font->len(line.time.c_str()));
    diff_width = std::max(diff_width, COL_MARGIN + font->len(line.diff.c_str()));
    speed_width = std::max(speed_width, COL_MARGIN + font->len(line.speed.c_str()));

    lines.emplace_back(std::move(line));
}

void checkpoint_table::reset_lines() {
    abc8* font = EolSettings->large_font() ? MediumFont : SmallFont;
    name_width = COL_MARGIN + font->len("Checkpoint 0");
    time_width = COL_MARGIN + font->len("00:00");
    diff_width = COL_MARGIN + font->len("+00:00");
    speed_width = COL_MARGIN + font->len("000.00");

    lines.clear();
}

static bool show_speed() {
    CheckpointSpeed speed = EolSettings->checkpoint_table_speed();
    if (speed == CheckpointSpeed::On) {
        return true;
    }
    if (speed == CheckpointSpeed::IfSpeedometer && EolSettings->show_speedometer()) {
        return true;
    }
    return false;
}

int checkpoint_table::width() const {
    int width = 0;
    width += name_width;
    width += time_width;
    if (EolSettings->checkpoint_table_diff()) {
        width += diff_width;
    }
    if (show_speed()) {
        width += speed_width;
    }
    return width;
}

void checkpoint_table::render(pic8& screen, bool force_title) const {
    if (lines.empty() && !force_title) {
        return;
    }

    box bx = click_box();

    // Calculate number of visible rows
    abc8* font = EolSettings->large_font() ? MediumFont : SmallFont;
    int dy = font->line_height();
    int rows = (bx.y2 - bx.y1) / dy - 1;
    rows = std::max(rows, 1);

    // Calculate x positions
    int x = bx.x1;
    int x_name = x;
    x += name_width;
    x += time_width;
    int x_time = x;

    bool diff = EolSettings->checkpoint_table_diff();
    if (diff) {
        x += diff_width;
    }
    int x_diff = x;

    bool speed = show_speed();
    if (speed) {
        x += speed_width;
    }
    int x_speed = x;

    // Draw the column headers
    int header_y = bx.y1 + rows * dy;
    font->write_right_align(&screen, x_time, header_y, "Time");
    if (diff) {
        font->write_right_align(&screen, x_diff, header_y, "Diff");
    }
    if (speed) {
        font->write_right_align(&screen, x_speed, header_y, "Speed");
    }

    // Draw the lines
    int line_offset = std::max(0, (int)lines.size() - rows);
    int row_i = 0;
    for (int i = line_offset; i < lines.size(); i++) {
        int y = bx.y1 + (rows - row_i - 1) * dy;
        const checkpoint_line& line = lines[i];
        font->write(&screen, x_name, y, line.name.c_str());
        font->write_right_align(&screen, x_time, y, line.time.c_str());
        if (diff) {
            font->write_right_align(&screen, x_diff, y, line.diff.c_str());
        }
        if (speed) {
            font->write_right_align(&screen, x_speed, y, line.speed.c_str());
        }
        row_i++;
    }
}

checkpoint_table CheckpointTable;
