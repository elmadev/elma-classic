#include	"w_all.h"

int Win_95 = 1;

// ddinit setmode utan 1-be megy, hiba figyeli:
int Margrafikus = 0;

extern int Waitforretrace; // main.cpp es h-ban ven definialva

// static LRESULT CALLBACK _export WndProc( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam );
static long FAR PASCAL WndProc( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam );

HINSTANCE ghInstance = 0;
HWND GhWnd = 0;

static char szAppName[100] = "Elasto Mania";
static char szCaption[100] = "Elasto Mania";

static int Aktiv = 0;

static void* Dummypvoid = NULL;

/*void jelol( char* text ) {
	FILE* h = fopen( "jelol.txt", "wt" );
	if( h ) {
		fprintf( h, "%s\n", text );
		fclose( h );
	}
}*/

int Nemkellint = 0;

void semmitor( void ) {
	int i = 10;
	i++;
	Nemkellint++;
}

// WinMain allitja be ha iniicializalta DD-t es DI-t:
static int Inicializalva = 0;

DWORD DwDXVersion = 0;

int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,				LPSTR lpszCmdLine, int CmdShow ) {
	/*void __cdecl egysor( unsigned char lyuk,  unsigned char szam, 
			 unsigned char* sor, unsigned char* tomb, 
			 long futox, long futoy, long dx, long dy );
	egysor( 0,  1, NULL, NULL, 2, 3, 4, 5 );
	*/

	/*void segedfv( void );
	segedfv();
	*/

	WNDCLASS wndclass;

	/*semmitor(); //ez itt nem kell
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	semmitor(); semmitor(); semmitor(); semmitor(); semmitor();
	*/

	if( !hPrevInstance ) {
		// Register parent window class:
		wndclass.style				= CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT;
		wndclass.lpfnWndProc 	= (WNDPROC)WndProc;
		wndclass.cbClsExtra		= 0;
		wndclass.cbWndExtra		= 0;
		wndclass.hInstance		= hInstance;
		wndclass.hIcon				= LoadIcon( hInstance, szAppName );
		wndclass.hCursor			= LoadCursor( NULL, IDC_ARROW );
		wndclass.hbrBackground	= (HBRUSH)GetStockObject( GRAY_BRUSH );
		wndclass.lpszMenuName	= (LPSTR)NULL;
		wndclass.lpszClassName	= szAppName;

		if( !RegisterClass( &wndclass ) )
			return 0;
	}

	ghInstance = hInstance;

	//GhWnd = CreateWindowEx( WS_EX_TOPMOST, szAppName, szCaption, WS_POPUP,
	//						0, 0, 600, 400,
	//						(HWND)NULL, (HMENU)NULL, hInstance, (LPVOID)NULL );

	// DirectDraw-hoz ez volt:
	GhWnd = CreateWindowEx( WS_EX_TOPMOST, szAppName,
   						szCaption, WS_POPUP,
							0, 0, 100, 60,
							(HWND)NULL, (HMENU)NULL,
                     hInstance, (LPVOID)NULL );

	/*ghWnd    = CreateWindowEx (WS_EX_TOPMOST, szAppName,
                              szCaption, WS_POPUP,
                              0, 0, 640, 480,
                              (HWND)NULL, (HMENU)NULL,
                              hInstance, (LPVOID)NULL) ;*/

	if ( !GhWnd )
		return( FALSE );

	ShowWindow( GhWnd, CmdShow );
	UpdateWindow( GhWnd );

	Aktiv = 1;

	void initqopen( void );
	initqopen();

	// Verzio ellenorzese:
	void GetDXVersion( LPDWORD pdwDXVersion, LPDWORD pdwDXPlatform );
	DWORD dwDXPlatform;
	GetDXVersion( &DwDXVersion, &dwDXPlatform );
	if( DwDXVersion < 0x300 )
		uzenet( "You need DirectDraw 3.0 or later installed to run this game!" );


	initDI();
	mk_init();
	
	// Ezeket atraktam lejatszoba:
	//starthanghigh();
	//initdsound();
	Mute = 1;
	
	// Ezt hiba miatt kell mar most meghivni:
	void initmenukep1( void );
	void initmenukep2( void );
	initmenukep1();

	mv_check( /*"W_V init utan"*/ );


	//for( int i = 0; i < 500000000; i++ )
	//	semmitor(); 

	//for( int i = 0; i < 2000000; i++ )
	//	mv_check();

	/*while( 1 ) {
		mv_check();
		if( mk_kbhit() ) {
			if( mk_getextchar() == MK_ENTER )
				uzenet( "Enter nyomva!" );
		}
		refreshDI();
		for( int i = 0; i < 256; i++ ) {
			if( getDIkeystate( i ) ) {
				//uzenet( "Van DI state!" );
				char tmp[10];
				sprintf( tmp, "%d", i );

				releaseDI();
				if( i == DIK_ESCAPE )
					uzenet( "Kode = ESC:", tmp );
				else
					uzenet( "Kode != ESC:", tmp );
			}
		}
	}*/

	if( access( "noretrac.inf", 0 ) == 0 )
		Waitforretrace = 0;


	ddinit();
	Inicializalva = 1;

	// Ez csak palettat allitja be:
	initmenukep2();
	
	ShowCursor( 0 );

	void check_kommand( const char* );
	//check_kommand( GetCommandLine() );

   	mv_main();

	// Ide mar nem szabad jonnie, kene valami uzenet!
	hiba( "WinMain vegere erkezett!" );

	Dummypvoid = &lpszCmdLine;

	return 0;
}

