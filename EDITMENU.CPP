#include "EDITMENU.H"
#include "editor_canvas.h"
#include "editor_dialog.h"
#include "EDITUJ.H"
#include "level.h"
#include "level_load.h"

bool editor_dialog_exit() {
    invalidateegesz();
    if (Valtozott) {
        if (dialog("There are unsaved changes in the level file.",
                   "If you exit now, you will lose these changes.", "Do you still want to exit?",
                   DIALOG_BUTTONS, "Yes", "No") == 1) {
            return false;
        }
    }
    invalidate_level();
    return true;
}

void editor_zoom_out() {
    zoom_out();
    invalidate();
}
