#include "ALL.H"
#include "directinput_scancodes.h"

// Customize kiirashoz kell:
const char* kodtobillnev(int billkod) {
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
    case DIK_PREVTRACK:
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
        case DIK_PREVTRACK: return "";
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

typedef int* billmuttomb[NAV_ENTRIES_RIGHT_MAX_LENGTH + 1];

static billmuttomb Pbilltomb0;
static billmuttomb Pbilltomb1;
static billmuttomb Pbilltomb2;

static int Rubrikaszam0 = 6;
static int Rubrikaszam12 = 8;

static void betolt(billmuttomb bmuttomb, int sorszam, const char* szoveg, int* pbillkod) {
    strcpy(NavEntriesLeft[sorszam], szoveg);

    const char* billnev = kodtobillnev(*pbillkod);
    char tmp[20] = "";
    if (!billnev) {
        if (*pbillkod == 0) {
            sprintf(tmp, "???");
        } else {
            sprintf(tmp, "Key code: %d", *pbillkod);
        }
        billnev = tmp;
    }

    strcpy(NavEntriesRight[sorszam], billnev);
    bmuttomb[sorszam] = pbillkod;
}

static void gombotvalaszt(int rubrikaszam, billmuttomb bmuttomb, int sorszam) {
    menu_nav val;
    val.selected_index = sorszam;
    val.x_left = 60;
    val.x_right = 400;
    val.y_entries = 86;
    val.dy = 40;

    strcpy(val.title, "Customize controls");
    strcpy(NavEntriesRight[sorszam], "_");

    val.setup(rubrikaszam, true); // tab-os kiiras

    val.navigate(nullptr, 0, true); // Csak kirajzolja kepet

    // Megnezzuk milyen gombot nyomott le:
    while (1) {
        handle_events();
        for (int i = 1; i < 256; i++) {
            if (mk_getstate(MK_ESC)) {
                return;
            }

            if (i == 28 || i == MK_ESC) {
                continue;
            }

            if (!mk_getstate(i)) {
                continue;
            }

            // Ez a billentyu le van nyomva:

            // char tmp[10];
            // sprintf( tmp, "Bill: %d", i );
            // hiba( tmp );

            // Most meg van nyomva egy gomb:
            // Kiszedjuk mindenhonnan mashonnan ezt a billt:
            for (int j = 3; j < Rubrikaszam0; j++) { // 1-tol megy
                if (*Pbilltomb0[j] == i) {
                    *Pbilltomb0[j] = 0;
                }
            }
            for (int j = 0; j < Rubrikaszam12; j++) { // 0-tol megy
                if (*Pbilltomb1[j] == i) {
                    *Pbilltomb1[j] = 0;
                }
                if (*Pbilltomb2[j] == i) {
                    *Pbilltomb2[j] = 0;
                }
            }
            *bmuttomb[sorszam] = i;
            return;
        }
        // Golyokat leptetjuk:
        val.render();
    }
}

static void bejegyez0(void) {
    strcpy(NavEntriesLeft[0], "Reset all controls to default");
    NavEntriesRight[0][0] = 0;
    strcpy(NavEntriesLeft[1], "Customize Player A");
    NavEntriesRight[1][0] = 0;
    strcpy(NavEntriesLeft[2], "Customize Player B");
    NavEntriesRight[2][0] = 0;
    betolt(Pbilltomb0, 3, "Inc. Screen Size", &State->key_increase_screen_size);
    betolt(Pbilltomb0, 4, "Dec. Screen Size", &State->key_decrease_screen_size);
    betolt(Pbilltomb0, 5, "Make a Screenshot", &State->key_screenshot);
}

static void bejegyez12(billmuttomb bmuttomb, player_keys* pjatopc) {
    betolt(bmuttomb, 0, "Throttle", &pjatopc->gas);
    betolt(bmuttomb, 1, "Brake", &pjatopc->brake);
    betolt(bmuttomb, 2, "Rotate left", &pjatopc->left_volt);
    betolt(bmuttomb, 3, "Rotate right", &pjatopc->right_volt);
    betolt(bmuttomb, 4, "Change direction", &pjatopc->turn);
    betolt(bmuttomb, 5, "Toggle Navigator", &pjatopc->toggle_minimap);
    betolt(bmuttomb, 6, "Toggle Time", &pjatopc->toggle_timer);
    betolt(bmuttomb, 7, "Toggle Show/Hide", &pjatopc->toggle_visibility);
}

static void customizeplayer(billmuttomb bmuttomb, player_keys* pjatopc, const char* ABjel) {
    int kurrens = 0;
    while (1) {
        menu_nav val;
        val.selected_index = kurrens;
        val.x_left = 60;
        val.x_right = 400;
        val.y_entries = 86;
        val.dy = 40;

        strcpy(val.title, "Customize Player ");
        strcat(val.title, ABjel);

        bejegyez12(bmuttomb, pjatopc);

        val.setup(Rubrikaszam12, true); // tab-os kiiras

        kurrens = val.navigate();

        if (kurrens < 0) {
            return;
        }

        if (kurrens >= 0 && kurrens < Rubrikaszam12) {
            gombotvalaszt(Rubrikaszam12, bmuttomb, kurrens);
        }
    }
}

void customize(void) {
    bejegyez12(Pbilltomb1, &State->keys1);
    bejegyez12(Pbilltomb2, &State->keys2);

    int kurrens = 0;
    while (1) {
        menu_nav val;
        val.selected_index = kurrens;
        val.x_left = 60;
        val.x_right = 400;
        val.y_entries = 86;
        val.dy = 40;

        strcpy(val.title, "Customize controls");

        bejegyez0();

        val.setup(Rubrikaszam0, true); // tab-os kiiras

        kurrens = val.navigate();

        if (kurrens < 0) {
            return;
        }

        if (kurrens == 0) {
            State->reset_keys();
            bejegyez12(Pbilltomb1, &State->keys1);
            bejegyez12(Pbilltomb2, &State->keys2);
            bejegyez0();
        }

        if (kurrens == 1) {
            customizeplayer(Pbilltomb1, &State->keys1, "A");
        }

        if (kurrens == 2) {
            customizeplayer(Pbilltomb2, &State->keys2, "B");
        }

        if (kurrens >= 3 && kurrens < Rubrikaszam0) {
            gombotvalaszt(Rubrikaszam0, Pbilltomb0, kurrens);
        }
    }
}
