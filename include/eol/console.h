#ifndef EOL_CONSOLE_H
#define EOL_CONSOLE_H

#include <string>
#include <vector>

class abc8;
class pic8;

class console {
  public:
    console();

    void add_line(std::string text);
    void clear();
    void render(pic8* surface, abc8* font);

    bool is_input_active() const;
    void activate_input();
    void deactivate_input();
    void handle_input();

  private:
    static constexpr int MAX_LINES = 200;
    static constexpr int VISIBLE_LINES = 5;
    static constexpr int LINE_HEIGHT = 12;
    static constexpr int MARGIN_X = 20;
    static constexpr int MARGIN_Y = 2;
    static constexpr int MAX_INPUT_LENGTH = 195;

    std::vector<std::string> lines_;
    bool input_active_ = false;
    std::string input_buffer_;
    int cursor_pos_ = 0;
};

extern console* Console;

#endif
