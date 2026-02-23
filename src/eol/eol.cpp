#include "eol/eol.h"
#include "eol/status_messages.h"

eol::eol()
    : proto(*this) {}

void eol::process(const login& l) {
    if (l.success) {
        StatusMessages->add("login successful");
        id = l.id;
        id2 = l.id2;
    } else {
        StatusMessages->add("login unsuccessful");
    }
}
