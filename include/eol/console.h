#ifndef EOL_CONSOLE_H
#define EOL_CONSOLE_H

#include <string>
#include <vector>

class abc8;
class pic8;

class console {
  public:
    enum class LineType {
        Chat,
        System,
    };

    void add_line(std::string text, LineType type);
    void render(pic8& screen, abc8& font);

    bool is_input_active() const;
    void toggle_active();
    void deactivate_input();
    void handle_input();

  private:
    static constexpr size_t MAX_LINES = 1000;
    static constexpr int LINE_HEIGHT = 12;
    static constexpr int MARGIN_X = 20;
    static constexpr int MARGIN_Y = 2;
    static constexpr size_t MAX_INPUT_LENGTH = 195;

    enum class Mode { Chat, Console };

    struct console_line {
        std::string text;
        LineType type;
    };

    void clear();
    void activate_input();

    Mode mode = Mode::Chat;
    std::vector<console_line> lines;
    bool input_active = false;
    std::string input_buffer;
    int cursor_pos = 0;
};

extern console* Console;

#endif
