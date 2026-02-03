#include "EDITPLAY.H"
#include "EDITTOLT.H"
#include "EDITUJ.H"
#include "keys.h"
#include "LEJATSZO.H"
#include "level.h"
#include "LOAD.H"
#include "platform_impl.h"

void editplay(bool view_map) {
    invalidateegesz();
    if (Valtozott || State->editor_filename[0] == 0 || Ptop->topology_errors) {
        if (!tolt_save()) {
            return;
        }
    }

    if (Ptop->topology_errors) {
        return;
    }

    floadlevel_p(State->editor_filename);

    Rec1->erase(State->editor_filename);
    Rec2->erase(State->editor_filename);
    lejatszo(State->editor_filename, view_map ? CameraMode::MapViewer : CameraMode::Normal);

    empty_keypress_buffer();

    // Visszaallitjuk eredeti allapotot:
    Pal_editor->set();

    /*initmou( 640, 480 );

    // Megprobaljuk kozepre hozni egeret:
    for( int i = 0; i < 1000; i++ )
        get_mouse_position( &Moux, &Mouy );
    int mmx = 300;
    int mmy = 255;
    Moux = mmx;
    Mouy = mmy;
    set_mouse_position( mmx, mmy );
    Moux = mmx;
    Mouy = mmy;*/
}
