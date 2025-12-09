#include "w_all.h"

LPDIRECTDRAW Pdd = NULL;                // DirectDraw object
LPDIRECTDRAWSURFACE Pddsprimary = NULL; // DirectDraw primary surface
LPDIRECTDRAWSURFACE Pddsback = NULL;    // DirectDraw back surface

// Ha igaz, alulrol indul y (kirajz320 allitja igazba):
int Locky0_alul = 0;

/*void uzenet( char* text1, char* text2, char* text3 ) {
    hiba( text1, text2, text3 );
} */

void ddfinish(void) {
    if (Pdd != NULL) {
        if (Pddsprimary != NULL) {
            Pddsprimary->Release();
            Pddsprimary = NULL;
        }
        // Get normal mode:
        // HRESULT ddrval = Pdd->SetCooperativeLevel( GhWnd, 0 );
        Pdd->Release();
        Pdd = NULL;
    }
}

void ddinit(void) {
    // create the main DirectDraw object:
    HRESULT ddrval = DirectDrawCreate(NULL, &Pdd, NULL);
    if (ddrval != DD_OK) {
        hiba("DirectDrawCreate did not succeed!");
    }

    // Get exclusive mode:
    ddrval = Pdd->SetCooperativeLevel(GhWnd,
                                      // DDSCL_NOWINDOWCHANGES |
                                      DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
    unsigned tmp = ddrval;
    if (ddrval != DD_OK) {
        for (int i = 0; i < 1000; i++) {
            if (MAKE_DDHRESULT(i) == ddrval) {
                int err = i;
                char tmp[10];
                itoa(err, tmp, 10);
                hiba("SetCooperativeLevel did not succeed, talalt!: ", tmp);
            }
        }
        uzenet("SetCooperativeLevel did not succeed!");
    }

    // Set the video mode:
    ddrval = Pdd->SetDisplayMode(640, 480, 8);
    if (ddrval != DD_OK) {
        for (int i = 0; i < 1000; i++) {
            if (MAKE_DDHRESULT(i) == ddrval) {
                int err = i;
                char tmp[10];
                itoa(err, tmp, 10);
                uzenet("SetDisplayMode (setresdd) did not succeed!", tmp);
            }
        }
        uzenet("SetDisplayMode (setresdd) did not succeed!");
    }
    // ddinit setmode utan 1-be megy, hiba figyeli:
    extern int Margrafikus;
    Margrafikus = 1;

    // Create the primary surface with 1 back buffer
    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
    ddsd.dwBackBufferCount = 2;
    ddrval = Pdd->CreateSurface(&ddsd, &Pddsprimary, NULL);
    if (ddrval != DD_OK) {
        // Nem tudta trippla bufferrel megnyitni,
        // most megprobalja duplaval:
        ddsd.dwSize = sizeof(ddsd);
        ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
        ddsd.dwBackBufferCount = 1;
        ddrval = Pdd->CreateSurface(&ddsd, &Pddsprimary, NULL);
        if (ddrval != DD_OK) {
            uzenet("DDraw CreateSurface did not succeed!");
        }
    }

    DDSCAPS ddscaps;
    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
    ddrval = Pddsprimary->GetAttachedSurface(&ddscaps, &Pddsback);

    if (ddrval != DD_OK) {
        uzenet("DDraw GetAttachedSurface did not succeed!");
    }
}

static unsigned char* Mutbuffer[480];
static int Bufflocked = 0;

unsigned char** lockbackbuffer(int fizikai) {
    fizikai++;

    if (Bufflocked) {
        hiba("lockbackbuffer-ben Bufflocked!");
    }
    Bufflocked = 1;

    DDSURFACEDESC ddsd;
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    HRESULT ddrval = DDERR_WASSTILLDRAWING;
    int restored = 0;
    // while( ddrval == DDERR_WASSTILLDRAWING ) {
    while (ddrval != DD_OK) {
        ddrval = Pddsback->Lock(NULL, &ddsd, 0, NULL);
        if (ddrval == DDERR_SURFACELOST) {
            // Access to this surface is being refused because the surface
            // is gone. The DIRECTDRAWSURFACE object representing this
            // surface should have Restore called on it.

            // if( restored )
            //	hiba( "lockbackbuffer-ben tobbszor kene restore!" );

            ddrval = Pddsprimary->Restore();
            if (ddrval != DD_OK) {
                continue;
                /*for( int i = 0; i < 1000; i++ ) {
                    if( MAKE_DDHRESULT( i ) == ddrval ) {
                        int err = i;
                        char tmp[10];
                        itoa( err, tmp, 10 );
                        hiba( "Pddsback->Restore did not succeed, talalt!: ",
                                        tmp );
                    }
                }
                hiba( "Pddsback->Restore did not succeed, nem talalt!" );
                */
            }
            restored = 1;
            ddrval = DDERR_WASSTILLDRAWING;
        }
    }
    /*if( ddrval != DD_OK ) {
        for( int i = 0; i < 1000; i++ ) {
            if( MAKE_DDHRESULT( i ) == ddrval ) {
                int err = i;
                char tmp[10];
                itoa( err, tmp, 10 );
                hiba( "Pddsback->Lock did not succeed, talalt!: ", tmp );
            }
        }
        hiba( "Pddsback->Lock did not succeed, nem talalt!" );
    }*/

    unsigned char* mut = (unsigned char*)ddsd.lpSurface;
    if (Locky0_alul) {
        for (int y = 0; y < 480; y++) {
            Mutbuffer[480 - 1 - y] = mut;
            mut += ddsd.lPitch;
        }
    } else {
        for (int y = 0; y < 480; y++) {
            Mutbuffer[y] = mut;
            mut += ddsd.lPitch;
        }
    }

    return Mutbuffer;
}

void unlockbackbuffer(void) {
    if (!Bufflocked) {
        hiba("unlockbackbuffer-ben !Bufflocked!");
    }
    Bufflocked = 0;

    HRESULT ddrval = Pddsback->Unlock(NULL);
    if (ddrval != DD_OK) {
        hiba("Pddsback->Unlock did not succeed!");
    }
    while (1) {
        ddrval = Pddsprimary->Flip(NULL, 0);
        if (ddrval == DD_OK) {
            break;
        }
        if (ddrval == DDERR_SURFACELOST) {
            ddrval = Pddsprimary->Restore();
            if (ddrval != DD_OK) {
                break;
            }
        }
        if (ddrval != DDERR_WASSTILLDRAWING) {
            break;
        }
    }
}

unsigned char** lockfrontbuffer(void) {
    if (Bufflocked) {
        hiba("lockfrontbuffer-ben Bufflocked!");
    }
    Bufflocked = 1;

    DDSURFACEDESC ddsd;
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    HRESULT ddrval = DDERR_WASSTILLDRAWING;
    int restored = 0;
    while (ddrval == DDERR_WASSTILLDRAWING) {
        ddrval = Pddsprimary->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
        if (ddrval == DDERR_SURFACELOST) {
            // Access to this surface is being refused because the surface
            // is gone. The DIRECTDRAWSURFACE object representing this
            // surface should have Restore called on it.
            // if( restored )
            //	hiba( "lockbackbuffer-ben tobbszor kene restore!" );

            ddrval = Pddsprimary->Restore();
            if (ddrval != DD_OK) {
                for (int i = 0; i < 1000; i++) {
                    if (MAKE_DDHRESULT(i) == ddrval) {
                        int err = i;
                        char tmp[10];
                        itoa(err, tmp, 10);
                        hiba("Pddsback->Restore did not succeed, talalt!: ", tmp);
                    }
                }
                hiba("Pddsback->Restore did not succeed, nem talalt!");
            }
            restored = 1;
            ddrval = DDERR_WASSTILLDRAWING;
        }
    }
    if (ddrval != DD_OK) {
        for (int i = 0; i < 1000; i++) {
            if (MAKE_DDHRESULT(i) == ddrval) {
                int err = i;
                char tmp[10];
                itoa(err, tmp, 10);
                hiba("Pddsback->Lock did not succeed, talalt!: ", tmp);
            }
        }
        hiba("Pddsback->Lock did not succeed, nem talalt!");
    }

    unsigned char* mut = (unsigned char*)ddsd.lpSurface;
    if (Locky0_alul) {
        for (int y = 0; y < 480; y++) {
            Mutbuffer[480 - 1 - y] = mut;
            mut += ddsd.lPitch;
        }
    } else {
        for (int y = 0; y < 480; y++) {
            Mutbuffer[y] = mut;
            mut += ddsd.lPitch;
        }
    }

    return Mutbuffer;
}

void unlockfrontbuffer(void) {
    if (!Bufflocked) {
        hiba("unlockbackbuffer-ben !Bufflocked!");
    }
    Bufflocked = 0;

    HRESULT ddrval = Pddsprimary->Unlock(NULL);
}

// PALETTA:

//	LPDIRECTDRAWPALETTE pddpal;
ddpal::ddpal(unsigned char* tomb) {
    pddpal = NULL;

    PALETTEENTRY pe[256];
    for (int i = 0; i < 256; i++) {
        pe[i].peRed = (unsigned char)(tomb[3 * i] << 2);
        pe[i].peGreen = (unsigned char)(tomb[3 * i + 1] << 2);
        pe[i].peBlue = (unsigned char)(tomb[3 * i + 2] << 2);
    }
    HRESULT ddrval = Pdd->CreatePalette(DDPCAPS_8BIT, pe, (LPDIRECTDRAWPALETTE*)&(pddpal), NULL);
    if (ddrval != DD_OK) {
        hiba("CreatePalette did not succeed!");
    }
}

ddpal::~ddpal(void) {
    if (pddpal) {
        ((LPDIRECTDRAWPALETTE)pddpal)->Release();
    }
}

void ddpal::set(void) {
    HRESULT ddrval = Pddsprimary->SetPalette((LPDIRECTDRAWPALETTE)pddpal);
    // Egyszer ez elojott, ugyhogy kiveszem:
    // if( ddrval != DD_OK )
    //	hiba( "SetPalette did not succeed!" );
}
