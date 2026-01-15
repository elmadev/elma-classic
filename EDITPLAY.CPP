#include "EDITPLAY.H"
#include "EDITTOLT.H"
#include "EDITUJ.H"
#include "LEJATSZO.H"
#include "LOAD.H"
#include "platform_impl.h"
#include "TOPOL.H"

void editplay(void) {
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

    Prec1->erase(State->editor_filename);
    Prec2->erase(State->editor_filename);
    lejatszo(State->editor_filename);

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
