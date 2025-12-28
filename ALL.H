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
#include "SPRITE.H"
#include "anim.h"

#include "abc8.h"

#include "vect2.h"
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

#include "menu_controls.h"
#include "menu_options.h"

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
#include "menu_pic.h"

#include "menu_nav.h"
#include "menu_dialog.h"

#include "ball.h"
#include "ball_handler.h"
#include "ball_collision.h"

#include "M_K.H"
#include "M_PIC.H"
#include "platform_impl.h"
