#include "eol/status_messages.h"

#include "abc8.h"
#include "pic8.h"
#include "platform_impl.h"

constexpr size_t MAX_STATUS_MESSAGES = 3;
constexpr long long STATUS_MESSAGE_LIFETIME_MS = 8000;
constexpr long long MAX_STATUS_MESSAGE_AGE_MS = 30000;

status_messages* StatusMessages = nullptr;

void status_messages::add(std::string text) {
    messages_.emplace_back(std::move(text), get_milliseconds(), 0);
    if (messages_.size() > MAX_STATUS_MESSAGES) {
        messages_.pop_front();
    }
}

void status_messages::remove_expired() {
    long long now = get_milliseconds();
    while (!messages_.empty()) {
        auto& front = messages_.front();
        bool expired_seen =
            front.first_seen > 0 && now - front.first_seen > STATUS_MESSAGE_LIFETIME_MS;
        bool expired_age = now - front.created_at > MAX_STATUS_MESSAGE_AGE_MS;
        if (!expired_seen && !expired_age) {
            break;
        }
        messages_.pop_front();
    }
}

void status_messages::render(pic8& dest, abc8& font) {
    remove_expired();
    long long now = get_milliseconds();
    for (auto& msg : messages_) {
        if (msg.first_seen == 0) {
            msg.first_seen = now;
        }
    }

    constexpr int STATUS_MESSAGE_MARGIN_TOP = 37;
    const int line_height = font.line_height();
    const int x = dest.get_width() / 4;
    const int top = dest.get_height() - STATUS_MESSAGE_MARGIN_TOP;
    int index = 0;
    for (auto& msg : messages_) {
        int y = top - index * line_height;
        font.write(&dest, x, y, msg.text.c_str());
        index++;
    }
}
