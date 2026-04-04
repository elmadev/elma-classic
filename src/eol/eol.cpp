#include "eol/eol.h"
#include "eol/status_messages.h"
#include <cstring>

static kuski* get_kuski(std::vector<kuski>& kuskis, unsigned int id) {
    for (kuski& k : kuskis) {
        if (k.id == id) {
            return &k;
        }
    }

    return nullptr;
}

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

void eol::process(const new_kuski& nk) { kuskis.push_back(nk.k); }

void eol::process(const kuski_logout& kl) {
    std::erase_if(kuskis, [&kl](const kuski& k) { return k.id == kl.id || k.id == kl.id2; });
}

void eol::process(const kuski_set_level& l) {
    kuski* k = get_kuski(kuskis, l.id);
    if (!k) {
        return;
    }

    strncpy(k->level, (const char*)l.level, MAX_FILENAME_LEN);
}
