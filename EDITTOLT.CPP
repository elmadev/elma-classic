#include "EDITTOLT.H"
#include "abc8.h"
#include "editor_canvas.h"
#include "editor_dialog.h"
#include "EDITUJ.H"
#include "fs_utils.h"
#include "keys.h"
#include "level.h"
#include "level_load.h"
#include "lgr.h"
#include "M_PIC.H"
#include "main.h"
#include "menu_pic.h"
#include "pic8.h"
#include "platform_impl.h"
#include "platform_utils.h"
#include "polygon.h"
#include "util/util.h"
#include "sprite.h"
#include <algorithm>
#include <cstring>
#include <directinput/scancodes.h>
#include <vector>

std::vector<std::string> ListEntries;

static void sort_files() {
    std::sort(ListEntries.begin(), ListEntries.end(),
              [](const std::string& a, const std::string& b) {
                  // Case-insensitive comparison
                  int result = strcmpi(b.c_str(), a.c_str());
                  if (result > 0) {
                      return true;
                  }
                  if (result < 0) {
                      return false;
                  }
                  // If identical, case-sensitive comparison
                  return b > a;
              });
}

static int populate_list(const char* pattern, int max_filename_length) {
    ListEntries.clear();
    ListEntries.reserve(100);

    finame filename;
    bool done = find_first(pattern, filename, max_filename_length);
    while (!done) {
        if (strlen(filename) > max_filename_length + 4) {
            internal_error("populate_list strlen(filename) > max_filename_length + 4");
        }

        // Remove extension
        int i = strlen(filename) - 1;
        while (i > 0) {
            if (filename[i] == '.') {
                filename[i] = 0;
                break;
            }
            i--;
        }

        if (strlen(filename) > max_filename_length) {
            internal_error("populate_list strlen(filename) > max_filename_length");
        }

        ListEntries.emplace_back(filename);

        done = find_next(filename);
    }
    find_close();

    sort_files();

    return ListEntries.size();
}

static int list_index(const std::string& target) {
    for (int i = 0; i < ListEntries.size(); i++) {
        if (strcmpi(ListEntries[i].c_str(), target.c_str()) == 0) {
            return i;
        }
    }
    return 0;
}

static void open_level(const char* filename) {
    // Sanity checks
    if (!Ptop) {
        internal_error("open_level !Ptop");
    }
    if (!*filename) {
        internal_error("open_level !*filename");
    }
    char tmp[15];
    if (strlen(filename) > MAX_FILENAME_LEN) {
        internal_error("open_level strlen(filename) > MAX_FILENAME_LEN");
    }
    strcpy(tmp, filename);
    strcat(tmp, ".lev");
    if (strlen(tmp) > 12) {
        internal_error("open_level strlen( tmp ) > 12!");
    }
    strcpy(State->editor_filename, tmp);

    // Load level
    Valtozott = 0;
    invalidate_level();
    if (!load_level_editor(tmp)) {
        State->editor_filename[0] = 0;
    }
    zoom_fill();
}

static void adjust_list_view(int& selected_index, int& view_index, int length,
                             int max_visible_entries, bool& rerender, box& box_up, box& box_down,
                             box& box_list) {
    if (was_key_down(DIK_UP)) {
        selected_index--;
        rerender = true;
    }
    if (was_key_down(DIK_PRIOR) ||
        (was_left_mouse_just_clicked() && is_in_box(Moux, Mouy, box_up))) {
        selected_index -= max_visible_entries - 1;
        rerender = true;
    }
    if (was_key_down(DIK_DOWN)) {
        selected_index++;
        rerender = true;
    }
    if (was_key_down(DIK_NEXT) ||
        (was_left_mouse_just_clicked() && is_in_box(Moux, Mouy, box_down))) {
        selected_index += max_visible_entries - 1;
        rerender = true;
    }
    int wheel = get_mouse_wheel_delta();
    if (wheel != 0 && is_in_box(Moux, Mouy, box_list)) {
        selected_index -= wheel;
        rerender = true;
    }

    selected_index = std::max(selected_index, 0);
    selected_index = std::min(selected_index, length - 1);
    view_index = std::min(view_index, selected_index);
    view_index = std::max(view_index, selected_index - max_visible_entries + 1);
}

// Draw a borderless box with background color and centered text
// Optionally add '_' to the end of the string for input prompt
static void draw_textbox_centered(pic8* pic, box bx, unsigned char fill_id, const char* text,
                                  bool add_underline = false) {
    pic->fill_box(bx.x1 + 1, bx.y1 + 1, bx.x2 - 1, bx.y2 - 1, fill_id);
    if (add_underline) {
        int length = Pabc2->len(text);
        int x = (bx.x1 + bx.x2) / 2 - length / 2 - 3;
        Pabc2->write(pic, x, (bx.y1 + bx.y2) / 2 + 5, text);
        for (int i = 0; i < 6; i++) {
            pic->ppixel(x + length + i + 1, (bx.y1 + bx.y2) / 2 + 4, 0);
        }
    } else {
        Pabc2->write_centered(pic, (bx.x1 + bx.x2) / 2, (bx.y1 + bx.y2) / 2 + 5, text);
    }
}

// Draw a borderless box with background color and left-justified text
static void draw_textbox_left(pic8* pic, box bx, unsigned char fill_id, const char* text) {
    pic->fill_box(bx.x1 + 1, bx.y1 + 1, bx.x2 - 1, bx.y2 - 1, fill_id);
    Pabc2->write(pic, bx.x1 + 2, (bx.y1 + bx.y2) / 2 + 5, text);
}

// Draw an up or down arrow (no background box)
static void draw_arrow(pic8* pic, box bx, unsigned char palette_id, bool up_arrow) {
    int x1 = bx.x1;
    int y1 = bx.y1;
    int x2 = bx.x2;
    int y2 = bx.y2;
    int half_width = 16;
    x1 += half_width;
    x2 -= half_width;
    y1 += 5;
    y2 -= 8;
    int x_center = (x1 + x2) / 2;
    for (int x_right = x1; x_right <= x_center; x_right++) {
        int x_left = x_center + (x_center - x_right);
        int y = int(y1 + (y2 - y1) / double(x_center - x1) * (x_right - x1));
        if (up_arrow) {
            y = bx.y2 - (y - bx.y1);
            pic->ppixel(x_right, y, palette_id);
            pic->ppixel(x_right, y - 3, palette_id);
            pic->ppixel(x_left, y, palette_id);
            pic->ppixel(x_left, y - 3, palette_id);
        } else {
            pic->ppixel(x_right, y, palette_id);
            pic->ppixel(x_right, y + 3, palette_id);
            pic->ppixel(x_left, y, palette_id);
            pic->ppixel(x_left, y + 3, palette_id);
        }
    }
}

