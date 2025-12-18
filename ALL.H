#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

// Ez Win95-os verzio h file-ja, csak ennek egy szinten kell
// lennie tobbi forras file-lal.

// #include	<i86.h>

// #include	<graph.h>

#include "MAIN.H"
#include "pic8.h"
#include "anim.h"

#include "abc8.h"

#include "VEKT2.H"
#include "physics_init.h"
#include "physics_move.h"
#include "LEPTET.H"
#include "UTKOZES.H"
#include "FOGOCSKA.H"

#include "RECORDER.H"

#include "SZAKASZ.H"
#include "UTKOZES2.H"

#include "LEJATSZO.H"

// Kozos:

#include "KIRAJZOL.H"

#include "WAV.H"
#include "HANG.H"
#include "HANGHIGH.H"

#include "KISKEP.H"
#include "DOBOZ.H"

#include "state.h"

#include "TELJES.H"

#include "JATEKOS.H"
#include "MAINMENU.H"
#include "PLAY.H"
#include "SKIP.H"
#include "best_times.h"

#include "QOPEN.H"

#include "KOZOS.H"
#include "DIGIT.H"

#include "CUSTOM.H"
#include "OPTIONS.H"

#include "TOPOL.H"
#include "EDITUJ.H"
#include "DIALOG.H"
#include "DISPLAY.H"
#include "EDITTOOL.H"
#include "EDITMENU.H"
#include "EDITTOLT.H"

#include "EDITHELP.H"
#include "EDITPLAY.H"
#include "ED_CHECK.H"

#include "KOVETO.H"
#include "PICLIST.H"
#include "LGRFILE.H"

#include "LOAD.H"
#include "ECSET.H"
#include "MENUKEP.H"

#include "VALASZT2.H"
#include "DIALOG32.H"

#include "GOLYOK.H"
#include "GOLYKISZ.H"
#include "GOLYUTK.H"

#include "M_DD.H"
#include "M_K.H"
#include "M_V.H"
#include "M_PIC.H"
