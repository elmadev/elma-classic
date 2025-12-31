#include "ALL.H"

enum class DigitSegment {
    VERTICAL_BOTTOMLEFT,
    VERTICAL_BOTTOMRIGHT,
    VERTICAL_TOPLEFT,
    VERTICAL_TOPRIGHT,
    HORIZONTAL_BOTTOM,
    HORIZONTAL_MIDDLE,
    HORIZONTAL_TOP,
};

static int DigitLineWidth;
static int DigitLineHeight;

static int DigitSpacing;
static int DigitAndColonSpacing;
static int Koztesdx;

static int Digitxsize;
static int Digitysize;

static int ColonOffsetX;
static int ColonOffsetY1;
static int ColonOffsetY2;

static pic8* Dest = nullptr;
static unsigned char* ReferencePaletteMap = nullptr;

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

// Draw the ":" character
static void draw_colon(int x, int y) {
    horizontal_line(Dest, x + ColonOffsetX, y + ColonOffsetY1, 2, ReferencePaletteMap);
    horizontal_line(Dest, x + ColonOffsetX, y + ColonOffsetY1 + 1, 2, ReferencePaletteMap);

    horizontal_line(Dest, x + ColonOffsetX, y + ColonOffsetY2, 2, ReferencePaletteMap);
    horizontal_line(Dest, x + ColonOffsetX, y + ColonOffsetY2 + 1, 2, ReferencePaletteMap);
}

// Draw one line in the seven segment digit display
static void draw_digit_segment(DigitSegment segment, int x, int y) {
    switch (segment) {
    case DigitSegment::VERTICAL_BOTTOMLEFT:
        vertical_line(Dest, x, y + 1, DigitLineHeight, ReferencePaletteMap);
        break;
    case DigitSegment::VERTICAL_BOTTOMRIGHT:
        vertical_line(Dest, x + DigitLineWidth + 1, y + 1, DigitLineHeight, ReferencePaletteMap);
        break;
    case DigitSegment::VERTICAL_TOPLEFT:
        vertical_line(Dest, x, y + DigitLineHeight + 2, DigitLineHeight, ReferencePaletteMap);
        break;
    case DigitSegment::VERTICAL_TOPRIGHT:
        vertical_line(Dest, x + DigitLineWidth + 1, y + DigitLineHeight + 2, DigitLineHeight,
                      ReferencePaletteMap);
        break;
    case DigitSegment::HORIZONTAL_BOTTOM:
        horizontal_line(Dest, x + 1, y, DigitLineWidth, ReferencePaletteMap);
        break;
    case DigitSegment::HORIZONTAL_MIDDLE:
        horizontal_line(Dest, x + 1, y + DigitLineHeight + 1, DigitLineWidth, ReferencePaletteMap);
        break;
    case DigitSegment::HORIZONTAL_TOP:
        horizontal_line(Dest, x + 1, y + DigitLineHeight + DigitLineHeight + 2, DigitLineWidth,
                        ReferencePaletteMap);
        break;
    default:
        hiba("draw_digit_segment segment out of range!");
        break;
    }
}

