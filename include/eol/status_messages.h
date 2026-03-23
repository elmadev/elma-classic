#ifndef STATUS_MESSAGES_H
#define STATUS_MESSAGES_H

#include <deque>
#include <string>

class pic8;
class abc8;

class status_messages {
  public:
    void add(std::string text);
    void render(pic8& dest, abc8& font);

  private:
    struct status_message {
        std::string text;
        long long created_at = 0;
        long long first_seen = 0;
    };

    std::deque<status_message> messages_;

    void remove_expired();
};

extern status_messages* StatusMessages;

#endif
