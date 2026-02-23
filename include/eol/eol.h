#ifndef EOL_H
#define EOL_H

#include "eol/eol_events.h"
#include "eol/null_protocol.h"

using protocol = null_protocol;

class eol {
  public:
    eol();

    void connect() { proto.connect(); }
    void tick() { proto.tick(); }

    void process(const login&);

  private:
    protocol proto;
    unsigned int id;
    unsigned int id2;
};

extern eol* EolClient;

#endif
