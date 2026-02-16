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

  private:
    static constexpr int MAX_LINES = 200;
    static constexpr int VISIBLE_LINES = 5;
    static constexpr int LINE_HEIGHT = 12;
    static constexpr int MARGIN_X = 20;
    static constexpr int MARGIN_Y = 2;

    std::vector<std::string> lines_;
};

extern console* Console;

#endif
