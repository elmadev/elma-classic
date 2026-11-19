#include "eol/overlays.h"
#include "eol/checkpoint_table.h"
#include "pic/pic8.h"

namespace overlays {

bool Editor = false;

void render(pic8& screen) {
    CheckpointTable.render(screen, Editor);

    if (Editor) {
        CheckpointTable.render_outline(screen);
    }
}

void get_closest(const game_mouse& pos, int& dist, clickable*& closest) {
    if (!Editor) {
        return;
    }
    CheckpointTable.get_closest(pos, dist, closest);
}

} // namespace overlays
