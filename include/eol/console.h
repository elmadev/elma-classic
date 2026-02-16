#ifndef EOL_CONSOLE_H
#define EOL_CONSOLE_H

#include <string>
#include <vector>

class console {
  public:
    console();

    void add_line(std::string text);
    void clear();

  private:
    static constexpr int MAX_LINES = 200;

    std::vector<std::string> lines_;
};

#endif