static std::string editor_window_list_levels(bool show_new_button) {
    int list_length = populate_list("lev/*.lev", MAX_FILENAME_LEN);

    if (list_length < 1) {
        return "";
    }

    std::string current_filename = State->editor_filename;
    if (current_filename.length() > 4) {
        current_filename.erase(current_filename.length() - 4);
    }
    int selected_index = list_index(current_filename);

    // Display menu
    int max_visible_entries = 10;
    int dy = 20;
    int x1 = 200;
    int y1 = 100;
    int x2 = 401;
    int y2 = 174 + max_visible_entries * dy;
    int lx1 = x1 + 10;
    int ly1 = y1 + 37;
    int lx2 = lx1 + 100;
    int ly2 = ly1 + max_visible_entries * dy;

    box box_list = {lx1, ly1, lx2, ly2};
    box box_up = {x1 + 10, y1 + 11, x1 + 110, y1 + 31};
    box box_down = {x1 + 10, y2 - 30, x1 + 110, y2 - 10};
    box box_cancel = {x1 + 121, (y2 + y1) / 2 - 10, x1 + 121 + 70, (y2 + y1) / 2 + 10};
    box box_new = {x1 + 121, (y2 + y1) / 2 - 40, x1 + 121 + 70, (y2 + y1) / 2 - 20};

    int view_index = 0;
    bool rerender = true;
    empty_keypress_buffer();
    while (true) {
        handle_events();
        update_and_draw_cursor();
        adjust_list_view(selected_index, view_index, list_length, max_visible_entries, rerender,
                         box_up, box_down, box_list);
        if (was_key_just_pressed(DIK_ESCAPE)) {
            return "";
        }
        if (was_key_just_pressed(DIK_RETURN)) {
            return ListEntries[selected_index];
        }
        if (was_left_mouse_just_clicked()) {
            if (is_in_box(Moux, Mouy, box_cancel)) {
                return "";
            }
            if (show_new_button && is_in_box(Moux, Mouy, box_new)) {
                return "";
            }
            if (Moux > lx1 && Moux < lx2 && Mouy > ly1 && Mouy < ly2) {
                if (Mouy < ly1 + dy * max_visible_entries) {
                    int index = (Mouy - ly1) / dy;
                    index += view_index;
                    if (index < list_length) {
                        return ListEntries[index];
                    }
                }
            }
        }
        if (rerender) {
            rerender = false;

            push();
            render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            render_box(BufferMain, lx1, ly1, lx2, ly2, EditorPalette_List,
                       DIALOG_BORDER_PALETTE_ID);
            for (int i = 0; i < max_visible_entries && i + view_index < list_length; i++) {
                if (i + view_index == selected_index) {
                    BufferMain->fill_box(lx1 + 1, ly1 + i * dy + 1, lx2 - 1, ly1 + (i + 1) * dy - 1,
                                         EditorPalette_ListSelected);
                }
                Pabc2->write(BufferMain, lx1 + 3, ly1 + 15 + i * dy,
                             ListEntries[i + view_index].c_str());
            }
            render_box(BufferMain, box_up, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            draw_arrow(BufferMain, box_up, DIALOG_BORDER_PALETTE_ID, true);
            render_box(BufferMain, box_down, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            draw_arrow(BufferMain, box_down, DIALOG_BORDER_PALETTE_ID, false);
            render_box(BufferMain, box_cancel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (box_cancel.x1 + box_cancel.x2) / 2,
                                  box_cancel.y1 + 15, "CANCEL");
            if (show_new_button) {
                render_box(BufferMain, box_new, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
                Pabc2->write_centered(BufferMain, (box_new.x1 + box_new.x2) / 2, box_new.y1 + 15,
                                      "NEW");
            }

            bltfront(BufferMain, x1, y1, x2, y2);
            pop();
        }
    }
}

void editor_window_welcome() {
    invalidateegesz();

    std::string filename = editor_window_list_levels(true);

    if (filename.length() > MAX_FILENAME_LEN) {
        internal_error("editor_window_welcome strlen(selected_filename) > MAX_FILENAME_LEN");
    }

    if (filename.length() == 0) {
        State->editor_filename[0] = 0;
    } else {
        strcpy(State->editor_filename, (filename + ".lev").c_str());
    }
}

void editor_window_open() {
    invalidateegesz();

    if (Valtozott) {
        if (dialog("There are unsaved changes in the level file.",
                   "If you open another file, you will loose these changes.",
                   "Do you still want to continue?", DIALOG_BUTTONS, "Yes", "No") == 1) {
            return;
        }
    }

    std::string filename = editor_window_list_levels(false);

    if (ListEntries.size() < 1) {
        dialog("There are no level files (*.lev) in the LEV directory!");
        return;
    }

    if (filename.length() == 0) {
        return;
    }

    open_level(filename.c_str());
}

void editor_new() {
    invalidateegesz();
    if (Valtozott) {
        if (dialog("There are unsaved changes in the level file.", "Do you still want to continue?",
                   DIALOG_BUTTONS, "Yes", "No") == 1) {
            return;
        }
    }

    Valtozott = 0;
    load_level_editor(DEFAULT_LEVEL_FILENAME);
    State->editor_filename[0] = 0;
    zoom_fill();
}

bool editor_window_save() {
    invalidateegesz();
    if (State->editor_filename[0] == 0) {
        return editor_window_save_as();
    }
    Ptop->save(State->editor_filename);
    Valtozott = 0;
    return true;
}

bool editor_window_save_as() {
    invalidateegesz();

    box box_window = {20, 200, 600, 300};
    box box_save = {320 - 30, 273, 320 + 30, 290};

    push();
    render_box(BufferMain, box_window, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
    render_box(BufferMain, box_save, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
    Pabc2->write_centered(
        BufferMain, (box_window.x2 + box_window.x1) / 2, 220,
        "Type in the file name and press ENTER or click on SAVE, or press ESC to cancel!");
    Pabc2->write_centered(BufferMain, (box_save.x2 + box_save.x1) / 2, box_save.y1 + 13, "SAVE");

    bltfront(BufferMain, box_window.x1, box_window.y1, box_window.x2, box_window.y2);
    pop();

    empty_keypress_buffer();
    finame filename_input = "";
    int i = 0;
    char filename_input_prev[10] = "$%...%^&";
    while (true) {
        handle_events();
        if (strcmp(filename_input_prev, filename_input) != 0) {
            strcpy(filename_input_prev, filename_input);

            int ex1 = box_window.x1 + 100;
            int ey1 = box_window.y1 + 30;
            int ex2 = box_window.x2 - 100;
            int ey2 = box_window.y1 + 55;
            BufferMain->fill_box(ex1, ey1, ex2, ey2, DIALOG_PALETTE_ID);
            Pabc2->write(BufferMain, 290, 250, filename_input);
            char tmp[40];
            strcpy(tmp, filename_input);
            tmp[i] = 0;
            Pabc2->write(BufferMain, 290 + Pabc1->len(tmp), 255, "-");
            bltfront(BufferMain, ex1, ey1, ex2, ey2);
        }
        if (was_key_just_pressed(DIK_ESCAPE)) {
            return false;
        }
        if (was_key_just_pressed(DIK_RETURN)) {
            if (filename_input[0] != 0) {
                strcat(filename_input, ".lev");
                char path[40];
                strcpy(path, "lev/");
                strcat(path, filename_input);
                if (access(path, 0) == 0) {
                    if (dialog("File exists, overwrite?", filename_input, DIALOG_BUTTONS, "Yes",
                               "No") == 1) {
                        return false;
                    }
                }
                Ptop->save(filename_input);
                strcpy(State->editor_filename, filename_input);
                Valtozott = 0;
                return true;
            }
        }
        if (was_key_down(DIK_BACK)) {
            if (filename_input[0] && i > 0) {
                int filename_input_length = strlen(filename_input);
                for (int j = i - 1; j < filename_input_length; j++) {
                    filename_input[j] = filename_input[j + 1];
                }
                i--;
            }
        }
        if (was_key_down(DIK_DELETE)) {
            if (!((filename_input[0] == 0) || (i >= (int)strlen(filename_input)))) {
                int filename_input_length = strlen(filename_input);
                for (int j = i; j < filename_input_length; j++) {
                    filename_input[j] = filename_input[j + 1];
                }
            }
        }
        if (was_key_down(DIK_LEFT)) {
            if (i > 0) {
                i--;
                filename_input_prev[0] = 0;
            }
        }
        if (was_key_down(DIK_RIGHT)) {
            if (i < (int)strlen(filename_input)) {
                i++;
                filename_input_prev[0] = 0;
            }
        }
        while (has_text_input()) {
            char c = pop_text_input();
            if (Pabc2->has_char(c) && util::text::is_filename_char(c)) {
                if (strlen(filename_input) < MAX_FILENAME_LEN) {
                    int filename_input_length = strlen(filename_input);
                    for (int j = filename_input_length; j >= i; j--) {
                        filename_input[j + 1] = filename_input[j];
                    }
                    filename_input[i] = c;
                    i++;
                }
            }
        }
        if (was_right_mouse_just_clicked()) {
            return false;
        }
        if (was_left_mouse_just_clicked()) {
            update_and_draw_cursor();
            if (is_in_box(Moux, Mouy, box_save)) {
                if (filename_input[0] != 0) {
                    strcat(filename_input, ".lev");
                    char path[40];
                    strcpy(path, "lev/");
                    strcat(path, filename_input);
                    if (access(path, 0) == 0) {
                        if (dialog("File exists, overwrite?", filename_input, DIALOG_BUTTONS, "Yes",
                                   "No") == 1) {
                            return false;
                        }
                    }
                    Ptop->save(filename_input);
                    strcpy(State->editor_filename, filename_input);
                    Valtozott = 0;
                    return true;
                }
            }
        }
        update_and_draw_cursor();
    }
}

static unsigned char LgrPaletteMap[256];

// Create a map converting from LGR palette id to Editor palette id
// The Editor palette contains a 64-color "true color" RGB222 palette located from 64-127
// in the form 0-63 -> 0x00RRGGBB
static void create_lgr_palette_map(unsigned char* pal) {
    for (int i = 0; i < 256; i++) {
        int r = pal[i * 3];
        int g = pal[i * 3 + 1];
        int b = pal[i * 3 + 2];
        r >>= 6;
        g >>= 6;
        b >>= 6;
        int target_id = r * 16 + g * 4 + b;
        if (target_id < 0 || target_id >= 64) {
            internal_error("create_lgr_palette_map invalid target_id");
        }
        LgrPaletteMap[i] = (unsigned char)(target_id + 64);
    }
}

static void preview_picture(pic8* dest, picture* pict, int x, int y) {
    if (x < 0 || y < 0 || x >= dest->get_width() || y >= dest->get_height()) {
        internal_error("preview_picture out of bounds");
    }

    // Trim width and height to be in-bounds
    int width = std::min(pict->width, dest->get_width() - x);
    int height = std::min(pict->height, dest->get_height() - y);

    // Draw picture
    int data_offset = 0;
    unsigned char* data = pict->data;
    for (int i = y; i < y + height; i++) {
        unsigned char* row = dest->get_row(i) + x;
        int j = 0;
        while (true) {
            int skip = read_varint(data, data_offset);
            if (skip == -1) {
                break;
            }
            j += skip;

            int count = read_varint(data, data_offset);
            for (int j_offset = 0; j_offset < count; j_offset++) {
                if (j + j_offset < width) {
                    row[j + j_offset] = LgrPaletteMap[data[data_offset + j_offset]];
                }
            }

            j += count;
            data_offset += count;
        }
    }
}

static void preview_texture(pic8* dest, texture* text, mask* msk, int x, int y) {
    if (x < 0 || y < 0 || x >= dest->get_width() || y >= dest->get_height()) {
        internal_error("preview_texture out of bounds");
    }

    pic8* source = text->pic;

    // Trim height to be in-bounds
    int height = msk->height;
    if (y + height > dest->get_height()) {
        height = dest->get_height() - y;
    }

    // Draw texture/mask
    int data_offset = 0;
    int texture_width = source->get_width();
    for (int i = y; i < y + height; i++) {
        unsigned char* dest_row = dest->get_row(i) + x;
        unsigned char* src_row = source->get_row((i - y) % source->get_height());
        int src_x = 0;
        int dest_x = x;
        while (msk->data[data_offset].type != MaskEncoding::EndOfLine) {
            if (msk->data[data_offset].type == MaskEncoding::Solid) {
                for (int j = 0; j < msk->data[data_offset].length; j++) {
                    // Trim width to be in-bounds, assuming screen size is exactly 640
                    // Crash here if screen is too small
                    if (dest_x < 640) {
                        *dest_row = LgrPaletteMap[src_row[src_x]];
                    }
                    dest_row++;
                    src_x++;
                    src_x %= texture_width;
                    dest_x++;
                }
            } else {
                dest_row += msk->data[data_offset].length;
                src_x += msk->data[data_offset].length;
                src_x %= texture_width;
                dest_x += msk->data[data_offset].length;
            }
            data_offset++;
        }
        data_offset++;
    }
}

enum class SpriteType { Picture, Texture, Mask };

static void editor_window_select_sprite_name(char* picture_name, char* texture_name,
                                             char* mask_name, SpriteType sprite_type) {
    create_lgr_palette_map(Lgr->palette_data);

    char* name = nullptr;
    int list_length = 0;
    if (sprite_type == SpriteType::Picture) {
        name = picture_name;
        list_length = Lgr->picture_count;
    }
    if (sprite_type == SpriteType::Texture) {
        name = texture_name;
        list_length = Lgr->texture_count;
    }
    if (sprite_type == SpriteType::Mask) {
        name = mask_name;
        list_length = Lgr->mask_count;
    }

    if (!name) {
        internal_error("editor_window_select_sprite_name !name");
    }

    if (list_length < 1) {
        return;
    }

    char original_name[10];
    if (strlen(name) > 8) {
        internal_error("editor_window_select_sprite_name strlen(name) > 8");
    }
    strcpy(original_name, name);

    int selected_index = 0;
    for (int i = 0; i < list_length; i++) {
        if (sprite_type == SpriteType::Picture && strcmpi(Lgr->pictures[i].name, name) == 0) {
            selected_index = i;
        }
        if (sprite_type == SpriteType::Texture && strcmpi(Lgr->textures[i].name, name) == 0) {
            selected_index = i;
        }
        if (sprite_type == SpriteType::Mask && strcmpi(Lgr->masks[i].name, name) == 0) {
            selected_index = i;
        }
    }

    if (selected_index == 0) {
        if (sprite_type == SpriteType::Picture) {
            strcpy(name, Lgr->pictures[0].name);
        }
        if (sprite_type == SpriteType::Texture) {
            strcpy(name, Lgr->textures[0].name);
        }
        if (sprite_type == SpriteType::Mask) {
            strcpy(name, Lgr->masks[0].name);
        }
    }

    int max_visible_entries = 10;
    int dy = 20;
    int x1 = 200;
    int y1 = 100;
    int x2 = 401;
    int y2 = 174 + max_visible_entries * dy;
    int lx1 = x1 + 10;
    int ly1 = y1 + 37;
    int lx2 = lx1 + 100;
    int ly2 = ly1 + max_visible_entries * dy;

    box box_list = {lx1, ly1, lx2, ly2};
    box box_up = {x1 + 10, y1 + 11, x1 + 110, y1 + 31};
    box box_down = {x1 + 10, y2 - 30, x1 + 110, y2 - 10};
    box box_cancel = {x1 + 121, (y2 + y1) / 2 - 10, x1 + 121 + 70, (y2 + y1) / 2 + 10};

    int view_index = 0;
    bool rerender = true;
    empty_keypress_buffer();

    // Keep track of the box where we are drawing, so we can restore the background later
    int preview_x2 = 0;
    int preview_y2 = 0;
    push();
    blit8(BufferBall, BufferMain);
    pop();

    while (true) {
        handle_events();
        update_and_draw_cursor();
        adjust_list_view(selected_index, view_index, list_length, max_visible_entries, rerender,
                         box_up, box_down, box_list);
        if (was_key_just_pressed(DIK_ESCAPE)) {
            strcpy(name, original_name);
            return;
        }
        if (was_key_just_pressed(DIK_RETURN)) {
            if (sprite_type == SpriteType::Picture) {
                strcpy(name, Lgr->pictures[selected_index].name);
            }
            if (sprite_type == SpriteType::Texture) {
                strcpy(name, Lgr->textures[selected_index].name);
            }
            if (sprite_type == SpriteType::Mask) {
                strcpy(name, Lgr->masks[selected_index].name);
            }
            return;
        }
        if (was_left_mouse_just_clicked()) {
            if (is_in_box(Moux, Mouy, box_cancel)) {
                strcpy(name, original_name);
                return;
            }
            if (Moux > lx1 && Moux < lx2 && Mouy > ly1 && Mouy < ly2) {
                if (Mouy < ly1 + dy * max_visible_entries) {
                    int index = (Mouy - ly1) / dy;
                    index += view_index;
                    if (index < list_length) {
                        if (sprite_type == SpriteType::Picture) {
                            strcpy(name, Lgr->pictures[index].name);
                        }
                        if (sprite_type == SpriteType::Texture) {
                            strcpy(name, Lgr->textures[index].name);
                        }
                        if (sprite_type == SpriteType::Mask) {
                            strcpy(name, Lgr->masks[index].name);
                        }
                        return;
                    }
                }
            }
        }
        if (rerender) {
            rerender = false;

            if (sprite_type == SpriteType::Picture) {
                strcpy(name, Lgr->pictures[selected_index].name);
            }
            if (sprite_type == SpriteType::Texture) {
                strcpy(name, Lgr->textures[selected_index].name);
            }
            if (sprite_type == SpriteType::Mask) {
                strcpy(name, Lgr->masks[selected_index].name);
            }

            // Restore background image
            push();
            blit8(BufferMain, BufferBall, 0, 0, 0, 0, preview_x2, preview_y2);

            // Draw new sprite
            if (picture_name[0] && (texture_name[0] || mask_name[0])) {
                internal_error("editor_window_select_sprite_name too many names");
            }
            if (picture_name[0]) {
                int index = Lgr->get_picture_index(picture_name);
                if (index < 0) {
                    internal_error("editor_window_select_sprite_name picture_index < 0");
                }
                picture* pict = &Lgr->pictures[index];
                preview_picture(BufferMain, pict, 10, 10);
                preview_x2 = 10 + pict->width - 1;
                preview_y2 = 10 + pict->height - 1;
            }
            if (texture_name[0] && mask_name[0]) {
                int index = Lgr->get_texture_index(texture_name);
                if (index < 0) {
                    internal_error("editor_window_select_sprite_name texture index < 0");
                }
                texture* text = &Lgr->textures[index];

                index = Lgr->get_mask_index(mask_name);
                if (index < 0) {
                    internal_error("editor_window_select_sprite_name mask index < 0");
                }
                mask* msk = &Lgr->masks[index];

                preview_texture(BufferMain, text, msk, 10, 10);
                preview_x2 = 10 + msk->width - 1;
                preview_y2 = 10 + msk->height - 1;
            }

            // Draw the list of sprites
            render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            render_box(BufferMain, lx1, ly1, lx2, ly2, EditorPalette_List,
                       DIALOG_BORDER_PALETTE_ID);
            for (int i = 0; i < max_visible_entries && i + view_index < list_length; i++) {
                if (i + view_index == selected_index) {
                    BufferMain->fill_box(lx1 + 1, ly1 + i * dy + 1, lx2 - 1, ly1 + (i + 1) * dy - 1,
                                         EditorPalette_ListSelected);
                }
                if (sprite_type == SpriteType::Picture) {
                    Pabc2->write(BufferMain, lx1 + 3, ly1 + 15 + i * dy,
                                 Lgr->pictures[i + view_index].name);
                }
                if (sprite_type == SpriteType::Texture) {
                    Pabc2->write(BufferMain, lx1 + 3, ly1 + 15 + i * dy,
                                 Lgr->textures[i + view_index].name);
                }
                if (sprite_type == SpriteType::Mask) {
                    Pabc2->write(BufferMain, lx1 + 3, ly1 + 15 + i * dy,
                                 Lgr->masks[i + view_index].name);
                }

                if (picture_name[0] || texture_name[0]) {
                    int default_distance = 0;
                    if (picture_name[0]) {
                        default_distance = Lgr->pictures[i + view_index].default_distance;
                    } else {
                        default_distance = Lgr->textures[i + view_index].default_distance;
                    }

                    char details[20];
                    sprintf(details, "%d", default_distance);
                    Pabc2->write(BufferMain, lx1 + 67, ly1 + 15 + i * dy, details);
                    Clipping default_clipping = Clipping::Unclipped;
                    if (picture_name[0]) {
                        default_clipping = Lgr->pictures[i + view_index].default_clipping;
                    } else {
                        default_clipping = Lgr->textures[i + view_index].default_clipping;
                    }

                    strcpy(details, "U");
                    if (default_clipping == Clipping::Ground) {
                        strcpy(details, "G");
                    }
                    if (default_clipping == Clipping::Sky) {
                        strcpy(details, "S");
                    }
                    Pabc2->write(BufferMain, lx1 + 92, ly1 + 15 + i * dy, details);
                }
            }

            // Draw other buttons
            render_box(BufferMain, box_up, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            draw_arrow(BufferMain, box_up, DIALOG_BORDER_PALETTE_ID, true);
            render_box(BufferMain, box_down, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            draw_arrow(BufferMain, box_down, DIALOG_BORDER_PALETTE_ID, false);
            render_box(BufferMain, box_cancel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (box_cancel.x1 + box_cancel.x2) / 2,
                                  box_cancel.y1 + 15, "CANCEL");

            bltfront(BufferMain);
            pop();
        }
    }
}

void editor_window_choose_sprite() {
    if (!Lgr) {
        internal_error("editor_window_choose_sprite !Lgr!");
    }

    invalidateegesz();

    char picture_name[10], texture_name[10], mask_name[10], null_name[10] = "";
    strcpy(picture_name, Lgr->editor_picture_name);
    strcpy(texture_name, Lgr->editor_texture_name);
    strcpy(mask_name, Lgr->editor_mask_name);

    int x1 = 160;
    int y1 = 100;
    int x2 = 390;
    int y2 = y1 + 185;

    int text_x1 = x1 + 120;
    box box_picture = {text_x1, y1 + 40, text_x1 + 100, y1 + 40 + 20};
    box box_texture = {text_x1, y1 + 70, text_x1 + 100, y1 + 70 + 20};
    box box_mask = {text_x1, y1 + 100, text_x1 + 100, y1 + 100 + 20};

    box box_ok = {x1 + 50, y2 - 30, x1 + 100, y2 - 10};
    box box_cancel = {x1 + 130, y2 - 30, x1 + 180, y2 - 10};

    empty_keypress_buffer();
    bool rerender = true;
    while (true) {
        handle_events();
        if (was_key_just_pressed(DIK_ESCAPE)) {
            return;
        }
        if (was_key_just_pressed(DIK_RETURN)) {
            strcpy(Lgr->editor_picture_name, picture_name);
            strcpy(Lgr->editor_texture_name, texture_name);
            strcpy(Lgr->editor_mask_name, mask_name);
            return;
        }
        if (was_left_mouse_just_clicked()) {
            update_and_draw_cursor();
            if (is_in_box(Moux, Mouy, box_ok)) {
                strcpy(Lgr->editor_picture_name, picture_name);
                strcpy(Lgr->editor_texture_name, texture_name);
                strcpy(Lgr->editor_mask_name, mask_name);
                return;
            }
            if (is_in_box(Moux, Mouy, box_cancel)) {
                return;
            }
            if (is_in_box(Moux, Mouy, box_picture)) {
                editor_window_select_sprite_name(picture_name, null_name, null_name,
                                                 SpriteType::Picture);
                push();
                blit8(BufferMain, BufferBall);
                bltfront(BufferMain);
                pop();
                if (picture_name[0]) {
                    texture_name[0] = mask_name[0] = 0;
                }
                rerender = true;
            }
            if (is_in_box(Moux, Mouy, box_texture)) {
                editor_window_select_sprite_name(null_name, texture_name, mask_name,
                                                 SpriteType::Texture);
                push();
                blit8(BufferMain, BufferBall);
                bltfront(BufferMain);
                pop();
                if (texture_name[0]) {
                    picture_name[0] = 0;
                }
                rerender = true;
            }
            if (is_in_box(Moux, Mouy, box_mask)) {
                editor_window_select_sprite_name(null_name, texture_name, mask_name,
                                                 SpriteType::Mask);
                push();
                blit8(BufferMain, BufferBall);
                bltfront(BufferMain);
                pop();
                if (mask_name[0]) {
                    picture_name[0] = 0;
                }
                rerender = true;
            }
        }
        if (rerender) {
            rerender = false;

            push();

            render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);

            Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y1 + 20, "Choose picture");

            render_box(BufferMain, box_ok, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            render_box(BufferMain, box_cancel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (box_ok.x1 + box_ok.x2) / 2, box_ok.y1 + 15, "OK");
            Pabc2->write_centered(BufferMain, (box_cancel.x1 + box_cancel.x2) / 2,
                                  box_cancel.y1 + 15, "CANCEL");

            int label_x1 = x1 + 8;
            Pabc2->write(BufferMain, label_x1, box_picture.y1 + 15, "Normal Picture:");
            render_box(BufferMain, box_picture, Feherszin, DIALOG_BORDER_PALETTE_ID);
            draw_textbox_left(BufferMain, box_picture, Feherszin, picture_name);

            Pabc2->write(BufferMain, label_x1, box_texture.y1 + 15, "Texture:");
            render_box(BufferMain, box_texture, Feherszin, DIALOG_BORDER_PALETTE_ID);
            draw_textbox_left(BufferMain, box_texture, Feherszin, texture_name);

            Pabc2->write(BufferMain, label_x1, box_mask.y1 + 15, "Mask:");
            render_box(BufferMain, box_mask, Feherszin, DIALOG_BORDER_PALETTE_ID);
            draw_textbox_left(BufferMain, box_mask, Feherszin, mask_name);

            bltfront(BufferMain);
            pop();
        }
        update_and_draw_cursor();
    }
}

bool editor_window_choose_lgr(char* lgrname) {
    invalidateegesz();

    blit8(BufferBall, BufferMain);

    int list_length = populate_list("lgr/*.lgr", MAX_FILENAME_LEN);

    if (list_length < 1) {
        external_error("There are no LGR files (*.lgr) in the LGR directory!");
    }

    int selected_index = list_index(lgrname);

    // Display menu
    int max_visible_entries = 10;
    int dy = 20;
    int x1 = 200;
    int y1 = 100;
    int x2 = 401;
    int bottom_margin = 40;
    int y2 = 174 + max_visible_entries * dy + bottom_margin;
    int lx1 = x1 + 10;
    int ly1 = y1 + 37;
    int lx2 = lx1 + 100;
    int ly2 = ly1 + max_visible_entries * dy;

    box box_list = {lx1, ly1, lx2, ly2};
    box box_up = {x1 + 10, y1 + 11, x1 + 110, y1 + 31};
    box box_down = {x1 + 10, y2 - 30 - bottom_margin, x1 + 110, y2 - 10 - bottom_margin};
    box box_cancel = {x1 + 121, (y2 + y1) / 2 - 10, x1 + 121 + 70, (y2 + y1) / 2 + 10};

    int view_index = 0;
    bool rerender = true;
    empty_keypress_buffer();
    while (true) {
        handle_events();
        update_and_draw_cursor();
        adjust_list_view(selected_index, view_index, list_length, max_visible_entries, rerender,
                         box_up, box_down, box_list);
        if (was_key_just_pressed(DIK_ESCAPE)) {
            return false;
        }
        if (was_key_just_pressed(DIK_RETURN)) {
            strcpy(lgrname, ListEntries[selected_index].c_str());
            return true;
        }
        if (was_left_mouse_just_clicked()) {
            if (is_in_box(Moux, Mouy, box_cancel)) {
                return false;
            }
            if (Moux > lx1 && Moux < lx2 && Mouy > ly1 && Mouy < ly2) {
                if (Mouy < ly1 + dy * max_visible_entries) {
                    int index = (Mouy - ly1) / dy;
                    index += view_index;
                    if (index < list_length) {
                        strcpy(lgrname, ListEntries[index].c_str());
                        return true;
                    }
                }
            }
        }
        if (rerender) {
            rerender = false;

            push();
            render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            render_box(BufferMain, lx1, ly1, lx2, ly2, EditorPalette_List,
                       DIALOG_BORDER_PALETTE_ID);
            for (int i = 0; i < max_visible_entries && i + view_index < list_length; i++) {
                if (i + view_index == selected_index) {
                    BufferMain->fill_box(lx1 + 1, ly1 + i * dy + 1, lx2 - 1, ly1 + (i + 1) * dy - 1,
                                         EditorPalette_ListSelected);
                }
                Pabc2->write(BufferMain, lx1 + 3, ly1 + 15 + i * dy,
                             ListEntries[i + view_index].c_str());
            }
            render_box(BufferMain, box_up, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            draw_arrow(BufferMain, box_up, DIALOG_BORDER_PALETTE_ID, true);
            render_box(BufferMain, box_down, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            draw_arrow(BufferMain, box_down, DIALOG_BORDER_PALETTE_ID, false);
            render_box(BufferMain, box_cancel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (box_cancel.x1 + box_cancel.x2) / 2,
                                  box_cancel.y1 + 15, "CANCEL");

            Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y2 - 36, "Original LGR file:");
            Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y2 - 18, lgrname);

            bltfront(BufferMain, x1, y1, x2, y2);
            pop();
        }
    }
}

static int prompt_distance(pic8* pic, box bx, int distance) {
    push();

    empty_keypress_buffer();
    bool rerender = true;
    int distance_prev = distance;
    while (true) {
        handle_events();
        if (rerender) {
            rerender = false;
            char tmp[10];
            sprintf(tmp, "%d", distance);
            draw_textbox_centered(pic, bx, Feherszin, tmp, true);
            bltfront(pic, bx.x1, bx.y1, bx.x2, bx.y2);
        }
        if (was_key_just_pressed(DIK_ESCAPE)) {
            pop();
            return distance_prev;
        }
        if (was_key_just_pressed(DIK_RETURN)) {
            pop();
            return distance;
        }
        if (was_key_down(DIK_BACK)) {
            if (distance > 0) {
                distance /= 10;
                rerender = true;
            }
        }
        while (has_text_input()) {
            char c = pop_text_input();
            if (c >= '0' && c <= '9') {
                if (distance < 100) {
                    distance *= 10;
                    distance += c - '0';
                    rerender = true;
                }
            }
        }
    }
}

static Clipping prompt_clipping(pic8* pic, box bx, Clipping clipping) {
    push();
    draw_textbox_centered(pic, bx, Feherszin, "", true);
    bltfront(pic, bx.x1, bx.y1, bx.x2, bx.y2);
    empty_keypress_buffer();
    while (true) {
        handle_events();
        if (was_key_just_pressed(DIK_ESCAPE)) {
            pop();
            return clipping;
        }
        while (has_text_input()) {
            char c = pop_text_input();
            if (c == 'u' || c == 'U') {
                pop();
                return Clipping::Unclipped;
            }
            if (c == 'g' || c == 'G') {
                pop();
                return Clipping::Ground;
            }
            if (c == 's' || c == 'S') {
                pop();
                return Clipping::Sky;
            }
        }
    }
}

void editor_window_sprite_properties(sprite* spr) {
    invalidateegesz();

    int x1 = 200;
    int y1 = 100;
    int x2 = 442;
    int y2 = 300;

    box box_ok = {x1 + 32, y2 - 30, x1 + 32 + 70, y2 - 10};
    box box_cancel = {x1 + 135, y2 - 30, x1 + 135 + 70, y2 - 10};
    box box_distance = {x1 + 102, y1 + 118, x1 + 102 + 34, y1 + 118 + 16};
    box box_clipping = {x1 + 181, y1 + 118, x1 + 181 + 34, y1 + 118 + 16};

    bool rerender = true;
    int distance = spr->distance;
    Clipping clipping = spr->clipping;
    while (true) {
        handle_events();
        if (was_key_just_pressed(DIK_ESCAPE)) {
            return;
        }
        if (was_key_just_pressed(DIK_RETURN)) {
            if (spr->distance != distance || spr->clipping != clipping) {
                Valtozott = 1;
            }
            spr->distance = distance;
            spr->clipping = clipping;
            return;
        }
        if (was_left_mouse_just_clicked()) {
            update_and_draw_cursor();
            if (is_in_box(Moux, Mouy, box_ok)) {
                if (spr->distance != distance || spr->clipping != clipping) {
                    Valtozott = 1;
                }
                spr->distance = distance;
                spr->clipping = clipping;
                return;
            }
            if (is_in_box(Moux, Mouy, box_cancel)) {
                return;
            }
            if (is_in_box(Moux, Mouy, box_distance)) {
                distance = prompt_distance(BufferMain, box_distance, distance);
                rerender = true;
            }
            if (is_in_box(Moux, Mouy, box_clipping)) {
                clipping = prompt_clipping(BufferMain, box_clipping, clipping);
                rerender = true;
            }
        }
        if (rerender) {
            rerender = false;
            push();
            render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            render_box(BufferMain, box_ok, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            render_box(BufferMain, box_cancel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (box_ok.x1 + box_ok.x2) / 2, box_ok.y1 + 15, "OK");
            Pabc2->write_centered(BufferMain, (box_cancel.x1 + box_cancel.x2) / 2,
                                  box_cancel.y1 + 15, "CANCEL");

            Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y1 + 15, "Set Picture Properties");
            if (spr->picture_name[0]) {
                Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y1 + 45, spr->picture_name);
            } else {
                Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y1 + 34, spr->texture_name);
                Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y1 + 56, spr->mask_name);
            }
            Pabc2->write(BufferMain, x1 + 92, y1 + 76, "Distance");
            Pabc2->write(BufferMain, x1 + 172, y1 + 76, "Clipping");

            // Default values
            Pabc2->write(BufferMain, x1 + 12, y1 + 100, "Default:");
            int default_distance = -1;
            Clipping default_clipping = Clipping::Unknown;
            if (spr->picture_name[0]) {
                int index = Lgr->get_picture_index(spr->picture_name);
                if (index < 0) {
                    internal_error("editor_window_sprite_properties index < 0");
                }
                default_distance = Lgr->pictures[index].default_distance;
                default_clipping = Lgr->pictures[index].default_clipping;
            }
            if (spr->texture_name[0]) {
                int index = Lgr->get_texture_index(spr->texture_name);
                if (index < 0) {
                    internal_error("editor_window_sprite_properties index < 0");
                }
                default_distance = Lgr->textures[index].default_distance;
                default_clipping = Lgr->textures[index].default_clipping;
            }

            char tmp[10];
            if (default_distance >= 0) {
                sprintf(tmp, "%d", default_distance);
            } else {
                sprintf(tmp, "-");
            }
            Pabc2->write_centered(BufferMain, x1 + 119, y1 + 100, tmp);

            if (default_clipping == Clipping::Unknown) {
                strcpy(tmp, "-");
            }
            if (default_clipping == Clipping::Unclipped) {
                strcpy(tmp, "U");
            }
            if (default_clipping == Clipping::Ground) {
                strcpy(tmp, "G");
            }
            if (default_clipping == Clipping::Sky) {
                strcpy(tmp, "S");
            }
            Pabc2->write_centered(BufferMain, x1 + 198, y1 + 100, tmp);

            // Actual values
            Pabc2->write(BufferMain, x1 + 12, y1 + 130, "Current:");

            render_box(BufferMain, box_distance, Feherszin, DIALOG_BORDER_PALETTE_ID);
            sprintf(tmp, "%d", distance);
            draw_textbox_centered(BufferMain, box_distance, Feherszin, tmp);
            Pabc2->write_centered(BufferMain, x1 + 119, box_distance.y2 + 14, "(1-999)");

            render_box(BufferMain, box_clipping, Feherszin, DIALOG_BORDER_PALETTE_ID);
            if (clipping == Clipping::Unclipped) {
                strcpy(tmp, "U");
            }
            if (clipping == Clipping::Ground) {
                strcpy(tmp, "G");
            }
            if (clipping == Clipping::Sky) {
                strcpy(tmp, "S");
            }
            draw_textbox_centered(BufferMain, box_clipping, Feherszin, tmp);
            Pabc2->write_centered(BufferMain, x1 + 198, box_clipping.y2 + 14, "(U, S, G)");

            bltfront(BufferMain, x1, y1, x2, y2);
            pop();
        }
        update_and_draw_cursor();
    }
}

void editor_window_polygon_properties(polygon* poly) {
    invalidateegesz();

    int x1 = 200;
    int y1 = 100;
    int x2 = 384;
    int y2 = 200;

    box box_ok = {x1 + 12, y2 - 30, x1 + 12 + 70, y2 - 10};
    box box_cancel = {x1 + 103, y2 - 30, x1 + 103 + 70, y2 - 10};

    // Only existing property for now is grass
    int box_left = x1 + 132;
    box box_grass = {box_left, y1 + 35, box_left + 30, y1 + 55};

    bool rerender = true;

    int is_grass = poly->is_grass;

    while (true) {
        handle_events();
        if (was_key_just_pressed(DIK_ESCAPE)) {
            return;
        }
        if (was_key_just_pressed(DIK_RETURN)) {
            if (poly->is_grass != is_grass) {
                Valtozott = 1;
            }
            poly->is_grass = is_grass;
            return;
        }
        if (was_left_mouse_just_clicked()) {
            update_and_draw_cursor();
            if (is_in_box(Moux, Mouy, box_ok)) {
                if (poly->is_grass != is_grass) {
                    Valtozott = 1;
                }
                poly->is_grass = is_grass;
                return;
            }
            if (is_in_box(Moux, Mouy, box_cancel)) {
                return;
            }
            if (is_in_box(Moux, Mouy, box_grass)) {
                is_grass = !is_grass;
                rerender = true;
            }
        }
        if (rerender) {
            rerender = false;

            push();
            render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);

            Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y1 + 15, "Set Polygon Properties");

            int label_x1 = x1 + 18;
            Pabc2->write(BufferMain, label_x1, box_grass.y1 + 15, "Grass polygon:");
            render_box(BufferMain, box_grass, Feherszin, DIALOG_BORDER_PALETTE_ID);
            if (is_grass) {
                Pabc2->write_centered(BufferMain, (box_grass.x1 + box_grass.x2) / 2,
                                      box_grass.y1 + 15, "YES");
            } else {
                Pabc2->write_centered(BufferMain, (box_grass.x1 + box_grass.x2) / 2,
                                      box_grass.y1 + 15, "NO");
            }

            render_box(BufferMain, box_ok, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            render_box(BufferMain, box_cancel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (box_ok.x1 + box_ok.x2) / 2, box_ok.y1 + 15, "OK");
            Pabc2->write_centered(BufferMain, (box_cancel.x1 + box_cancel.x2) / 2,
                                  box_cancel.y1 + 15, "CANCEL");

            bltfront(BufferMain, x1, y1, x2, y2);
            pop();
        }
        update_and_draw_cursor();
    }
}

void editor_window_food_properties(const char* title, object::Property* property, int* animation) {
    invalidateegesz();

    int list_length = 5;
    int dy = 20;

    int x1 = 200;
    int y1 = 100;
    int x2 = 442;
    int y2 = y1 + list_length * dy + 120;

    box box_cancel = {(x1 + x2) / 2 - 50, y2 - 30, (x1 + x2) / 2 + 50, y2 - 10};
    box box_animation = {x2 - 50, y2 - 56, x2 - 50 + 20, y2 - 56 + 20};
    int ly1 = y1 + 38;
    int ly2 = ly1 + dy * list_length - 1;
    // Position of the first box only:
    box box_list = {(x1 + x2) / 2 - 70, ly1, (x1 + x2) / 2 + 70, ly1 + dy};

    push();
    render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
    Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y1 + 20, title);
    render_box(BufferMain, box_cancel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
    Pabc2->write_centered(BufferMain, (box_cancel.x1 + box_cancel.x2) / 2, box_cancel.y1 + 15,
                          "Cancel");

    Pabc2->write(BufferMain, box_animation.x1 - 162, box_animation.y1 + 15,
                 "Food anim number (1-9):");
    char tmp[20];
    sprintf(tmp, "%d", (int)(*animation + 1));
    render_box(BufferMain, box_animation, Feherszin, DIALOG_BORDER_PALETTE_ID);
    Pabc2->write_centered(BufferMain, (box_animation.x1 + box_animation.x2) / 2,
                          box_animation.y1 + 15, tmp);

    for (int i = 0; i < list_length; i++) {
        render_box(BufferMain, box_list, EditorPalette_List, DIALOG_BORDER_PALETTE_ID);
        int lx = (box_list.x1 + box_list.x2) / 2;
        int ly = box_list.y1 + 15;
        switch (i) {
        case 0:
            Pabc2->write_centered(BufferMain, lx, ly, "Normal Food");
            break;
        case 1:
            Pabc2->write_centered(BufferMain, lx, ly, "Gravity Up");
            break;
        case 2:
            Pabc2->write_centered(BufferMain, lx, ly, "Gravity Down");
            break;
        case 3:
            Pabc2->write_centered(BufferMain, lx, ly, "Gravity Left");
            break;
        case 4:
            Pabc2->write_centered(BufferMain, lx, ly, "Gravity Right");
            break;
        default:
            internal_error("yuiwegfyjkweg");
        }
        // Update box position to the next box
        box_list.y1 += dy;
        box_list.y2 += dy;
    }
    // Update box position to cover all subboxes
    box_list.y1 = ly1;
    box_list.y2 = ly2;

    bltfront(BufferMain, x1, y1, x2, y2);
    pop();

    while (true) {
        handle_events();
        if (was_key_just_pressed(DIK_ESCAPE)) {
            return;
        }
        if (was_left_mouse_just_clicked()) {
            update_and_draw_cursor();
            if (is_in_box(Moux, Mouy, box_cancel)) {
                return;
            }
            if (is_in_box(Moux, Mouy, box_animation)) {
                push();
                render_box(BufferMain, box_animation, Feherszin, DIALOG_BORDER_PALETTE_ID);
                Pabc2->write_centered(BufferMain, (box_animation.x1 + box_animation.x2) / 2,
                                      box_animation.y1 + 18, "-");
                bltfront(BufferMain, x1, y1, x2, y2);

                empty_keypress_buffer();
                while (true) {
                    handle_events();
                    if (was_key_just_pressed(DIK_ESCAPE)) {
                        pop();
                        return;
                    }
                    while (has_text_input()) {
                        char c = pop_text_input();
                        if (c >= '1' && c <= '9') {
                            *animation = c - '1';
                            pop();
                            return;
                        }
                    }
                }
            }
            if (is_in_box(Moux, Mouy, box_list)) {
                int index = (Mouy - box_list.y1) / dy;
                if (index >= list_length) {
                    index = list_length - 1;
                }
                Valtozott = 1;
                switch (index) {
                case 0:
                    *property = object::Property::None;
                    break;
                case 1:
                    *property = object::Property::GravityUp;
                    break;
                case 2:
                    *property = object::Property::GravityDown;
                    break;
                case 3:
                    *property = object::Property::GravityLeft;
                    break;
                case 4:
                    *property = object::Property::GravityRight;
                    break;
                default:
                    internal_error("editor_window_food_properties invalid property");
                }
                return;
            }
        }
        update_and_draw_cursor();
    }
}

static void editor_window_level_name(char* level_name) {
    char orig_level_name[LEVEL_NAME_LENGTH + 10];
    strcpy(orig_level_name, level_name);

    push();

    blit8(BufferBall, BufferMain);

    invalidateegesz();

    int x1 = 50;
    int y1 = 200;
    int x2 = 600;
    int y2 = 280;

    empty_keypress_buffer();
    int i = 0;
    while (true) {
        handle_events();

        BufferMain->fill_box(x1, y1, x2, y2, EditorPalette_LevelNameDialog);
        BufferMain->line(x1, y1, x2, y1, EditorPalette_LevelNameDialogBorder);
        BufferMain->line(x1, y2, x2, y2, EditorPalette_LevelNameDialogBorder);
        BufferMain->line(x1, y1, x1, y2, EditorPalette_LevelNameDialogBorder);
        BufferMain->line(x2, y1, x2, y2, EditorPalette_LevelNameDialogBorder);
        Pabc2->write_centered(
            BufferMain, (x2 + x1) / 2, 220,
            "Type in the name of the level and the designer and press ENTER (ESC to cancel)!");
        int x = (x2 + x1) / 2 - Pabc1->len(level_name) / 2;
        Pabc2->write(BufferMain, x, 250, level_name);
        char tmp[LEVEL_NAME_LENGTH + 10];
        strcpy(tmp, level_name);
        tmp[i] = 0;
        Pabc2->write(BufferMain, x + Pabc1->len(tmp), 255, "-");
        bltfront(BufferMain, x1, y1, x2, y2);

        if (was_key_just_pressed(DIK_ESCAPE)) {
            strcpy(level_name, orig_level_name);
            pop();
            return;
        }
        if (was_key_just_pressed(DIK_RETURN)) {
            if (level_name[0] != 0) {
                pop();
                return;
            }
        }
        if (was_key_down(DIK_BACK)) {
            if (level_name[0] != 0 && i > 0) {
                int level_name_length = strlen(level_name);
                for (int j = i - 1; j < level_name_length; j++) {
                    level_name[j] = level_name[j + 1];
                }
                i--;
            }
        }
        if (was_key_down(DIK_DELETE)) {
            if (level_name[0] != 0 && i < (int)strlen(level_name)) {
                int level_name_length = strlen(level_name);
                for (int j = i; j < level_name_length; j++) {
                    level_name[j] = level_name[j + 1];
                }
            }
        }
        if (was_key_down(DIK_LEFT)) {
            if (i > 0) {
                i--;
            }
        }
        if (was_key_down(DIK_RIGHT)) {
            if (i < (int)strlen(level_name)) {
                i++;
            }
        }
        while (has_text_input()) {
            char c = pop_text_input();
            if (Pabc2->has_char(c) && util::text::is_filename_char(c)) {
                char tmp[LEVEL_NAME_LENGTH + 10];
                sprintf(tmp, "Ext: %s", level_name);

                if (strlen(level_name) < LEVEL_NAME_LENGTH) {
                    int level_name_length = strlen(level_name);
                    for (int j = level_name_length; j >= i; j--) {
                        level_name[j + 1] = level_name[j];
                    }
                    level_name[i] = c;
                    i++;
                }
            }
        }
    }
}

void editor_window_level_properties() {
    invalidateegesz();

    char foreground_name[10];
    char background_name[10];
    char level_name[LEVEL_NAME_LENGTH + 20];
    strcpy(foreground_name, Ptop->foreground_name);
    strcpy(background_name, Ptop->background_name);
    strcpy(level_name, Ptop->level_name);

    finame lgr_name = "";
    strcpy(lgr_name, Ptop->lgr_name);

    int x1 = 130;
    int y1 = 100;
    int x2 = 570;
    int y2 = 280;

    int okcancel_spacing = 10;
    int okcancel_width = 70;
    int x_middle = (x1 + x2) / 2;
    box box_ok = {x_middle - okcancel_spacing - okcancel_width, y2 - 30,
                  x_middle - okcancel_spacing, y2 - 10};
    box box_cancel = {x_middle + okcancel_spacing, y2 - 30,
                      x_middle + okcancel_spacing + okcancel_width, y2 - 10};

    int box_left = x1 + 100;
    box box_foreground = {box_left, y1 + 38, box_left + 80, y1 + 58};
    box box_background = {box_left, y1 + 60, box_left + 80, y1 + 80};
    box box_levelname = {box_left, y1 + 85, box_left + 320, y1 + 105};
    box box_lgr = {box_left, y1 + 110, box_left + 80, y1 + 130};

    bool rerender = true;
    while (true) {
        handle_events();
        if (was_key_just_pressed(DIK_ESCAPE)) {
            return;
        }
        if (was_key_just_pressed(DIK_RETURN)) {
            if (strcmpi(Ptop->foreground_name, foreground_name) != 0 ||
                strcmpi(Ptop->background_name, background_name) != 0 ||
                strcmp(Ptop->level_name, level_name) != 0) {
                Valtozott = 1;
            }
            strcpy(Ptop->foreground_name, foreground_name);
            strcpy(Ptop->background_name, background_name);
            strcpy(Ptop->level_name, level_name);

            if (strcmpi(lgr_name, Ptop->lgr_name) != 0) {
                Valtozott = 1;
                strcpy(Ptop->lgr_name, lgr_name);
                lgrfile::load_lgr_file(Ptop->lgr_name);
                if (Ptop->discard_missing_lgr_assets(Lgr)) {
                    dialog_warn_lgr_assets_deleted();
                }
            }
            return;
        }
        if (was_left_mouse_just_clicked()) {
            update_and_draw_cursor();
            if (is_in_box(Moux, Mouy, box_ok)) {
                if (strcmpi(Ptop->foreground_name, foreground_name) != 0 ||
                    strcmpi(Ptop->background_name, background_name) != 0 ||
                    strcmp(Ptop->level_name, level_name) != 0) {
                    Valtozott = 1;
                }
                strcpy(Ptop->foreground_name, foreground_name);
                strcpy(Ptop->background_name, background_name);
                strcpy(Ptop->level_name, level_name);

                if (strcmpi(lgr_name, Ptop->lgr_name) != 0) {
                    Valtozott = 1;
                    strcpy(Ptop->lgr_name, lgr_name);
                    lgrfile::load_lgr_file(Ptop->lgr_name);
                    if (Ptop->discard_missing_lgr_assets(Lgr)) {
                        dialog_warn_lgr_assets_deleted();
                    }
                }
                return;
            }
            if (is_in_box(Moux, Mouy, box_cancel)) {
                return;
            }
            if (is_in_box(Moux, Mouy, box_foreground)) {
                char null_name[10] = "";
                char mask_name[10] = "";
                if (Lgr->get_mask_index("maskbig") >= 0) {
                    strcpy(mask_name, "maskbig");
                }
                editor_window_select_sprite_name(null_name, foreground_name, mask_name,
                                                 SpriteType::Texture);
                push();
                blit8(BufferMain, BufferBall);
                bltfront(BufferMain);
                pop();
                rerender = true;
            }
            if (is_in_box(Moux, Mouy, box_background)) {
                char null_name[10] = "";
                char mask_name[10] = "";
                if (Lgr->get_mask_index("maskbig") >= 0) {
                    strcpy(mask_name, "maskbig");
                }
                editor_window_select_sprite_name(null_name, background_name, mask_name,
                                                 SpriteType::Texture);
                push();
                blit8(BufferMain, BufferBall);
                bltfront(BufferMain);
                pop();
                rerender = true;
            }
            if (is_in_box(Moux, Mouy, box_levelname)) {
                editor_window_level_name(level_name);
                push();
                blit8(BufferMain, BufferBall);
                bltfront(BufferMain);
                pop();
                rerender = true;
            }
            if (is_in_box(Moux, Mouy, box_lgr)) {
                editor_window_choose_lgr(lgr_name);

                push();
                blit8(BufferMain, BufferBall);
                bltfront(BufferMain);
                pop();
                rerender = true;
            }
        }
        if (rerender) {
            rerender = false;

            push();

            render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);

            Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y1 + 22, "Level properties");

            render_box(BufferMain, box_ok, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (box_ok.x1 + box_ok.x2) / 2, box_ok.y1 + 15, "OK");

            render_box(BufferMain, box_cancel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (box_cancel.x1 + box_cancel.x2) / 2,
                                  box_cancel.y1 + 15, "CANCEL");

            int label_x1 = x1 + 10;
            Pabc2->write(BufferMain, label_x1, box_foreground.y1 + 15, "Foreground:");
            render_box(BufferMain, box_foreground, Feherszin, DIALOG_BORDER_PALETTE_ID);
            draw_textbox_left(BufferMain, box_foreground, Feherszin, foreground_name);

            Pabc2->write(BufferMain, label_x1, box_background.y1 + 15, "Background:");
            render_box(BufferMain, box_background, Feherszin, DIALOG_BORDER_PALETTE_ID);
            draw_textbox_left(BufferMain, box_background, Feherszin, background_name);

            Pabc2->write(BufferMain, label_x1, box_levelname.y1 + 15, "Level name:");
            render_box(BufferMain, box_levelname, Feherszin, DIALOG_BORDER_PALETTE_ID);
            draw_textbox_left(BufferMain, box_levelname, Feherszin, level_name);

            Pabc2->write(BufferMain, label_x1, box_lgr.y1 + 15, "LGR file:");
            render_box(BufferMain, box_lgr, Feherszin, DIALOG_BORDER_PALETTE_ID);
            draw_textbox_left(BufferMain, box_lgr, Feherszin, lgr_name);

            bltfront(BufferMain, x1, y1, x2, y2);
            pop();
        }
        update_and_draw_cursor();
    }
}

