#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<math.h>
#include	<time.h>
#include	<ctype.h>
#ifdef _WIN32
#include	<io.h>
#else
#include	<unistd.h>
#endif

// Ez Win95-os verzio h file-ja, csak ennek egy szinten kell
// lennie tobbi forras file-lal.

#define W_95

#define WATCOM

//#include	<i86.h>

//#include	<graph.h>

#define VEGLEGES

//#define TEST

#include	"main.h"
#include	"pic8.h"
#include	"anim.h"
//#include	"picmcga.h"
//#include	"picl8ves.h"

#include	"abc8.h"

//#include	"mouse.h"

#include	"rayback.h"

#include	"vekt2.h"
#include	"kor.h"
#include	"adatok.h"
#include	"beallit.h"
#include	"leptet.h"
#include	"utkozes.h"
#include	"fogocska.h"

#include	"recorder.h"

#include	"szakasz.h"
#include	"utkozes2.h"

#include	"lejatszo.h"

// Kozos:

#include	"kirajzol.h"

#include	"wav.h"
#include	"hang.h"
#include	"hanghigh.h"

#include	"kiskep.h"
#include	"doboz.h"

#include	"state.h"

#include	"teljes.h"

#include	"jatekos.h"
#include	"mainmenu.h"
#include	"play.h"
#include	"skip.h"
#include	"besttime.h"

#include	"qopen.h"

#include	"kozos.h"
#include	"digit.h"

#include	"custom.h"
#include	"options.h"
#include	"crccheck.h"

#include	"newuj.h"

#include	"topol.h"
#include	"edituj.h"
#include	"dialog.h"
#include	"display.h"
#include	"edittool.h"
#include	"editmenu.h"
#include	"edittolt.h"

#include	"edithelp.h"
#include	"editplay.h"
#include	"ed_check.h"

#include	"koveto.h"
#include	"piclist.h"
#include	"lgrfile.h"

#include	"load.h"
#include	"ecset.h"
#include	"menukep.h"

#include	"valaszt2.h"
#include	"dialog32.h"


#include	"pubgoly.h"
#include	"golyok.h"
#include	"golykisz.h"
#include	"golyutk.h"


#include	"upgrade.h"
#include	"readme.h"

#include	"m_dd.h"
#include	"m_k.h"
#include	"m_v.h"
#include	"m_pic.h"


#include	"d_nempub.h"


