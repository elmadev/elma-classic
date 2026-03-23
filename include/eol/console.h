#ifndef EOL_CONSOLE_H
#define EOL_CONSOLE_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

class abc8;
class pic8;

class console {
  public:
    enum class LineType {
        Chat,
        System,
    };

    struct command {
        std::function<void(std::string_view args)> callback;
    };

    void register_console_commands();
    void add_line(std::string text, LineType type);
    void render(pic8& screen, abc8& font);

    bool is_input_active() const;
    void toggle_active();
    void deactivate_input();
    void handle_input();

    void register_command(std::string_view name,
                          std::function<void(std::string_view args)> callback);
    void register_alias(std::string_view alias, const std::string& cmd);

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
    void submit_input();
    void paste_text(std::string_view text);

    Mode mode = Mode::Chat;
    std::vector<console_line> lines;
    std::unordered_map<std::string, command> commands;
    bool input_active = false;
    std::string input_buffer;
    int cursor_pos = 0;
};

extern console* Console;

#endif
