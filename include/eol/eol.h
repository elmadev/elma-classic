#ifndef EOL_H
#define EOL_H

#include "eol/eol_events.h"
#include "eol/null_protocol.h"
#include <vector>

using protocol = null_protocol;

class eol {
  public:
    eol();

    void connect() { proto.connect(); }
    void tick() { proto.tick(); }

    void process(const login&);
    void process(const new_kuski&);
    void process(const kuski_logout&);

  private:
    protocol proto;
    unsigned int id;
    unsigned int id2;
    std::vector<kuski> kuskis;
};

extern eol* EolClient;

#endif