// Draw one timer digit
static void draw_digit(int c, int x, int y) {
    switch (c) {
    case '0':
        draw_digit_segment(DigitSegment::HORIZONTAL_BOTTOM, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_BOTTOMLEFT, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_BOTTOMRIGHT, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_TOPLEFT, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_TOPRIGHT, x, y);
        draw_digit_segment(DigitSegment::HORIZONTAL_TOP, x, y);
        break;
    case '1':
        draw_digit_segment(DigitSegment::VERTICAL_BOTTOMRIGHT, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_TOPRIGHT, x, y);
        break;
    case '2':
        draw_digit_segment(DigitSegment::HORIZONTAL_BOTTOM, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_BOTTOMLEFT, x, y);
        draw_digit_segment(DigitSegment::HORIZONTAL_MIDDLE, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_TOPRIGHT, x, y);
        draw_digit_segment(DigitSegment::HORIZONTAL_TOP, x, y);
        break;
    case '3':
        draw_digit_segment(DigitSegment::HORIZONTAL_BOTTOM, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_BOTTOMRIGHT, x, y);
        draw_digit_segment(DigitSegment::HORIZONTAL_MIDDLE, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_TOPRIGHT, x, y);
        draw_digit_segment(DigitSegment::HORIZONTAL_TOP, x, y);
        break;
    case '4':
        draw_digit_segment(DigitSegment::VERTICAL_BOTTOMRIGHT, x, y);
        draw_digit_segment(DigitSegment::HORIZONTAL_MIDDLE, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_TOPLEFT, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_TOPRIGHT, x, y);
        break;
    case '5':
        draw_digit_segment(DigitSegment::HORIZONTAL_BOTTOM, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_BOTTOMRIGHT, x, y);
        draw_digit_segment(DigitSegment::HORIZONTAL_MIDDLE, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_TOPLEFT, x, y);
        draw_digit_segment(DigitSegment::HORIZONTAL_TOP, x, y);
        break;
    case '6':
        draw_digit_segment(DigitSegment::HORIZONTAL_BOTTOM, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_BOTTOMLEFT, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_BOTTOMRIGHT, x, y);
        draw_digit_segment(DigitSegment::HORIZONTAL_MIDDLE, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_TOPLEFT, x, y);
        draw_digit_segment(DigitSegment::HORIZONTAL_TOP, x, y);
        break;
    case '7':
        draw_digit_segment(DigitSegment::VERTICAL_BOTTOMRIGHT, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_TOPRIGHT, x, y);
        draw_digit_segment(DigitSegment::HORIZONTAL_TOP, x, y);
        break;
    case '8':
        draw_digit_segment(DigitSegment::HORIZONTAL_BOTTOM, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_BOTTOMLEFT, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_BOTTOMRIGHT, x, y);
        draw_digit_segment(DigitSegment::HORIZONTAL_MIDDLE, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_TOPLEFT, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_TOPRIGHT, x, y);
        draw_digit_segment(DigitSegment::HORIZONTAL_TOP, x, y);
        break;
    case '9':
        draw_digit_segment(DigitSegment::HORIZONTAL_BOTTOM, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_BOTTOMRIGHT, x, y);
        draw_digit_segment(DigitSegment::HORIZONTAL_MIDDLE, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_TOPLEFT, x, y);
        draw_digit_segment(DigitSegment::VERTICAL_TOPRIGHT, x, y);
        draw_digit_segment(DigitSegment::HORIZONTAL_TOP, x, y);
        break;
    default:
        hiba("draw_digit c out of range!");
    }
}

// Draw one timer (00:00:00)
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

// Draw the 3 times: Best Time, Flag Tag Time, Current Time
void draw_timers(const char* best_time_text, double flag_tag_time, double current_time, pic8* dest,
                 int dest_width, int dest_height) {

    Dest = dest;
    ReferencePaletteMap = Plgr->idonegtomb;

    double width_ratio = dest_width / 640.0;
    double height_ratio = dest_height / 480.0;

    int edge_width = (int)(28.0 * width_ratio);
    int x = edge_width;
    int y = (int)(420.0 * height_ratio);

    DigitLineWidth = (int)(10.0 * height_ratio);
    DigitLineHeight = (int)(16.0 * height_ratio);

    DigitSpacing = (int)(16.0 * height_ratio);
    DigitAndColonSpacing = (int)(24.0 * height_ratio);
    Koztesdx = (int)(364.0 * width_ratio);

    Digitxsize = DigitSpacing * 7 + DigitAndColonSpacing * 4 + Koztesdx;
    Digitysize = 2 * DigitLineHeight + 5;

    ColonOffsetX = (int)(17.0 * height_ratio);
    ColonOffsetY1 = (int)(9.0 * height_ratio);
    ColonOffsetY2 = (int)(23.0 * height_ratio);

    int current_time_centiseconds = (int)(current_time * TimeToCentiseconds);
    char current_time_text[20];
    centiseconds_to_string(current_time_centiseconds, current_time_text);

    int best_time_x = x;
    if (best_time_text[0] != 0) {
        draw_timer(best_time_text, best_time_x, y);
    }

    x += DigitSpacing * 3 + DigitAndColonSpacing * 2;

    x += Koztesdx;
    x = dest_width - (edge_width + DigitSpacing * 3 + DigitAndColonSpacing * 3);

    draw_timer(current_time_text, x, y);

    if (flag_tag_time >= 0) {
        int flag_tag_time_centiseconds = (int)(flag_tag_time * TimeToCentiseconds);
        char flag_tag_time_text[20];
        centiseconds_to_string(flag_tag_time_centiseconds, flag_tag_time_text);
        draw_timer(flag_tag_time_text, (best_time_x + x) >> 1, y);
    }
}