bool ShowPolygons = true;
bool ShowGrassPolygons = true;
bool ShowObjects = true;

void editor_window_view_options() {
    invalidateegesz();

    int x1 = 200;
    int y1 = 100;
    int x2 = 360;
    int y2 = 230;

    int label_x1 = x1 + 14;

    box box_ok = {(x1 + x2) / 2 - 20, y2 - 30, (x1 + x2) / 2 + 20, y2 - 10};

    box box_polygons = {x1 + 110, y1 + 18, x1 + 140, y1 + 38};
    box box_grass = {x1 + 110, y1 + 40, x1 + 140, y1 + 60};
    box box_objects = {x1 + 110, y1 + 65, x1 + 140, y1 + 85};

    bool rerender = true;
    while (true) {
        handle_events();
        if (was_key_just_pressed(DIK_ESCAPE) || was_key_just_pressed(DIK_RETURN)) {
            return;
        }
        if (was_left_mouse_just_clicked()) {
            update_and_draw_cursor();
            if (is_in_box(Moux, Mouy, box_ok)) {
                return;
            }
            if (is_in_box(Moux, Mouy, box_polygons)) {
                ShowPolygons = !ShowPolygons;
                rerender = true;
            }
            if (is_in_box(Moux, Mouy, box_grass)) {
                ShowGrassPolygons = !ShowGrassPolygons;
                rerender = true;
            }
            if (is_in_box(Moux, Mouy, box_objects)) {
                ShowObjects = !ShowObjects;
                rerender = true;
            }
        }
        if (rerender) {
            rerender = false;
            push();

            render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);

            render_box(BufferMain, box_ok, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (box_ok.x1 + box_ok.x2) / 2, box_ok.y1 + 15, "OK");

            Pabc2->write(BufferMain, label_x1, box_polygons.y1 + 15, "View Polygons");
            render_box(BufferMain, box_polygons, Feherszin, DIALOG_BORDER_PALETTE_ID);
            if (ShowPolygons) {
                draw_textbox_centered(BufferMain, box_polygons, Feherszin, "Yes");
            } else {
                draw_textbox_centered(BufferMain, box_polygons, Feherszin, "No");
            }

            Pabc2->write(BufferMain, label_x1, box_grass.y1 + 15, "View Grass");
            render_box(BufferMain, box_grass, Feherszin, DIALOG_BORDER_PALETTE_ID);
            if (ShowGrassPolygons) {
                draw_textbox_centered(BufferMain, box_grass, Feherszin, "Yes");
            } else {
                draw_textbox_centered(BufferMain, box_grass, Feherszin, "No");
            }

            Pabc2->write(BufferMain, label_x1, box_objects.y1 + 15, "View Pictures");
            render_box(BufferMain, box_objects, Feherszin, DIALOG_BORDER_PALETTE_ID);
            if (ShowObjects) {
                draw_textbox_centered(BufferMain, box_objects, Feherszin, "Yes");
            } else {
                draw_textbox_centered(BufferMain, box_objects, Feherszin, "No");
            }

            bltfront(BufferMain, x1, y1, x2, y2);
            pop();
        }
        update_and_draw_cursor();
    }
}
