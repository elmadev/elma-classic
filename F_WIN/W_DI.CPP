#include "w_all.h"
#include "..\state.h"

static LPDIRECTINPUT g_pdi = NULL;
static LPDIRECTINPUTDEVICE g_pKeyboard = NULL;
// static BYTE DIbuffer[256];

void initDI(void) {
    //	for( int i = 0; i < 256; i++ )
    //		DIbuffer[i] = 0;

    // DirectInput regisztracio:
    HRESULT hr = DirectInputCreate(ghInstance, DIRECTINPUT_VERSION, &g_pdi, NULL);
    if (FAILED(hr)) {
        uzenet("Could not create DirectInput!");
    }
    // DirectInput interface szerzes:
    hr = g_pdi->CreateDevice(GUID_SysKeyboard, &g_pKeyboard, NULL);
    if (FAILED(hr)) {
        uzenet("Could not create DirectInput Device!");
    }
    // Adat formatum megadasa:
    hr = g_pKeyboard->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(hr)) {
        uzenet("Could not set data format for DirectInput!");
    }
    // Kooperacios szint beallitasa:
    hr = g_pKeyboard->SetCooperativeLevel(GhWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
    if (FAILED(hr)) {
        uzenet("Could not set cooperative level for DirectInput!");
    }
}

void releaseDI(void) {
    if (g_pKeyboard) {
        g_pKeyboard->Unacquire();
        g_pKeyboard->Release();
        g_pKeyboard = NULL;
    }
    if (g_pdi) {
        g_pdi->Release();
        g_pdi = NULL;
    }
}

// static int Elsoh = 1;

void refreshDI(BYTE* buffer) {
    // FILE* h = NULL;
    // if( Elsoh )
    //	h = fopen( "refr.txt", "wt" );
    // else
    //	h = fopen( "refr.txt", "a+t" );
    // if( !h )
    //	uzenet( "Nem nyilik file!" );
    // Elsoh = 0;

    // fprintf( h, "Bement.\n" );

    if (!g_pKeyboard) {
        hiba("refreshDI-ben !g_pKeyboard!");
    }

    HRESULT hr = g_pKeyboard->GetDeviceState(256, buffer);

    // if( SUCCEEDED( hr ) )
    //	fprintf( h, "Elso sikeres.\n" );
    // else
    //	fprintf( h, "Elso sikertelen.\n" );

    if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {

        //	fprintf( h, "Bement masodikra.\n" );

        hr = g_pKeyboard->Acquire();

        //	if( SUCCEEDED( hr ) )
        //		fprintf( h, "Acquire sikeres.\n" );
        //	else
        //		fprintf( h, "Acquire sikertelen.\n" );

        hr = g_pKeyboard->GetDeviceState(256, buffer);

        //	if( SUCCEEDED( hr ) )
        //		fprintf( h, "Masodik sikeres.\n" );
        //	else
        //		fprintf( h, "Masodik sikertelen.\n" );
    }

    // for( int i = 0; i < 256; i++ ) {
    //	if( buffer[i] )
    //		fprintf( h, "Lenyomot billentyu: %d\n", i );
    // }

    // fclose( h );

    // if( SUCCEEDED( hr ) )
    //	uzenet( "Sikeres volt!" );
}

/*a	if( !SUCCEEDED( hr ) ) {
        if( hr == DIERR_NOTACQUIRED )
            uzenet( "DIERR_NOTACQUIRED" );
*/
// char tmp[10];
// sprintf( tmp, "%x", hr );
// uzenet( "Nem succeed!: ", tmp );

/*a		switch( hr ) {
case DI_OK:
case DI_NOTATTACHED                  :
//case DI_BUFFEROVERFLOW               :
//case DI_PROPNOEFFECT                 :
//case DI_NOEFFECT:
case DI_POLLEDDEVICE:
case DI_DOWNLOADSKIPPED:
case DI_EFFECTRESTARTED:
case DI_TRUNCATED   :
case DI_TRUNCATEDANDRESTARTED:
case DIERR_OLDDIRECTINPUTVERSION :
case DIERR_BETADIRECTINPUTVERSION :
case DIERR_BADDRIVERVER            :
case DIERR_DEVICENOTREG             :
case DIERR_NOTFOUND                  :
//case DIERR_OBJECTNOTFOUND            :
case DIERR_INVALIDPARAM              :
case DIERR_NOINTERFACE               :
case DIERR_GENERIC                   :
case DIERR_OUTOFMEMORY               :
case DIERR_UNSUPPORTED               :
case DIERR_NOTINITIALIZED            :
case DIERR_ALREADYINITIALIZED        :
case DIERR_NOAGGREGATION             :
    uzenet( "Elso felben!" );
case DIERR_OTHERAPPHASPRIO:
case DIERR_INPUTLOST       :
case DIERR_ACQUIRED         :
    uzenet( "Elso nyolcadban!" );
case DIERR_NOTACQUIRED       :
    uzenet( "1!" );
//case DIERR_READONLY           :
//case DIERR_HANDLEEXISTS        :
case E_PENDING     :
    uzenet( "2!" );
case DIERR_INSUFFICIENTPRIVS     :
    uzenet( "3!" );
case DIERR_DEVICEFULL          :
    uzenet( "Harmadik negyedben!" );
case DIERR_MOREDATA             :
case DIERR_NOTDOWNLOADED         :
case DIERR_HASEFFECTS             :
case DIERR_NOTEXCLUSIVEACQUIRED:
case DIERR_INCOMPLETEEFFECT     :
case DIERR_NOTBUFFERED           :
case DIERR_EFFECTPLAYING          :
    uzenet( "Megvan!" );
default:
    uzenet( "Nincs meg!" );
}
a*/
/*for( int i = 0; i < 1000; i++ ) {
    if( MAKE_HRESULT( i ) == hr ) {
        char tmp[10];
        itoa( i, tmp, 10 );
        uzenet( "Nem succeed, talalt!: ", tmp );
    }
}
uzenet( "Nem succeed es nem talalt!" );
*/
// a	}