void mv_check( /*char* hivta,*/ int nemkellnyomas ) {
	void hang( void /*char* hivta*/ );
	hang( /*hivta*/ );
	int elso = 1; // Hogy meg !Gameloop eseten is bejojjon:
	while( elso || !Aktiv ) {
		elso = 0;

		MSG msg;
		while( !Aktiv || PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) {
			if( !GetMessage( &msg, NULL, 0, 0 ) ) {
				exit( 0 );
			}
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}
	mkw_getDIstate();
	if( nemkellnyomas )
		return;
	mkw_setkeydown();
}

// Ez itt torlendo:
/*void szovegki( int y, char* szoveg ) {
	HDC hdc = GetDC( GhWnd );
	TextOut( hdc, 30, y, szoveg, strlen( szoveg ) );
	ReleaseDC( GhWnd, hdc );
}*/

static int Lmouselent = 0, Rmouselent = 0;

// mv_exit 1-be allitja hogy WM_CLOSE meghivja DestroyWindow-t:
static int Menubollepettki = 0;

static long FAR PASCAL WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	switch( msg ) {
		case WM_CLOSE:
			extern int Editorban_dialnak;
			extern int Valtozott;
			if( Menubollepettki || 
				(!Aktiv && !(Editorban_dialnak && Valtozott))){

				if( !Menubollepettki && Inicializalva ) {
					ddfinish();
					releaseDI();
				}
				DestroyWindow( hWnd );
			}
			break;
		case WM_CREATE:
			PostMessage( hWnd, WM_USER+1000, 0, 0 );
			break;
		case WM_ACTIVATEAPP:
			Aktiv = wParam;
			void invalidateegesz( void );
			invalidateegesz(); // editorhoz
			break;
		case WM_USER+1000:
			//ShowCursor( FALSE );
			break;
		case WM_DESTROY:
			Aktiv = 0;
			//ShowCursor( TRUE );
			PostQuitMessage( 0 );
			break;
		// Billentyuzet uzenetek:
		/*case WM_KEYDOWN:
			mk_wmkeydown( wParam );
			break;
		case WM_KEYUP:
			mk_wmkeyup( wParam );
			break;
		case WM_CHAR:
			mk_wmchar( wParam );
			break;*/
		// Eger:
		case WM_LBUTTONDOWN:
			Lmouselent = 1;
			break;
		case WM_LBUTTONUP:
			Lmouselent = 0;
			break;
		case WM_RBUTTONDOWN:
			Rmouselent = 1;
			break;
		case WM_RBUTTONUP:
			Rmouselent = 0;
			break;


		default:
			return DefWindowProc( hWnd, msg, wParam, lParam );
	}
	return 0;
}

