#ifndef NULL_PROTOCOL_H
#define NULL_PROTOCOL_H

#include <cstdint>

class eol;

class null_protocol {
  public:
    null_protocol(eol&) {}
    void connect() {}
    bool connected() const { return false; }
    void disconnect() {}
    void tick() {}
    bool play_offline() const { return true; }
    template <typename T> void send(const T&) {}
    bool in_battle_level() const { return false; }
    bool playing_battle_level() const { return false; }
    uint32_t battle_id() const { return 0; }
};

#endif
