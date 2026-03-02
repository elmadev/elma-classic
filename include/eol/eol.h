#ifndef EOL_H
#define EOL_H

#include "eol/eol_events.h"
#include "eol/eol_table.h"
#include "eol/null_protocol.h"
#include <vector>

using protocol = null_protocol;

class abc8;
class pic8;

class eol {
  public:
    eol();

    void connect() { proto.connect(); }
    void tick() { proto.tick(); }

    void process(const login&);
    void process(const new_kuski&);
    void process(const kuski_logout&);
    void process(const kuski_set_level&);
    void process(const chat_message&);

    void enter_level(const char* level_name, const level* lev);
    void exit_level(const char* level_name, double time, int apple_count, int level_apple_count,
                    bool dead);

    void set_table(TableType);
    void render_table(pic8& dest, abc8& title_font, abc8& data_font) const;

  private:
    void sync_players_online_table();

    protocol proto;
    unsigned int id;
    unsigned int id2;
    std::vector<kuski> kuskis;
    eol_table* cur_table;
    eol_table players_online_table;
};

extern eol* EolClient;

#endif
