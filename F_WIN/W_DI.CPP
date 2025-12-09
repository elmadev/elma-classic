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

// Customize kiirashoz kell:
char* kodtobillnev(int billkod) {
    switch (billkod) {
    case DIK_1:
        return "1";
    case DIK_2:
        return "2";
    case DIK_3:
        return "3";
    case DIK_4:
        return "4";
    case DIK_5:
        return "5";
    case DIK_6:
        return "6";
    case DIK_7:
        return "7";
    case DIK_8:
        return "8";
    case DIK_9:
        return "9";
    case DIK_0:
        return "0";
    case DIK_MINUS:
        return "-";
    case DIK_EQUALS:
        return "=";
    case DIK_BACK:
        return "<-";
    case DIK_TAB:
        return "TAB";
    case DIK_Q:
        return "Q";
    case DIK_W:
        return "W";
    case DIK_E:
        return "E";
    case DIK_R:
        return "R";
    case DIK_T:
        return "T";
    case DIK_Y:
        return "Y";
    case DIK_U:
        return "U";
    case DIK_I:
        return "I";
    case DIK_O:
        return "O";
    case DIK_P:
        return "P";
    case DIK_LBRACKET:
        return "["; // The [ key
    case DIK_RBRACKET:
        return "]"; // The ] key
    case DIK_RETURN:
        return "ENTER"; // ENTER key on main keyboard
    case DIK_LCONTROL:
        return "LEFT CTRL"; // Left CTRL key
    case DIK_A:
        return "A";
    case DIK_S:
        return "S";
    case DIK_D:
        return "D";
    case DIK_F:
        return "F";
    case DIK_G:
        return "G";
    case DIK_H:
        return "H";
    case DIK_J:
        return "J";
    case DIK_K:
        return "K";
    case DIK_L:
        return "L";
    case DIK_SEMICOLON:
        return ";";
    case DIK_APOSTROPHE:
        return "\"";
    case DIK_GRAVE:
        return "`"; // Grave accent (`) key
    case DIK_LSHIFT:
        return "LEFT SHIFT"; // Left SHIFT key
    case DIK_BACKSLASH:
        return "\\";
    case DIK_Z:
        return "Z";
    case DIK_X:
        return "X";
    case DIK_C:
        return "C";
    case DIK_V:
        return "V";
    case DIK_B:
        return "B";
    case DIK_N:
        return "N";
    case DIK_M:
        return "M";
    case DIK_COMMA:
        return ",";
    case DIK_PERIOD:
        return "."; // On main keyboard
    case DIK_SLASH:
        return "SLASH"; // Forward slash on main keyboard
    case DIK_RSHIFT:
        return "RIGHT SHIFT"; // Right SHIFT key
    case DIK_MULTIPLY:
        return "PAD_*"; // The * key on numeric keypad
    case DIK_LMENU:
        return "LEFT ALT"; // Left ALT key
    case DIK_SPACE:
        return "SPACEBAR"; // SPACEBAR
    case DIK_CAPITAL:
        return "CAPS LOCK"; // CAPS LOCK key
    case DIK_F1:
        return "F1";
    case DIK_F2:
        return "F2";
    case DIK_F3:
        return "F3";
    case DIK_F4:
        return "F4";
    case DIK_F5:
        return "F5";
    case DIK_F6:
        return "F6";
    case DIK_F7:
        return "F7";
    case DIK_F8:
        return "F8";
    case DIK_F9:
        return "F9";
    case DIK_F10:
        return "F10";
    case DIK_NUMLOCK:
        return "NUM LOCK";
    case DIK_SCROLL:
        return "SCROLL LOCK"; // SCROLL LOCK
    case DIK_NUMPAD7:
        return "PAD_HOME";
    case DIK_NUMPAD8:
        return "PAD_UP";
    case DIK_NUMPAD9:
        return "PAD_PGUP";
    case DIK_SUBTRACT:
        return "PAD_-"; // MINUS SIGN on numeric keypad
    case DIK_NUMPAD4:
        return "PAD_LEFT";
    case DIK_NUMPAD5:
        return "PAD_5";
    case DIK_NUMPAD6:
        return "PAD_RIGHT";
    case DIK_ADD:
        return "PAD_+"; // PLUS SIGN on numeric keypad
    case DIK_NUMPAD1:
        return "PAD_END";
    case DIK_NUMPAD2:
        return "PAD_DOWN";
    case DIK_NUMPAD3:
        return "PAD_PGDOWN";
    case DIK_NUMPAD0:
        return "PAD_INS";
    case DIK_DECIMAL:
        return "PAD_DEL"; // PERIOD (decimal point) on numeric keypad
    case DIK_F11:
        return "F11";
    case DIK_F12:
        return "F12";
    case DIK_F13:
        return "F13";
    case DIK_F14:
        return "F14";
    case DIK_F15:
        return "F15";
    case DIK_KANA:
        return "KANA"; // On Japanese keyboard
    case DIK_CONVERT:
        return "CONVERT"; // On Japanese keyboard
    case DIK_NOCONVERT:
        return "NOCONVERT"; // On Japanese keyboard
    case DIK_YEN:
        return "YEN"; // On Japanese keyboard
    case DIK_NUMPADEQUALS:
        return "PAD_="; // On numeric keypad (NEC PC98)
    case DIK_CIRCUMFLEX:
        return "CIRCUMFLEX"; // On Japanese keyboard
    case DIK_AT:
        return "AT"; // On Japanese keyboard
    case DIK_COLON:
        return "COLON"; // On Japanese keyboard
    case DIK_UNDERLINE:
        return "UNDERLINE"; // On Japanese keyboard
    case DIK_KANJI:
        return "KANJI"; // On Japanese keyboard
    case DIK_STOP:
        return "STOP"; // On Japanese keyboard
    case DIK_AX:
        return "AX"; // On Japanese keyboard
    case DIK_UNLABELED:
        return "UNLABELED"; // On Japanese keyboard
    case DIK_NUMPADENTER:
        return "PAD_ENTER";
    case DIK_RCONTROL:
        return "RIGHT CTRL"; // Right CTRL key
    case DIK_NUMPADCOMMA:
        return "COMMA"; // COMMA on NEC PC98 numeric keypad
    case DIK_DIVIDE:
        return "PAD_/"; // Forward slash on numeric keypad
    case DIK_SYSRQ:
        return "SYSRQ";
    case DIK_RMENU:
        return "RIGHT ALT"; // Right ALT key
    case DIK_HOME:
        return "HOME";
    case DIK_UP:
        return "UP ARROW"; // UP ARROW
    case DIK_PRIOR:
        return "PAGEUP"; // PAGE UP
    case DIK_LEFT:
        return "LEFT ARROW"; // LEFT ARROW
    case DIK_RIGHT:
        return "RIGHT ARROW"; // RIGHT ARROW
    case DIK_END:
        return "END";
    case DIK_DOWN:
        return "DOWN ARROW"; // DOWN ARROW
    case DIK_NEXT:
        return "PAGE DOWN"; // PAGE DOWN
    case DIK_INSERT:
        return "INS";
    case DIK_DELETE:
        return "DEL";
    case DIK_LWIN:
        return "LEFT WIN"; // Left Windows key
    case DIK_RWIN:
        return "RIGHT WIN"; // Right Windows key
    case DIK_APPS:
        return "APPLICATION"; // Application key

        // Japan:

        /*case DIK_AT: return "";
        case DIK_CIRCUMFLEX: return "";
        case DIK_COLON: return "";
        case DIK_CONVERT: return "";
        case DIK_KANA: return "";
        case DIK_KANJI: return "";
        case DIK_NOCONVERT: return "";
        case DIK_YEN: return "";
        case DIK_APOSTROPHE: return "";
        case DIK_EQUALS: return "";
        case DIK_GRAVE: return "";

        case DIK_F13: return "";
        case DIK_F14: return "";
        case DIK_F15: return "";
        case DIK_NUMPADCOMMA: return "";
        case DIK_NUMPADEQUALS: return "";
        case DIK_STOP: return "";
        case DIK_UNDERLINE: return "";
        case DIK_BACKSLASH: return "";
        case DIK_GRAVE: return "";
        case DIK_NUMLOCK: return "";
        case DIK_NUMPADENTER: return "";
        case DIK_RCONTROL: return "";
        case DIK_RMENU: return "";
        case DIK_RSHIFT: return "";
        case DIK_SCROLL: return "";

        case DIK_AX: return "";
        case DIK_CONVERT: return "";
        case DIK_KANJI: return "";
        case DIK_NOCONVERT: return "";

        case DIK_NOLABEL: return "";
        */
    }

    // Nem talalta meg billentyuhoz nevet:
    return NULL;
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
