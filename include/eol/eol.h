#ifndef EOL_H
#define EOL_H

#include "eol/null_protocol.h"

using protocol = null_protocol;

class eol {
  public:
    eol();

    void connect() { proto.connect(); }
    void tick() { proto.tick(); }

  private:
    protocol proto;
};

extern eol* EolClient;

#endif
