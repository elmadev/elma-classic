#include "EDITPLAY.H"
#include "EDITTOLT.H"
#include "EDITUJ.H"
#include "keys.h"
#include "LEJATSZO.H"
#include "level.h"
#include "level_load.h"
#include "platform_impl.h"

void editor_play(bool view_map) {
    invalidateegesz();
    if (Valtozott || State->editor_filename[0] == 0 || Ptop->topology_errors) {
        if (!tolt_save()) {
            return;
        }
    }

    if (Ptop->topology_errors) {
        return;
    }

    load_level_play(State->editor_filename);

    Rec1->erase(State->editor_filename);
    Rec2->erase(State->editor_filename);
    lejatszo(State->editor_filename, view_map ? CameraMode::MapViewer : CameraMode::Normal);

    empty_keypress_buffer();

    Pal_editor->set();
}
