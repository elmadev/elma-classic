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

#include "main.h"
#include "SPRITE.H"

#include "vect2.h"
#include "LEPTET.H"
#include "UTKOZES.H"

#include "RECORDER.H"

#include "SZAKASZ.H"
#include "UTKOZES2.H"

#include "LEJATSZO.H"

// Kozos:

#include "KIRAJZOL.H"

#include "WAV.H"
#include "HANG.H"
#include "HANGHIGH.H"

#include "DOBOZ.H"

#include "state.h"

#include "TELJES.H"

#include "JATEKOS.H"
#include "MAINMENU.H"
#include "PLAY.H"

#include "TOPOL.H"

#include "EDITHELP.H"
#include "EDITPLAY.H"
#include "ED_CHECK.H"

#include "platform_impl.h"
