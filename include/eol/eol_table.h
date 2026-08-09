#ifndef EOL_TABLE_H
#define EOL_TABLE_H

#include <string>
#include <vector>

class abc8;
class pic8;

class eol_table {
  public:
    enum class Align { Left, Center, Right };
    // WrapGroups: overflowing rows wrap into additional column groups drawn side by side.
    // NewestRows: no wrapping; only the newest rows that fit on screen are drawn.
    enum class Overflow { WrapGroups, NewestRows };

    eol_table(std::string title_)
        : title(std::move(title_)) {}

    void add_column(int width, Align alignment = Align::Left);
    void clear_columns();

    void add_row(std::vector<std::string> values);
    void clear_rows();

    static int table_y_offset(pic8& dest, abc8& data_font);
    void render(pic8& dest, abc8& title_font, abc8& data_font, Align alignment,
                int reserved_lines) const;

    void set_title(std::string title_) { title = std::move(title_); }
    void set_overflow(Overflow overflow_) { overflow = overflow_; }

  private:
    struct column {
        int width;
        Align alignment;
    };

    std::string title;
    std::vector<column> columns;
    std::vector<std::vector<std::string>> rows;
    Overflow overflow = Overflow::WrapGroups;
};

#endif
