#ifndef EOL_CONSOLE_H
#define EOL_CONSOLE_H

#include <string>
#include <vector>

class console {
  public:
    enum class LineType {
        Chat,
        System,
    };

    void add_line(std::string text, LineType type);

  private:
    static constexpr size_t MAX_LINES = 1000;

    enum class Mode { Chat, Console };

    struct console_line {
        std::string text;
        LineType type;
    };

    void clear();

    Mode mode = Mode::Chat;
    std::vector<console_line> lines;
};

#endif
