#ifndef NULL_PROTOCOL_H
#define NULL_PROTOCOL_H

class eol;

class null_protocol {
  public:
    null_protocol(eol&) {}
    void connect() {}
    void tick() {}
};

#endif
