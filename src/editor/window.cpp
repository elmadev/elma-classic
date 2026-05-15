#include "editor/window.h"
#include "abc8.h"
#include "editor/canvas.h"
#include "editor/dialog.h"
#include "EDITUJ.H"
#include "fs_utils.h"
#include "keys.h"
#include "level.h"
#include "level_load.h"
#include "lgr.h"
#include "M_PIC.H"
#include "main.h"
#include "menu/pic.h"
#include "pic8.h"
#include "platform/implementation.h"
#include "platform/utils.h"
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
    if (!Level) {
        internal_error("open_level !Level");
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
    LevelChanged = false;
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
    if (was_key_down(DIK_PRIOR) || clicked_box(box_up)) {
        selected_index -= max_visible_entries - 1;
        rerender = true;
    }
    if (was_key_down(DIK_DOWN)) {
        selected_index++;
        rerender = true;
    }
    if (was_key_down(DIK_NEXT) || clicked_box(box_down)) {
        selected_index += max_visible_entries - 1;
        rerender = true;
    }
    int wheel = get_mouse_wheel_delta();
    if (wheel != 0 && is_in_box(MouseX, MouseY, box_list)) {
        selected_index -= wheel;
        rerender = true;
    }

    selected_index = std::max(selected_index, 0);
    selected_index = std::min(selected_index, length - 1);
    view_index = std::min(view_index, selected_index);
    view_index = std::max(view_index, selected_index - max_visible_entries + 1);
}

template <typename NameAt>
static bool process_list_search(std::string& search_input, int& selected_index, int& view_index,
                                int list_length, int max_visible_entries, NameAt name_at) {
    bool changed = false;
    while (has_text_input()) {
        char c = pop_text_input();
        if (EditorBlackFont->has_char(c) && search_input.size() < MAX_FILENAME_LEN) {
            search_input.push_back(c);
            changed = true;
        }
    }
    if (was_key_down(DIK_BACK) && !search_input.empty()) {
        search_input.pop_back();
        changed = true;
    }
    if (!changed) {
        return false;
    }
    if (!search_input.empty() && list_length > 0) {
        const char* query = search_input.c_str();
        // Binary search for the first entry sorting >= query.
        int lo = 0;
        int hi = list_length;
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (strcmpi(name_at(mid), query) < 0) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        selected_index = std::min(lo, list_length - 1);
        // If the landing slot doesn't have the prefix, the previous entry
        // may share a longer common prefix with the query.
        if (selected_index > 0 &&
            strnicmp(name_at(selected_index), query, search_input.size()) != 0) {
            size_t a = util::text::common_prefix_len(query, name_at(selected_index));
            size_t b = util::text::common_prefix_len(query, name_at(selected_index - 1));
            if (b >= a) {
                selected_index -= 1;
            }
        }
    }
    view_index = std::min(view_index, selected_index);
    view_index = std::max(view_index, selected_index - max_visible_entries + 1);
    return true;
}

static void render_list_search(pic8* pic, box bx, const std::string& search_input) {
    std::string text = search_input.empty() ? "Type to Search" : "Search: " + search_input;

    EditorBlackFont->write_centered(pic, (bx.x1 + bx.x2) / 2, (bx.y1 + bx.y2) / 2 + 5,
                                    text.c_str());
}

// Draw a borderless box with background color and centered text
// Optionally add '_' to the end of the string for input prompt
static void draw_textbox_centered(pic8* pic, box bx, unsigned char fill_id, const char* text,
                                  bool add_underline = false) {
    pic->fill_box(bx.x1 + 1, bx.y1 + 1, bx.x2 - 1, bx.y2 - 1, fill_id);
    if (add_underline) {
        int length = EditorBlackFont->len(text);
        int x = (bx.x1 + bx.x2) / 2 - length / 2 - 3;
        EditorBlackFont->write(pic, x, (bx.y1 + bx.y2) / 2 + 5, text);
        for (int i = 0; i < 6; i++) {
            pic->ppixel(x + length + i + 1, (bx.y1 + bx.y2) / 2 + 4, 0);
        }
    } else {
        EditorBlackFont->write_centered(pic, (bx.x1 + bx.x2) / 2, (bx.y1 + bx.y2) / 2 + 5, text);
    }
}

