#include "all.h"

void editplay(void) {
    invalidateegesz();
    if (Valtozott || State->editfilenev[0] == 0 || Ptop->hibas) {
        if (!tolt_save(0)) {
            return;
        }
    }

    if (Ptop->hibas) {
        return;
    }

    floadlevel_p(State->editfilenev);

    Prec1->erase(State->editfilenev);
    Prec2->erase(State->editfilenev);
    lejatszo(State->editfilenev);

    mk_emptychar();

    // Visszaallitjuk eredeti allapotot:
    Pal_editor->set();

    /*initmou( 640, 480 );

    // Megprobaljuk kozepre hozni egeret:
    for( int i = 0; i < 1000; i++ )
        getmou( &Moux, &Mouy );
    int mmx = 300;
    int mmy = 255;
    Moux = mmx;
    Mouy = mmy;
    setmou( mmx, mmy );
    Moux = mmx;
    Mouy = mmy;*/
}
