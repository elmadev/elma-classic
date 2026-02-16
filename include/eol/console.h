#ifndef EOL_CONSOLE_H
#define EOL_CONSOLE_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

class abc8;
class pic8;

struct chat_command {
    std::string description;
    std::function<void(std::string_view args)> callback;
};

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

    void register_command(std::string_view name, std::string_view description,
                          std::function<void(std::string_view args)> callback);

    std::function<void(std::string_view text)> on_chat_message;

  private:
    static constexpr int MAX_LINES = 200;
    static constexpr int VISIBLE_LINES = 5;
    static constexpr int LINE_HEIGHT = 12;
    static constexpr int MARGIN_X = 20;
    static constexpr int MARGIN_Y = 2;
    static constexpr int MAX_INPUT_LENGTH = 195;

    void submit_input();

    std::vector<std::string> lines_;
    std::unordered_map<std::string, chat_command> commands_;
    bool input_active_ = false;
    std::string input_buffer_;
    int cursor_pos_ = 0;
};

extern console* Console;

#endif
