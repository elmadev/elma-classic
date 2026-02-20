#ifndef EOL_TABLE_H
#define EOL_TABLE_H

#include <string>
#include <vector>

class abc8;
class pic8;

class eol_table {
  public:
    enum class Align { Left, Center, Right };

    eol_table(std::string title_)
        : title(std::move(title_)) {}

    void add_column(int width, Align alignment = Align::Left);
    void clear_columns();

    void add_row(std::vector<std::string> values);
    void clear_rows();

    void render(pic8& dest, abc8& title_font, abc8& data_font, Align alignment) const;

  private:
    struct column {
        int width;
        Align alignment;
    };

    std::string title;
    std::vector<column> columns;
    std::vector<std::vector<std::string>> rows;
};

#endif
