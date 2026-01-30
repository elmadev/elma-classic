#include "EDITMENU.H"
#include "editor_canvas.h"
#include "editor_dialog.h"
#include "EDITUJ.H"
#include "level.h"

// igaz, ha kilep
int editor_exit_dialog(void) {
    invalidateegesz();
    if (Valtozott) {
        if (dialog("There are unsaved changes in the level file.",
                   "If you exit now, you will loose these changes.", "Do you still want to exit?",
                   DIALOG_BUTTONS, "Yes", "No") == 1) {
            return 0;
        }
    }
    delete Ptop;
    Ptop = NULL;
    return 1;
}

void menu_zoomout(void) {
    zoom_out();
    invalidate();
}
