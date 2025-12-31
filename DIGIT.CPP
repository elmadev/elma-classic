#include "ALL.H"

static int DigitLineWidth, DigitLineHeight;

static int DigitSpacing, DigitAndColonSpacing, Koztesdx;

static int Digitxsize, Digitysize;

static int ColonOffsetX, ColonOffsetY1, ColonOffsetY2;

static pic8* Dest = NULL;
static unsigned char* ReferencePaletteMap = NULL;

const double TimeToCentiseconds = 100.0 / (182.0 * 0.0024);

// Draw horizontal line for the ingame timer, using the provided palette map
static void horizontal_line(pic8* dest, int x, int y, int size, unsigned char* lookup) {
#ifdef DEBUG
    if (x < 0 || y < 0 || x + size - 1 >= dest->get_width() || y >= dest->get_height()) {
        hiba("horizontal_line x/y out of range!");
    }
#endif
    unsigned char* row = dest->get_row(y);
    for (int i = 0; i < size; i++) {
        row[x + i] = lookup[row[x + i]];
    }
}

// Draw vertical line for the ingame timer, using the provided palette map
static void vertical_line(pic8* dest, int x, int y, int size, unsigned char* lookup) {
#ifdef DEBUG
    if (x < 0 || y < 0 || x >= dest->get_width() || y + size - 1 >= dest->get_height()) {
        hiba("vertical_line x/y out of range!");
    }
#endif
    for (int i = 0; i < size; i++) {
        unsigned char* row = dest->get_row(y + i);
        row[x] = lookup[row[x]];
    }
}

static void draw_colon(int x, int y) {
    horizontal_line(Dest, x + ColonOffsetX, y + ColonOffsetY1, 2, ReferencePaletteMap);
    horizontal_line(Dest, x + ColonOffsetX, y + ColonOffsetY1 + 1, 2, ReferencePaletteMap);

    horizontal_line(Dest, x + ColonOffsetX, y + ColonOffsetY2, 2, ReferencePaletteMap);
    horizontal_line(Dest, x + ColonOffsetX, y + ColonOffsetY2 + 1, 2, ReferencePaletteMap);
}

static void draw_digit_segment(int segment, int x, int y) {
    switch (segment) {
    case 0:
        vertical_line(Dest, x, y + 1, DigitLineHeight, ReferencePaletteMap);
        break;
    case 1:
        vertical_line(Dest, x + DigitLineWidth + 1, y + 1, DigitLineHeight, ReferencePaletteMap);
        break;
    case 2:
        vertical_line(Dest, x, y + DigitLineHeight + 2, DigitLineHeight, ReferencePaletteMap);
        break;
    case 3:
        vertical_line(Dest, x + DigitLineWidth + 1, y + DigitLineHeight + 2, DigitLineHeight,
                      ReferencePaletteMap);
        break;
    case 4:
        horizontal_line(Dest, x + 1, y, DigitLineWidth, ReferencePaletteMap);
        break;
    case 5:
        horizontal_line(Dest, x + 1, y + DigitLineHeight + 1, DigitLineWidth, ReferencePaletteMap);
        break;
    case 6:
        horizontal_line(Dest, x + 1, y + DigitLineHeight + DigitLineHeight + 2, DigitLineWidth,
                        ReferencePaletteMap);
        break;
    default:
        hiba("draw_digit_segment segment out of range!");
        break;
    }
}

