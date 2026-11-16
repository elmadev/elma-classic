#ifndef EOL_CHECKPOINT_TABLE_H
#define EOL_CHECKPOINT_TABLE_H

#include "eol/clickable.h"
#include <string>

struct checkpoint_line {
    std::string name;
    std::string time;
    std::string diff;
    std::string speed;
};

class checkpoint_table : public box_overlay_clickable {
    box click_box() const override;
    void set_box(const box& new_box) override;
    void reset_box() override;

    void middle_clicked(const game_mouse& pos) override;

    std::vector<checkpoint_line> lines;

    int name_width;
    int time_width;
    int diff_width;
    int speed_width;

  public:
    checkpoint_table();

    void add_line(checkpoint_line line);
    void reset_lines();

    int width() const;
    void render(pic8& screen, bool force_title) const;
};

extern checkpoint_table CheckpointTable;

#endif