void mv_exit( char* text ) {
	//PostQuitMessage( 0 );
	//mv_check();
	ddfinish();
	releaseDI();
	//MessageBox( NULL, text, "Message", MB_OK | MB_ICONINFORMATION);
	Menubollepettki = 1;
	SendMessage( GhWnd, WM_CLOSE, 0, 0 );

	//exit( 0 );
	while( 1 )
		mv_check( /*"W_V mv_exit-ben"*/ );
}


static double Startido = 0;

// 182*sec-et adja vissza idot tortresszel egyutt!
// d_stoppe.cpp-ben vannak megvalositva:
double mv_stopperido( void ) {
	return timeGetTime()*0.182 - Startido;
}

void mv_startstopper( void ) {
	Startido = timeGetTime()*0.182;
}

static int Hibaban = 0;
static char Tmphibatext[500];

static void directhiba( char* text1, char* text2, char* text3, char* text4 ) {
	FILE* h;
	if( Hibaban )
		h = fopen( "error_b.txt", "wt" );
	else
		h = fopen( "error.txt", "wt" );
	if( h ) {
		fprintf( h, "%s\n", text1 );
		if( text2 )
			fprintf( h, "%s\n", text2 );
		if( text3 )
			fprintf( h, "%s\n", text3 );
		if( text4 )
			fprintf( h, "%s\n", text4 );
		if( Hibaban )
			fprintf( h, "Two errors while processing!\n" );

		fclose( h );
	}
	
	if( Hibaban )
		return;
	Hibaban = 1;

	if( Margrafikus ) {
		if( text4 ) {
			if( strlen( text3 ) + strlen( text4 ) < 490 ) {
				strcpy( Tmphibatext, text3 );
				strcat( Tmphibatext, " " );
				strcat( Tmphibatext, text4 );

				text3 = Tmphibatext;
			}
		}
		void grafikushiba( char* text1, char* text2 = NULL, char* text3 = NULL );
		grafikushiba( text1, text2, text3 );
		while( 1 ) {
			int c = mk_getextchar();
			if( c == MK_ESC || c == MK_ENTER )
				break;
		}
	}
	else {
		char text[200];
		strcpy( text, text1 );
		if( text2 ) {
			strcat( text, "   " );
			strcat( text, text2 );
		}
		if( text3 ) {
			strcat( text, "   " );
			strcat( text, text3 );
		}
		if( text4 ) {
			strcat( text, "   " );
			strcat( text, text4 );
		}
		MessageBox( NULL, text, "Message", MB_OK | MB_ICONINFORMATION);
	}
	mv_exit();
}

void hiba( char* text1, char* text2, char* text3 ) {
	if( access( "message.inf", 0 ) == 0 )
		directhiba( "<<message.inf accessed>>", text1, text2, text3 );
	directhiba( "Sorry, internal error.", NULL, NULL, NULL );
}

void uzenet( char* text1, char* text2, char* text3 ) {
	// about azert kell, hogy kezdo memchecknel ne agazzon el:
	if( strstr( text1, "memory" ) && !strstr( text1, "about" ) ) {
		// Ha memory szerepel text1-ben:
		if( access( "message.inf", 0 ) == 0 ) // Most mem-et is csak inf-re adja:
			directhiba( "message.inf accessed\n", text1, text2, text3 );
		directhiba( "Sorry, out of memory!", NULL, NULL, NULL );
    }
    directhiba( text1, text2, text3, NULL );
}

int s_random( int maximum ) {
	return rand()%maximum;
}

// MOUSE:
void	getmou( int* px, int* py ) {
	POINT pt;
	GetCursorPos( &pt );
	
	*px = pt.x;
	*py = pt.y;
}

void	setmou( int x, int y ) {
	SetCursorPos( x, y );
}

int	getbutbmou( void ) {
	mv_check( /*"W_V getbutbmuo-ban"*/ );
	return Lmouselent;
}

int	getbutjmou( void ) {
	mv_check( /*"W_V getbutjmuo-ban"*/ );
	return Rmouselent;
}