static void draw_digit(int c, int x, int y) {
    switch (c) {
    case '0':
        draw_digit_segment(4, x, y);
        draw_digit_segment(0, x, y);
        draw_digit_segment(1, x, y);
        draw_digit_segment(2, x, y);
        draw_digit_segment(3, x, y);
        draw_digit_segment(6, x, y);
        break;
    case '1':
        draw_digit_segment(1, x, y);
        draw_digit_segment(3, x, y);
        break;
    case '2':
        draw_digit_segment(4, x, y);
        draw_digit_segment(0, x, y);
        draw_digit_segment(5, x, y);
        draw_digit_segment(3, x, y);
        draw_digit_segment(6, x, y);
        break;
    case '3':
        draw_digit_segment(4, x, y);
        draw_digit_segment(1, x, y);
        draw_digit_segment(5, x, y);
        draw_digit_segment(3, x, y);
        draw_digit_segment(6, x, y);
        break;
    case '4':
        draw_digit_segment(1, x, y);
        draw_digit_segment(5, x, y);
        draw_digit_segment(2, x, y);
        draw_digit_segment(3, x, y);
        break;
    case '5':
        draw_digit_segment(4, x, y);
        draw_digit_segment(1, x, y);
        draw_digit_segment(5, x, y);
        draw_digit_segment(2, x, y);
        draw_digit_segment(6, x, y);
        break;
    case '6':
        draw_digit_segment(4, x, y);
        draw_digit_segment(0, x, y);
        draw_digit_segment(1, x, y);
        draw_digit_segment(5, x, y);
        draw_digit_segment(2, x, y);
        draw_digit_segment(6, x, y);
        break;
    case '7':
        draw_digit_segment(1, x, y);
        draw_digit_segment(3, x, y);
        draw_digit_segment(6, x, y);
        break;
    case '8':
        draw_digit_segment(4, x, y);
        draw_digit_segment(0, x, y);
        draw_digit_segment(1, x, y);
        draw_digit_segment(5, x, y);
        draw_digit_segment(2, x, y);
        draw_digit_segment(3, x, y);
        draw_digit_segment(6, x, y);
        break;
    case '9':
        draw_digit_segment(4, x, y);
        draw_digit_segment(1, x, y);
        draw_digit_segment(5, x, y);
        draw_digit_segment(2, x, y);
        draw_digit_segment(3, x, y);
        draw_digit_segment(6, x, y);
        break;
    default:
        hiba("draw_digit c out of range!");
    }
}

static void draw_timer(const char* time_text, int x, int y) {
    draw_digit(time_text[0], x, y);
    x += DigitSpacing;
    draw_digit(time_text[1], x, y);
    draw_colon(x, y);
    x += DigitAndColonSpacing;
    draw_digit(time_text[3], x, y);
    x += DigitSpacing;
    draw_digit(time_text[4], x, y);
    draw_colon(x, y);
    x += DigitAndColonSpacing;
    draw_digit(time_text[6], x, y);
    x += DigitSpacing;
    draw_digit(time_text[7], x, y);
}

void draw_timers(const char* best_time_text, double flag_tag_time, double current_time, pic8* dest,
                 int dest_width, int dest_height) {

    Dest = dest;
    ReferencePaletteMap = Plgr->idonegtomb;

    double width_ratio = dest_width / 640.0;
    double height_ratio = dest_height / 480.0;

    // unsigned char szin1 = Plgr->idoszin1;
    // unsigned char szin2 = Plgr->idoszin2;

    int edge_width = 28 * width_ratio;
    int x = edge_width;
    int y = 420 * height_ratio;

    DigitLineWidth = 10 * height_ratio, DigitLineHeight = 16 * height_ratio;

    DigitSpacing = 16 * height_ratio, DigitAndColonSpacing = 24 * height_ratio,
    Koztesdx = 364 * width_ratio;

    Digitxsize = DigitSpacing * 7 + DigitAndColonSpacing * 4 + Koztesdx,
    Digitysize = 2 * DigitLineHeight + 5;

    ColonOffsetX = 17 * height_ratio;
    ColonOffsetY1 = 9 * height_ratio;
    ColonOffsetY2 = 23 * height_ratio;

    long current_time_centiseconds = current_time * TimeToCentiseconds;
    char current_time_text[20];
    centiseconds_to_string(current_time_centiseconds, current_time_text);

    // Elso:
    int best_time_x = x;
    if (best_time_text[0] != 0) {
        draw_timer(best_time_text, best_time_x, y);
    }

    x += DigitSpacing * 3 + DigitAndColonSpacing * 2;

    x += Koztesdx;
    x = dest_width - (edge_width + DigitSpacing * 3 + DigitAndColonSpacing * 3);

    // Masodik:
    draw_timer(current_time_text, x, y);

    // Kirakjuk meg kozepre fogoido-t is:
    if (flag_tag_time >= 0) {
        long flag_tag_time_centiseconds = flag_tag_time * TimeToCentiseconds;
        char flag_tag_time_text[20];
        centiseconds_to_string(flag_tag_time_centiseconds, flag_tag_time_text);
        draw_timer(flag_tag_time_text, (best_time_x + x) >> 1, y);
    }
}