/*	if( !SUCCEEDED( hr ) )
        return;
    g_pKeyboard->GetDeviceState(
            sizeof( DIbuffer ), &DIbuffer );
}*/
// a}

/*int getDIkeystate( int key ) {
    if( key < 0 || key > 255 )
        hiba( "getDIkeystate-ben key < 0 || key > 255!" );
    return DIbuffer[key];
}*/

/*
case DI_OK:
case DI_NOTATTACHED                  :
case DI_BUFFEROVERFLOW               :
case DI_PROPNOEFFECT                 :
case DI_NOEFFECT:
case DI_POLLEDDEVICE:
case DI_DOWNLOADSKIPPED:
case DI_EFFECTRESTARTED:
case DI_TRUNCATED   :
case DI_TRUNCATEDANDRESTARTED:
case DIERR_OLDDIRECTINPUTVERSION :
case DIERR_BETADIRECTINPUTVERSION :
case DIERR_BADDRIVERVER            :
case DIERR_DEVICENOTREG             :
case DIERR_NOTFOUND                  :
case DIERR_OBJECTNOTFOUND            :
case DIERR_INVALIDPARAM              :
case DIERR_NOINTERFACE               :
case DIERR_GENERIC                   :
case DIERR_OUTOFMEMORY               :
case DIERR_UNSUPPORTED               :
case DIERR_NOTINITIALIZED            :
case DIERR_ALREADYINITIALIZED        :
case DIERR_NOAGGREGATION             :
case DIERR_OTHERAPPHASPRIO:
case DIERR_INPUTLOST       :
case DIERR_ACQUIRED         :
case DIERR_NOTACQUIRED       :
case DIERR_READONLY           :
case DIERR_HANDLEEXISTS        :
case E_PENDING                  :
case DIERR_INSUFFICIENTPRIVS     :
case DIERR_DEVICEFULL          :
case DIERR_MOREDATA             :
case DIERR_NOTDOWNLOADED         :
case DIERR_HASEFFECTS             :
case DIERR_NOTEXCLUSIVEACQUIRED:
case DIERR_INCOMPLETEEFFECT     :
case DIERR_NOTBUFFERED           :
case DIERR_EFFECTPLAYING          :
*/

void di_reset_state(state_s* mut) {
    mut->opciok1.billgaz = DIK_UP;
    mut->opciok1.billfek = DIK_DOWN;
    mut->opciok1.billugras1 = DIK_RIGHT;
    mut->opciok1.billugras2 = DIK_LEFT;
    mut->opciok1.billfordul = DIK_SPACE;
    mut->opciok1.billview = DIK_V;
    mut->opciok1.billtime = DIK_T;
    mut->opciok1.billshowkep = DIK_1;

    mut->opciok2.billgaz = DIK_NUMPAD5;
    mut->opciok2.billfek = DIK_NUMPAD2;
    mut->opciok2.billugras1 = DIK_NUMPAD3;
    mut->opciok2.billugras2 = DIK_NUMPAD1;
    mut->opciok2.billfordul = DIK_NUMPAD0;
    mut->opciok2.billview = DIK_B;
    mut->opciok2.billtime = DIK_Y;
    mut->opciok2.billshowkep = DIK_2;

    // Kozos:
    mut->billplussz = DIK_EQUALS;
    mut->billminusz = DIK_MINUS;
    mut->billsnap = DIK_I;
}

// Ezt a ket fv.-t valaszt2.cpp hivja sound volume allitashoz:
/*int itissoundup( int billkod ) {
    switch( billkod ) {
        case DIK_RIGHT:		// "RIGHT ARROW";
        case DIK_EQUALS:	// "=";
        case DIK_NUMPAD6:	// "PAD_RIGHT"
        case DIK_ADD:		// "PAD_+"
            return 1;
    }
    return 0;
}

int proba_i( void ) {
    int a = DIK_LEFT;
    int b = 2;
    a *= b;
    return a;
}

int itissounddown( int billkod ) {
    switch( billkod ) {
        case DIK_MINUS:		// "-";
        case DIK_LEFT:		// "LEFT ARROW";
        case DIK_SUBTRACT:	// "PAD_-";
        case DIK_NUMPAD4:	// "PAD_LEFT";
            return 1;
    }
    return 0;
}*/