// Draw a borderless box with background color and left-justified text
static void draw_textbox_left(pic8* pic, box bx, unsigned char fill_id, const char* text) {
    pic->fill_box(bx.x1 + 1, bx.y1 + 1, bx.x2 - 1, bx.y2 - 1, fill_id);
    EditorBlackFont->write(pic, bx.x1 + 2, (bx.y1 + bx.y2) / 2 + 5, text);
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
    int top_margin = 20;
    int y2 = 174 + top_margin + max_visible_entries * dy;
    int lx1 = x1 + 10;
    int ly1 = y1 + top_margin + 37;
    int lx2 = lx1 + 100;
    int ly2 = ly1 + max_visible_entries * dy;

    box box_list = {lx1, ly1, lx2, ly2};
    box box_up = {x1 + 10, y1 + top_margin + 11, x1 + 110, y1 + top_margin + 31};
    box box_down = {x1 + 10, y2 - 30, x1 + 110, y2 - 10};
    box box_cancel = {x1 + 121, (y2 + y1) / 2 - 10, x1 + 121 + 70, (y2 + y1) / 2 + 10};
    box box_new = {x1 + 121, (y2 + y1) / 2 - 40, x1 + 121 + 70, (y2 + y1) / 2 - 20};
    box box_search = {x1, y1, x2, box_up.y1};

    int view_index = 0;
    bool rerender = true;
    std::string search_input;
    empty_keypress_buffer();
    while (true) {
        handle_events();
        update_and_draw_cursor();
        adjust_list_view(selected_index, view_index, list_length, max_visible_entries, rerender,
                         box_up, box_down, box_list);
        if (process_list_search(search_input, selected_index, view_index, list_length,
                                max_visible_entries,
                                [](int i) { return ListEntries[i].c_str(); })) {
            rerender = true;
        }
        if (was_key_just_pressed(DIK_ESCAPE) && !search_input.empty()) {
            search_input.clear();
            rerender = true;
        } else if (was_key_just_pressed(DIK_ESCAPE) || clicked_box(box_cancel)) {
            return "";
        } else if (show_new_button && clicked_box(box_new)) {
            return "";
        } else if (was_key_just_pressed(DIK_RETURN)) {
            return ListEntries[selected_index];
        } else if (clicked_box(box_list)) {
            if (MouseY < ly1 + dy * max_visible_entries) {
                int index = (MouseY - ly1) / dy;
                index += view_index;
                if (index < list_length) {
                    return ListEntries[index];
                }
            }
        }
        if (rerender) {
            rerender = false;

            erase_cursor();
            render_box(BufferMain, x1, y1, x2, y2, EditorPaletteId::WINDOW,
                       EditorPaletteId::WINDOW_BORDER);
            render_box(BufferMain, lx1, ly1, lx2, ly2, EditorPaletteId::WINDOW_LIST,
                       EditorPaletteId::WINDOW_BORDER);
            for (int i = 0; i < max_visible_entries && i + view_index < list_length; i++) {
                if (i + view_index == selected_index) {
                    BufferMain->fill_box(lx1 + 1, ly1 + i * dy + 1, lx2 - 1, ly1 + (i + 1) * dy - 1,
                                         EditorPaletteId::WINDOW_LIST_SELECTED);
                }
                EditorBlackFont->write(BufferMain, lx1 + 3, ly1 + 15 + i * dy,
                                       ListEntries[i + view_index].c_str());
            }
            render_box(BufferMain, box_up, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            draw_arrow(BufferMain, box_up, EditorPaletteId::WINDOW_BORDER, true);
            render_box(BufferMain, box_down, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            draw_arrow(BufferMain, box_down, EditorPaletteId::WINDOW_BORDER, false);
            render_box(BufferMain, box_cancel, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            EditorBlackFont->write_centered(BufferMain, (box_cancel.x1 + box_cancel.x2) / 2,
                                            box_cancel.y1 + 15, "CANCEL");
            if (show_new_button) {
                render_box(BufferMain, box_new, EditorPaletteId::WINDOW_BUTTON,
                           EditorPaletteId::WINDOW_BORDER);
                EditorBlackFont->write_centered(BufferMain, (box_new.x1 + box_new.x2) / 2,
                                                box_new.y1 + 15, "NEW");
            }
            render_list_search(BufferMain, box_search, search_input);

            bltfront(BufferMain, x1, y1, x2, y2);
            draw_cursor();
        }
    }
}

void editor_window_welcome() {
    invalidate_editor_gui();

    std::string filename = editor_window_list_levels(true);

    if (filename.length() > MAX_FILENAME_LEN) {
        internal_error("editor_window_welcome strlen(selected_filename) > MAX_FILENAME_LEN");
    }

    if (filename.empty()) {
        State->editor_filename[0] = 0;
    } else {
        strcpy(State->editor_filename, (filename + ".lev").c_str());
    }
}

void editor_window_open() {
    invalidate_editor_gui();

    if (LevelChanged) {
        if (dialog("There are unsaved changes in the level file.",
                   "If you open another file, you will loose these changes.",
                   "Do you still want to continue?", DIALOG_BUTTONS, "Yes", "No") == 1) {
            return;
        }
    }

    std::string filename = editor_window_list_levels(false);

    if (ListEntries.empty()) {
        dialog("There are no level files (*.lev) in the LEV directory!");
        return;
    }

    if (filename.empty()) {
        return;
    }

    open_level(filename.c_str());
}

void editor_new() {
    invalidate_editor_gui();
    if (LevelChanged) {
        if (dialog("There are unsaved changes in the level file.", "Do you still want to continue?",
                   DIALOG_BUTTONS, "Yes", "No") == 1) {
            return;
        }
    }

    LevelChanged = false;
    load_level_editor(DEFAULT_LEVEL_FILENAME);
    State->editor_filename[0] = 0;
    zoom_fill();
}

bool editor_window_save() {
    invalidate_editor_gui();
    if (State->editor_filename[0] == 0) {
        return editor_window_save_as();
    }
    Level->save(State->editor_filename);
    LevelChanged = false;
    return true;
}

bool editor_window_save_as() {
    invalidate_editor_gui();

    box box_window = {20, 200, 600, 300};
    box box_save = {320 - 30, 273, 320 + 30, 290};

    erase_cursor();
    render_box(BufferMain, box_window, EditorPaletteId::WINDOW, EditorPaletteId::WINDOW_BORDER);
    render_box(BufferMain, box_save, EditorPaletteId::WINDOW_BUTTON,
               EditorPaletteId::WINDOW_BORDER);
    EditorBlackFont->write_centered(
        BufferMain, (box_window.x2 + box_window.x1) / 2, 220,
        "Type in the file name and press ENTER or click on SAVE, or press ESC to cancel!");
    EditorBlackFont->write_centered(BufferMain, (box_save.x2 + box_save.x1) / 2, box_save.y1 + 13,
                                    "SAVE");

    bltfront(BufferMain, box_window.x1, box_window.y1, box_window.x2, box_window.y2);
    draw_cursor();

    empty_keypress_buffer();
    finame filename_input = "";
    int i = 0;
    char filename_input_prev[10] = "$%...%^&";
    while (true) {
        handle_events();
        update_and_draw_cursor();
        if (strcmp(filename_input_prev, filename_input) != 0) {
            strcpy(filename_input_prev, filename_input);

            int ex1 = box_window.x1 + 100;
            int ey1 = box_window.y1 + 30;
            int ex2 = box_window.x2 - 100;
            int ey2 = box_window.y1 + 55;
            BufferMain->fill_box(ex1, ey1, ex2, ey2, EditorPaletteId::WINDOW);
            EditorBlackFont->write(BufferMain, 290, 250, filename_input);
            char tmp[40];
            strcpy(tmp, filename_input);
            tmp[i] = 0;
            EditorBlackFont->write(BufferMain, 290 + EditorWhiteFont->len(tmp), 255, "-");
            bltfront(BufferMain, ex1, ey1, ex2, ey2);
        }
        if (was_key_just_pressed(DIK_ESCAPE) || was_right_mouse_just_clicked()) {
            return false;
        }
        if (was_key_just_pressed(DIK_RETURN) || clicked_box(box_save)) {
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
                Level->save(filename_input);
                strcpy(State->editor_filename, filename_input);
                LevelChanged = false;
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
            if (EditorBlackFont->has_char(c) && util::text::is_filename_char(c)) {
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
    if (x < 0 || y < 0) {
        internal_error("preview_picture out of bounds");
    }
    if (x >= dest->get_width() || y >= dest->get_height()) {
        return;
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
    if (x < 0 || y < 0) {
        internal_error("preview_texture out of bounds");
    }
    if (x >= dest->get_width() || y >= dest->get_height()) {
        return;
    }

    pic8* source = text->pic;

    int dest_width = dest->get_width();

    // Trim height to be in-bounds
    int height = msk->height;
    height = std::min(height, dest->get_height() - y);

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
                    // Trim width to be in-bounds
                    if (dest_x < dest_width) {
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

    auto name_at = [sprite_type](int i) -> const char* {
        switch (sprite_type) {
        case SpriteType::Picture:
            return Lgr->pictures[i].name;
        case SpriteType::Texture:
            return Lgr->textures[i].name;
        case SpriteType::Mask:
            return Lgr->masks[i].name;
        }
        return "";
    };

    int selected_index = 0;
    for (int i = 0; i < list_length; i++) {
        if (strcmpi(name_at(i), name) == 0) {
            selected_index = i;
        }
    }

    if (selected_index == 0) {
        strcpy(name, name_at(0));
    }

    int max_visible_entries = 10;
    int dy = 20;
    int x1 = 200;
    int y1 = 100;
    int x2 = 401;
    int top_margin = 20;
    int y2 = 174 + top_margin + max_visible_entries * dy;
    int lx1 = x1 + 10;
    int ly1 = y1 + top_margin + 37;
    int lx2 = lx1 + 100;
    int ly2 = ly1 + max_visible_entries * dy;

    box box_list = {lx1, ly1, lx2, ly2};
    box box_up = {x1 + 10, y1 + top_margin + 11, x1 + 110, y1 + top_margin + 31};
    box box_down = {x1 + 10, y2 - 30, x1 + 110, y2 - 10};
    box box_cancel = {x1 + 121, (y2 + y1) / 2 - 10, x1 + 121 + 70, (y2 + y1) / 2 + 10};
    box box_search = {x1, y1, x2, box_up.y1};

    int view_index = 0;
    bool rerender = true;
    std::string search_input;
    empty_keypress_buffer();

    // Keep track of the box where we are drawing, so we can restore the background later
    int preview_x2 = 0;
    int preview_y2 = 0;
    erase_cursor();
    blit8(BufferBall, BufferMain);
    draw_cursor();

    while (true) {
        handle_events();
        update_and_draw_cursor();
        adjust_list_view(selected_index, view_index, list_length, max_visible_entries, rerender,
                         box_up, box_down, box_list);
        if (process_list_search(search_input, selected_index, view_index, list_length,
                                max_visible_entries, name_at)) {
            rerender = true;
        }
        if (was_key_just_pressed(DIK_ESCAPE) && !search_input.empty()) {
            search_input.clear();
            rerender = true;
        } else if (was_key_just_pressed(DIK_ESCAPE) || clicked_box(box_cancel)) {
            strcpy(name, original_name);
            return;
        } else if (was_key_just_pressed(DIK_RETURN)) {
            strcpy(name, name_at(selected_index));
            return;
        } else if (clicked_box(box_list)) {
            if (MouseY < ly1 + dy * max_visible_entries) {
                int index = (MouseY - ly1) / dy;
                index += view_index;
                if (index < list_length) {
                    strcpy(name, name_at(index));
                    return;
                }
            }
        }
        if (rerender) {
            rerender = false;

            strcpy(name, name_at(selected_index));

            // Restore background image
            erase_cursor();
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
            render_box(BufferMain, x1, y1, x2, y2, EditorPaletteId::WINDOW,
                       EditorPaletteId::WINDOW_BORDER);
            render_box(BufferMain, lx1, ly1, lx2, ly2, EditorPaletteId::WINDOW_LIST,
                       EditorPaletteId::WINDOW_BORDER);
            for (int i = 0; i < max_visible_entries && i + view_index < list_length; i++) {
                if (i + view_index == selected_index) {
                    BufferMain->fill_box(lx1 + 1, ly1 + i * dy + 1, lx2 - 1, ly1 + (i + 1) * dy - 1,
                                         EditorPaletteId::WINDOW_LIST_SELECTED);
                }
                EditorBlackFont->write(BufferMain, lx1 + 3, ly1 + 15 + i * dy,
                                       name_at(i + view_index));

                if (sprite_type == SpriteType::Picture || sprite_type == SpriteType::Texture) {
                    int default_distance = 0;
                    if (sprite_type == SpriteType::Picture) {
                        default_distance = Lgr->pictures[i + view_index].default_distance;
                    } else {
                        default_distance = Lgr->textures[i + view_index].default_distance;
                    }

                    char details[20];
                    sprintf(details, "%d", default_distance);
                    EditorBlackFont->write(BufferMain, lx1 + 67, ly1 + 15 + i * dy, details);
                    Clipping default_clipping = Clipping::Unclipped;
                    if (sprite_type == SpriteType::Picture) {
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
                    EditorBlackFont->write(BufferMain, lx1 + 92, ly1 + 15 + i * dy, details);
                }
            }

            // Draw other buttons
            render_box(BufferMain, box_up, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            draw_arrow(BufferMain, box_up, EditorPaletteId::WINDOW_BORDER, true);
            render_box(BufferMain, box_down, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            draw_arrow(BufferMain, box_down, EditorPaletteId::WINDOW_BORDER, false);
            render_box(BufferMain, box_cancel, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            EditorBlackFont->write_centered(BufferMain, (box_cancel.x1 + box_cancel.x2) / 2,
                                            box_cancel.y1 + 15, "CANCEL");
            render_list_search(BufferMain, box_search, search_input);

            bltfront(BufferMain);
            draw_cursor();
        }
    }
}

void editor_window_choose_sprite() {
    if (!Lgr) {
        internal_error("editor_window_choose_sprite !Lgr!");
    }

    invalidate_editor_gui();

    char picture_name[10];
    char texture_name[10];
    char mask_name[10];
    char null_name[10] = "";
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
        update_and_draw_cursor();
        if (was_key_just_pressed(DIK_ESCAPE) || clicked_box(box_cancel)) {
            return;
        } else if (was_key_just_pressed(DIK_RETURN) || clicked_box(box_ok)) {
            strcpy(Lgr->editor_picture_name, picture_name);
            strcpy(Lgr->editor_texture_name, texture_name);
            strcpy(Lgr->editor_mask_name, mask_name);
            return;
        } else if (clicked_box(box_picture)) {
            editor_window_select_sprite_name(picture_name, null_name, null_name,
                                             SpriteType::Picture);
            erase_cursor();
            blit8(BufferMain, BufferBall);
            bltfront(BufferMain);
            draw_cursor();
            if (picture_name[0]) {
                texture_name[0] = mask_name[0] = 0;
            }
            rerender = true;
        } else if (clicked_box(box_texture)) {
            editor_window_select_sprite_name(null_name, texture_name, mask_name,
                                             SpriteType::Texture);
            erase_cursor();
            blit8(BufferMain, BufferBall);
            bltfront(BufferMain);
            draw_cursor();
            if (texture_name[0]) {
                picture_name[0] = 0;
            }
            rerender = true;
        } else if (clicked_box(box_mask)) {
            editor_window_select_sprite_name(null_name, texture_name, mask_name, SpriteType::Mask);
            erase_cursor();
            blit8(BufferMain, BufferBall);
            bltfront(BufferMain);
            draw_cursor();
            if (mask_name[0]) {
                picture_name[0] = 0;
            }
            rerender = true;
        }
        if (rerender) {
            rerender = false;

            erase_cursor();

            render_box(BufferMain, x1, y1, x2, y2, EditorPaletteId::WINDOW,
                       EditorPaletteId::WINDOW_BORDER);

            EditorBlackFont->write_centered(BufferMain, (x1 + x2) / 2, y1 + 20, "Choose picture");

            render_box(BufferMain, box_ok, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            render_box(BufferMain, box_cancel, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            EditorBlackFont->write_centered(BufferMain, (box_ok.x1 + box_ok.x2) / 2, box_ok.y1 + 15,
                                            "OK");
            EditorBlackFont->write_centered(BufferMain, (box_cancel.x1 + box_cancel.x2) / 2,
                                            box_cancel.y1 + 15, "CANCEL");

            int label_x1 = x1 + 8;
            EditorBlackFont->write(BufferMain, label_x1, box_picture.y1 + 15, "Normal Picture:");
            render_box(BufferMain, box_picture, EditorPaletteId::WINDOW_INPUT,
                       EditorPaletteId::WINDOW_BORDER);
            draw_textbox_left(BufferMain, box_picture, EditorPaletteId::WINDOW_INPUT, picture_name);

            EditorBlackFont->write(BufferMain, label_x1, box_texture.y1 + 15, "Texture:");
            render_box(BufferMain, box_texture, EditorPaletteId::WINDOW_INPUT,
                       EditorPaletteId::WINDOW_BORDER);
            draw_textbox_left(BufferMain, box_texture, EditorPaletteId::WINDOW_INPUT, texture_name);

            EditorBlackFont->write(BufferMain, label_x1, box_mask.y1 + 15, "Mask:");
            render_box(BufferMain, box_mask, EditorPaletteId::WINDOW_INPUT,
                       EditorPaletteId::WINDOW_BORDER);
            draw_textbox_left(BufferMain, box_mask, EditorPaletteId::WINDOW_INPUT, mask_name);

            bltfront(BufferMain);
            draw_cursor();
        }
    }
}

bool editor_window_choose_lgr(char* lgrname) {
    invalidate_editor_gui();

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
    int top_margin = 20;
    int bottom_margin = 60;
    int y2 = 174 + top_margin + max_visible_entries * dy + bottom_margin;
    int lx1 = x1 + 10;
    int ly1 = y1 + top_margin + 37;
    int lx2 = lx1 + 100;
    int ly2 = ly1 + max_visible_entries * dy;

    box box_list = {lx1, ly1, lx2, ly2};
    box box_up = {x1 + 10, y1 + top_margin + 11, x1 + 110, y1 + top_margin + 31};
    box box_down = {x1 + 10, y2 - 30 - bottom_margin, x1 + 110, y2 - 10 - bottom_margin};
    box box_cancel = {x1 + 121, (y2 + y1) / 2 - 10, x1 + 121 + 70, (y2 + y1) / 2 + 10};
    box box_search = {x1, y1, x2, box_up.y1};

    int view_index = 0;
    bool rerender = true;
    std::string search_input;
    empty_keypress_buffer();
    while (true) {
        handle_events();
        update_and_draw_cursor();
        adjust_list_view(selected_index, view_index, list_length, max_visible_entries, rerender,
                         box_up, box_down, box_list);
        if (process_list_search(search_input, selected_index, view_index, list_length,
                                max_visible_entries,
                                [](int i) { return ListEntries[i].c_str(); })) {
            rerender = true;
        }
        if (was_key_just_pressed(DIK_ESCAPE) && !search_input.empty()) {
            search_input.clear();
            rerender = true;
        } else if (was_key_just_pressed(DIK_ESCAPE) || clicked_box(box_cancel)) {
            return false;
        } else if (was_key_just_pressed(DIK_RETURN)) {
            strcpy(lgrname, ListEntries[selected_index].c_str());
            return true;
        } else if (clicked_box(box_list)) {
            if (MouseY < ly1 + dy * max_visible_entries) {
                int index = (MouseY - ly1) / dy;
                index += view_index;
                if (index < list_length) {
                    strcpy(lgrname, ListEntries[index].c_str());
                    return true;
                }
            }
        }
        if (rerender) {
            rerender = false;

            erase_cursor();
            render_box(BufferMain, x1, y1, x2, y2, EditorPaletteId::WINDOW,
                       EditorPaletteId::WINDOW_BORDER);
            render_box(BufferMain, lx1, ly1, lx2, ly2, EditorPaletteId::WINDOW_LIST,
                       EditorPaletteId::WINDOW_BORDER);
            for (int i = 0; i < max_visible_entries && i + view_index < list_length; i++) {
                if (i + view_index == selected_index) {
                    BufferMain->fill_box(lx1 + 1, ly1 + i * dy + 1, lx2 - 1, ly1 + (i + 1) * dy - 1,
                                         EditorPaletteId::WINDOW_LIST_SELECTED);
                }
                EditorBlackFont->write(BufferMain, lx1 + 3, ly1 + 15 + i * dy,
                                       ListEntries[i + view_index].c_str());
            }
            render_box(BufferMain, box_up, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            draw_arrow(BufferMain, box_up, EditorPaletteId::WINDOW_BORDER, true);
            render_box(BufferMain, box_down, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            draw_arrow(BufferMain, box_down, EditorPaletteId::WINDOW_BORDER, false);
            render_box(BufferMain, box_cancel, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            EditorBlackFont->write_centered(BufferMain, (box_cancel.x1 + box_cancel.x2) / 2,
                                            box_cancel.y1 + 15, "CANCEL");

            render_list_search(BufferMain, box_search, search_input);

            EditorBlackFont->write_centered(BufferMain, (x1 + x2) / 2, y2 - 36,
                                            "Original LGR file:");
            EditorBlackFont->write_centered(BufferMain, (x1 + x2) / 2, y2 - 18, lgrname);

            bltfront(BufferMain, x1, y1, x2, y2);
            draw_cursor();
        }
    }
}

static int prompt_distance(pic8* pic, box bx, int distance) {
    erase_cursor();

    empty_keypress_buffer();
    bool rerender = true;
    int distance_prev = distance;
    while (true) {
        handle_events();
        if (rerender) {
            rerender = false;
            char tmp[10];
            sprintf(tmp, "%d", distance);
            draw_textbox_centered(pic, bx, EditorPaletteId::WINDOW_INPUT, tmp, true);
            bltfront(pic, bx.x1, bx.y1, bx.x2, bx.y2);
        }
        if (was_key_just_pressed(DIK_ESCAPE)) {
            draw_cursor();
            return distance_prev;
        }
        if (was_key_just_pressed(DIK_RETURN)) {
            draw_cursor();
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
    erase_cursor();
    draw_textbox_centered(pic, bx, EditorPaletteId::WINDOW_INPUT, "", true);
    bltfront(pic, bx.x1, bx.y1, bx.x2, bx.y2);
    empty_keypress_buffer();
    while (true) {
        handle_events();
        if (was_key_just_pressed(DIK_ESCAPE)) {
            draw_cursor();
            return clipping;
        }
        while (has_text_input()) {
            char c = pop_text_input();
            if (c == 'u' || c == 'U') {
                draw_cursor();
                return Clipping::Unclipped;
            }
            if (c == 'g' || c == 'G') {
                draw_cursor();
                return Clipping::Ground;
            }
            if (c == 's' || c == 'S') {
                draw_cursor();
                return Clipping::Sky;
            }
        }
    }
}

void editor_window_sprite_properties(sprite* spr) {
    invalidate_editor_gui();

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
        update_and_draw_cursor();
        if (was_key_just_pressed(DIK_ESCAPE) || clicked_box(box_cancel)) {
            return;
        } else if (was_key_just_pressed(DIK_RETURN) || clicked_box(box_ok)) {
            if (spr->distance != distance || spr->clipping != clipping) {
                LevelChanged = true;
            }
            spr->distance = distance;
            spr->clipping = clipping;
            return;
        } else if (clicked_box(box_distance)) {
            distance = prompt_distance(BufferMain, box_distance, distance);
            rerender = true;
        } else if (clicked_box(box_clipping)) {
            clipping = prompt_clipping(BufferMain, box_clipping, clipping);
            rerender = true;
        }
        if (rerender) {
            rerender = false;
            erase_cursor();
            render_box(BufferMain, x1, y1, x2, y2, EditorPaletteId::WINDOW,
                       EditorPaletteId::WINDOW_BORDER);
            render_box(BufferMain, box_ok, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            render_box(BufferMain, box_cancel, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            EditorBlackFont->write_centered(BufferMain, (box_ok.x1 + box_ok.x2) / 2, box_ok.y1 + 15,
                                            "OK");
            EditorBlackFont->write_centered(BufferMain, (box_cancel.x1 + box_cancel.x2) / 2,
                                            box_cancel.y1 + 15, "CANCEL");

            EditorBlackFont->write_centered(BufferMain, (x1 + x2) / 2, y1 + 15,
                                            "Set Picture Properties");
            if (spr->picture_name[0]) {
                EditorBlackFont->write_centered(BufferMain, (x1 + x2) / 2, y1 + 45,
                                                spr->picture_name);
            } else {
                EditorBlackFont->write_centered(BufferMain, (x1 + x2) / 2, y1 + 34,
                                                spr->texture_name);
                EditorBlackFont->write_centered(BufferMain, (x1 + x2) / 2, y1 + 56, spr->mask_name);
            }
            EditorBlackFont->write(BufferMain, x1 + 92, y1 + 76, "Distance");
            EditorBlackFont->write(BufferMain, x1 + 172, y1 + 76, "Clipping");

            // Default values
            EditorBlackFont->write(BufferMain, x1 + 12, y1 + 100, "Default:");
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
            EditorBlackFont->write_centered(BufferMain, x1 + 119, y1 + 100, tmp);

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
            EditorBlackFont->write_centered(BufferMain, x1 + 198, y1 + 100, tmp);

            // Actual values
            EditorBlackFont->write(BufferMain, x1 + 12, y1 + 130, "Current:");

            render_box(BufferMain, box_distance, EditorPaletteId::WINDOW_INPUT,
                       EditorPaletteId::WINDOW_BORDER);
            sprintf(tmp, "%d", distance);
            draw_textbox_centered(BufferMain, box_distance, EditorPaletteId::WINDOW_INPUT, tmp);
            EditorBlackFont->write_centered(BufferMain, x1 + 119, box_distance.y2 + 14, "(1-999)");

            render_box(BufferMain, box_clipping, EditorPaletteId::WINDOW_INPUT,
                       EditorPaletteId::WINDOW_BORDER);
            if (clipping == Clipping::Unclipped) {
                strcpy(tmp, "U");
            }
            if (clipping == Clipping::Ground) {
                strcpy(tmp, "G");
            }
            if (clipping == Clipping::Sky) {
                strcpy(tmp, "S");
            }
            draw_textbox_centered(BufferMain, box_clipping, EditorPaletteId::WINDOW_INPUT, tmp);
            EditorBlackFont->write_centered(BufferMain, x1 + 198, box_clipping.y2 + 14,
                                            "(U, S, G)");

            bltfront(BufferMain, x1, y1, x2, y2);
            draw_cursor();
        }
    }
}

void editor_window_polygon_properties(polygon* poly) {
    invalidate_editor_gui();

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
        update_and_draw_cursor();
        if (was_key_just_pressed(DIK_ESCAPE) || clicked_box(box_cancel)) {
            return;
        } else if (was_key_just_pressed(DIK_RETURN) || clicked_box(box_ok)) {
            if (poly->is_grass != is_grass) {
                LevelChanged = true;
            }
            poly->is_grass = is_grass;
            return;
        } else if (clicked_box(box_grass)) {
            is_grass = !is_grass;
            rerender = true;
        }
        if (rerender) {
            rerender = false;

            erase_cursor();
            render_box(BufferMain, x1, y1, x2, y2, EditorPaletteId::WINDOW,
                       EditorPaletteId::WINDOW_BORDER);

            EditorBlackFont->write_centered(BufferMain, (x1 + x2) / 2, y1 + 15,
                                            "Set Polygon Properties");

            int label_x1 = x1 + 18;
            EditorBlackFont->write(BufferMain, label_x1, box_grass.y1 + 15, "Grass polygon:");
            render_box(BufferMain, box_grass, EditorPaletteId::WINDOW_INPUT,
                       EditorPaletteId::WINDOW_BORDER);
            if (is_grass) {
                EditorBlackFont->write_centered(BufferMain, (box_grass.x1 + box_grass.x2) / 2,
                                                box_grass.y1 + 15, "YES");
            } else {
                EditorBlackFont->write_centered(BufferMain, (box_grass.x1 + box_grass.x2) / 2,
                                                box_grass.y1 + 15, "NO");
            }

            render_box(BufferMain, box_ok, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            render_box(BufferMain, box_cancel, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            EditorBlackFont->write_centered(BufferMain, (box_ok.x1 + box_ok.x2) / 2, box_ok.y1 + 15,
                                            "OK");
            EditorBlackFont->write_centered(BufferMain, (box_cancel.x1 + box_cancel.x2) / 2,
                                            box_cancel.y1 + 15, "CANCEL");

            bltfront(BufferMain, x1, y1, x2, y2);
            draw_cursor();
        }
    }
}

void editor_window_food_properties(const char* title, object::Property* property, int* animation) {
    invalidate_editor_gui();

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

    erase_cursor();
    render_box(BufferMain, x1, y1, x2, y2, EditorPaletteId::WINDOW, EditorPaletteId::WINDOW_BORDER);
    EditorBlackFont->write_centered(BufferMain, (x1 + x2) / 2, y1 + 20, title);
    render_box(BufferMain, box_cancel, EditorPaletteId::WINDOW_BUTTON,
               EditorPaletteId::WINDOW_BORDER);
    EditorBlackFont->write_centered(BufferMain, (box_cancel.x1 + box_cancel.x2) / 2,
                                    box_cancel.y1 + 15, "Cancel");

    EditorBlackFont->write(BufferMain, box_animation.x1 - 162, box_animation.y1 + 15,
                           "Food anim number (1-9):");
    char tmp[20];
    sprintf(tmp, "%d", (int)(*animation + 1));
    render_box(BufferMain, box_animation, EditorPaletteId::WINDOW_INPUT,
               EditorPaletteId::WINDOW_BORDER);
    EditorBlackFont->write_centered(BufferMain, (box_animation.x1 + box_animation.x2) / 2,
                                    box_animation.y1 + 15, tmp);

    for (int i = 0; i < list_length; i++) {
        render_box(BufferMain, box_list, EditorPaletteId::WINDOW_LIST,
                   EditorPaletteId::WINDOW_BORDER);
        int lx = (box_list.x1 + box_list.x2) / 2;
        int ly = box_list.y1 + 15;
        switch (i) {
        case 0:
            EditorBlackFont->write_centered(BufferMain, lx, ly, "Normal Food");
            break;
        case 1:
            EditorBlackFont->write_centered(BufferMain, lx, ly, "Gravity Up");
            break;
        case 2:
            EditorBlackFont->write_centered(BufferMain, lx, ly, "Gravity Down");
            break;
        case 3:
            EditorBlackFont->write_centered(BufferMain, lx, ly, "Gravity Left");
            break;
        case 4:
            EditorBlackFont->write_centered(BufferMain, lx, ly, "Gravity Right");
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
    draw_cursor();

    while (true) {
        handle_events();
        update_and_draw_cursor();
        if (was_key_just_pressed(DIK_ESCAPE) || clicked_box(box_cancel)) {
            return;
        } else if (clicked_box(box_animation)) {
            erase_cursor();
            render_box(BufferMain, box_animation, EditorPaletteId::WINDOW_INPUT,
                       EditorPaletteId::WINDOW_BORDER);
            EditorBlackFont->write_centered(BufferMain, (box_animation.x1 + box_animation.x2) / 2,
                                            box_animation.y1 + 18, "-");
            bltfront(BufferMain, x1, y1, x2, y2);

            empty_keypress_buffer();
            while (true) {
                handle_events();
                if (was_key_just_pressed(DIK_ESCAPE)) {
                    draw_cursor();
                    return;
                }
                while (has_text_input()) {
                    char c = pop_text_input();
                    if (c >= '1' && c <= '9') {
                        *animation = c - '1';
                        LevelChanged = true;
                        draw_cursor();
                        return;
                    }
                }
            }
        } else if (clicked_box(box_list)) {
            int index = (MouseY - box_list.y1) / dy;
            if (index >= list_length) {
                index = list_length - 1;
            }
            LevelChanged = true;
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
}

static void editor_window_level_name(char* level_name) {
    char orig_level_name[LEVEL_NAME_LENGTH + 10];
    strcpy(orig_level_name, level_name);

    erase_cursor();

    blit8(BufferBall, BufferMain);

    invalidate_editor_gui();

    int x1 = 50;
    int y1 = 200;
    int x2 = 600;
    int y2 = 280;

    empty_keypress_buffer();
    int i = 0;
    while (true) {
        handle_events();

        BufferMain->fill_box(x1, y1, x2, y2, EditorPaletteId::LEVEL_NAME_WINDOW);
        BufferMain->line(x1, y1, x2, y1, EditorPaletteId::LEVEL_NAME_WINDOW_BORDER);
        BufferMain->line(x1, y2, x2, y2, EditorPaletteId::LEVEL_NAME_WINDOW_BORDER);
        BufferMain->line(x1, y1, x1, y2, EditorPaletteId::LEVEL_NAME_WINDOW_BORDER);
        BufferMain->line(x2, y1, x2, y2, EditorPaletteId::LEVEL_NAME_WINDOW_BORDER);
        EditorBlackFont->write_centered(
            BufferMain, (x2 + x1) / 2, 220,
            "Type in the name of the level and the designer and press ENTER (ESC to cancel)!");
        int x = (x2 + x1) / 2 - EditorWhiteFont->len(level_name) / 2;
        EditorBlackFont->write(BufferMain, x, 250, level_name);
        char tmp[LEVEL_NAME_LENGTH + 10];
        strcpy(tmp, level_name);
        tmp[i] = 0;
        EditorBlackFont->write(BufferMain, x + EditorWhiteFont->len(tmp), 255, "-");
        bltfront(BufferMain, x1, y1, x2, y2);

        if (was_key_just_pressed(DIK_ESCAPE)) {
            strcpy(level_name, orig_level_name);
            draw_cursor();
            return;
        }
        if (was_key_just_pressed(DIK_RETURN)) {
            if (level_name[0] != 0) {
                draw_cursor();
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
            if (EditorBlackFont->has_char(c) && util::text::is_filename_char(c)) {
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
    invalidate_editor_gui();

    char foreground_name[10];
    char background_name[10];
    char level_name[LEVEL_NAME_LENGTH + 20];
    strcpy(foreground_name, Level->foreground_name);
    strcpy(background_name, Level->background_name);
    strcpy(level_name, Level->level_name);

    finame lgr_name = "";
    strcpy(lgr_name, Level->lgr_name);

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
        update_and_draw_cursor();
        if (was_key_just_pressed(DIK_ESCAPE) || clicked_box(box_cancel)) {
            return;
        } else if (was_key_just_pressed(DIK_RETURN) || clicked_box(box_ok)) {
            if (strcmpi(Level->foreground_name, foreground_name) != 0 ||
                strcmpi(Level->background_name, background_name) != 0 ||
                strcmp(Level->level_name, level_name) != 0) {
                LevelChanged = true;
            }
            strcpy(Level->foreground_name, foreground_name);
            strcpy(Level->background_name, background_name);
            strcpy(Level->level_name, level_name);

            if (strcmpi(lgr_name, Level->lgr_name) != 0) {
                LevelChanged = true;
                strcpy(Level->lgr_name, lgr_name);
                lgrfile::load_lgr_file(Level->lgr_name);
                if (Level->discard_missing_lgr_assets(Lgr)) {
                    dialog_warn_lgr_assets_deleted();
                }
            }
            return;
        } else if (clicked_box(box_foreground)) {
            char null_name[10] = "";
            char mask_name[10] = "";
            if (Lgr->get_mask_index("maskbig") >= 0) {
                strcpy(mask_name, "maskbig");
            }
            editor_window_select_sprite_name(null_name, foreground_name, mask_name,
                                             SpriteType::Texture);
            erase_cursor();
            blit8(BufferMain, BufferBall);
            bltfront(BufferMain);
            draw_cursor();
            rerender = true;
        } else if (clicked_box(box_background)) {
            char null_name[10] = "";
            char mask_name[10] = "";
            if (Lgr->get_mask_index("maskbig") >= 0) {
                strcpy(mask_name, "maskbig");
            }
            editor_window_select_sprite_name(null_name, background_name, mask_name,
                                             SpriteType::Texture);
            erase_cursor();
            blit8(BufferMain, BufferBall);
            bltfront(BufferMain);
            draw_cursor();
            rerender = true;
        } else if (clicked_box(box_levelname)) {
            editor_window_level_name(level_name);
            erase_cursor();
            blit8(BufferMain, BufferBall);
            bltfront(BufferMain);
            draw_cursor();
            rerender = true;
        } else if (clicked_box(box_lgr)) {
            editor_window_choose_lgr(lgr_name);

            erase_cursor();
            blit8(BufferMain, BufferBall);
            bltfront(BufferMain);
            draw_cursor();
            rerender = true;
        }
        if (rerender) {
            rerender = false;

            erase_cursor();

            render_box(BufferMain, x1, y1, x2, y2, EditorPaletteId::WINDOW,
                       EditorPaletteId::WINDOW_BORDER);

            EditorBlackFont->write_centered(BufferMain, (x1 + x2) / 2, y1 + 22, "Level properties");

            render_box(BufferMain, box_ok, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            EditorBlackFont->write_centered(BufferMain, (box_ok.x1 + box_ok.x2) / 2, box_ok.y1 + 15,
                                            "OK");

            render_box(BufferMain, box_cancel, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            EditorBlackFont->write_centered(BufferMain, (box_cancel.x1 + box_cancel.x2) / 2,
                                            box_cancel.y1 + 15, "CANCEL");

            int label_x1 = x1 + 10;
            EditorBlackFont->write(BufferMain, label_x1, box_foreground.y1 + 15, "Foreground:");
            render_box(BufferMain, box_foreground, EditorPaletteId::WINDOW_INPUT,
                       EditorPaletteId::WINDOW_BORDER);
            draw_textbox_left(BufferMain, box_foreground, EditorPaletteId::WINDOW_INPUT,
                              foreground_name);

            EditorBlackFont->write(BufferMain, label_x1, box_background.y1 + 15, "Background:");
            render_box(BufferMain, box_background, EditorPaletteId::WINDOW_INPUT,
                       EditorPaletteId::WINDOW_BORDER);
            draw_textbox_left(BufferMain, box_background, EditorPaletteId::WINDOW_INPUT,
                              background_name);

            EditorBlackFont->write(BufferMain, label_x1, box_levelname.y1 + 15, "Level name:");
            render_box(BufferMain, box_levelname, EditorPaletteId::WINDOW_INPUT,
                       EditorPaletteId::WINDOW_BORDER);
            draw_textbox_left(BufferMain, box_levelname, EditorPaletteId::WINDOW_INPUT, level_name);

            EditorBlackFont->write(BufferMain, label_x1, box_lgr.y1 + 15, "LGR file:");
            render_box(BufferMain, box_lgr, EditorPaletteId::WINDOW_INPUT,
                       EditorPaletteId::WINDOW_BORDER);
            draw_textbox_left(BufferMain, box_lgr, EditorPaletteId::WINDOW_INPUT, lgr_name);

            bltfront(BufferMain, x1, y1, x2, y2);
            draw_cursor();
        }
    }
}

bool ShowPolygons = true;
bool ShowGrassPolygons = true;
bool ShowObjects = true;

void editor_window_view_options() {
    invalidate_editor_gui();

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
        update_and_draw_cursor();
        if (was_key_just_pressed(DIK_ESCAPE) || was_key_just_pressed(DIK_RETURN) ||
            clicked_box(box_ok)) {
            return;
        } else if (clicked_box(box_polygons)) {
            ShowPolygons = !ShowPolygons;
            rerender = true;
        } else if (clicked_box(box_grass)) {
            ShowGrassPolygons = !ShowGrassPolygons;
            rerender = true;
        } else if (clicked_box(box_objects)) {
            ShowObjects = !ShowObjects;
            rerender = true;
        }
        if (rerender) {
            rerender = false;
            erase_cursor();

            render_box(BufferMain, x1, y1, x2, y2, EditorPaletteId::WINDOW,
                       EditorPaletteId::WINDOW_BORDER);

            render_box(BufferMain, box_ok, EditorPaletteId::WINDOW_BUTTON,
                       EditorPaletteId::WINDOW_BORDER);
            EditorBlackFont->write_centered(BufferMain, (box_ok.x1 + box_ok.x2) / 2, box_ok.y1 + 15,
                                            "OK");

            EditorBlackFont->write(BufferMain, label_x1, box_polygons.y1 + 15, "View Polygons");
            render_box(BufferMain, box_polygons, EditorPaletteId::WINDOW_INPUT,
                       EditorPaletteId::WINDOW_BORDER);
            if (ShowPolygons) {
                draw_textbox_centered(BufferMain, box_polygons, EditorPaletteId::WINDOW_INPUT,
                                      "Yes");
            } else {
                draw_textbox_centered(BufferMain, box_polygons, EditorPaletteId::WINDOW_INPUT,
                                      "No");
            }

            EditorBlackFont->write(BufferMain, label_x1, box_grass.y1 + 15, "View Grass");
            render_box(BufferMain, box_grass, EditorPaletteId::WINDOW_INPUT,
                       EditorPaletteId::WINDOW_BORDER);
            if (ShowGrassPolygons) {
                draw_textbox_centered(BufferMain, box_grass, EditorPaletteId::WINDOW_INPUT, "Yes");
            } else {
                draw_textbox_centered(BufferMain, box_grass, EditorPaletteId::WINDOW_INPUT, "No");
            }

            EditorBlackFont->write(BufferMain, label_x1, box_objects.y1 + 15, "View Pictures");
            render_box(BufferMain, box_objects, EditorPaletteId::WINDOW_INPUT,
                       EditorPaletteId::WINDOW_BORDER);
            if (ShowObjects) {
                draw_textbox_centered(BufferMain, box_objects, EditorPaletteId::WINDOW_INPUT,
                                      "Yes");
            } else {
                draw_textbox_centered(BufferMain, box_objects, EditorPaletteId::WINDOW_INPUT, "No");
            }

            bltfront(BufferMain, x1, y1, x2, y2);
            draw_cursor();
        }
    }
}
