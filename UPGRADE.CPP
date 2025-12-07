#include	"all.h"

// Ha ez define-olva van, Billint-et 0-ra kell allitani:
// Ezzel lehet allitani hogy kulonallo upg file maker legyen-e:
//#define MAKEUPGFILE

// Most elma.res elso 40.000 byte-jat xor-olva reg.res 1.600.000-nal
// kezdodo byte-jaival:
// Ez a fuggveny seekel helma 40.000-edik byte-jara:
#ifdef MAKEUPGFILE
#ifdef W_95
	Ennek a kettonek nem szabad egyutt define-olva lennie.
#endif

static void beirxor40ezret( FILE* hout, FILE* hin, char* oldresnev ) {
	FILE* holdres = fopen( oldresnev, "rb" );
	if( !holdres )
		hiba( "beirxor40ezret-ben nem nyilik file!: ", oldresnev );

	fseek( holdres, 1600000, SEEK_SET );

	fseek( hin, 0, SEEK_SET );
	for( int i = 0; i < 40000; i++ ) {
		int a1 = fgetc( hin );
		if( a1 == EOF )
			hiba( "6742657854" );
		int a2 = fgetc( holdres );
		if( a2 == EOF )
			hiba( "9058437874" );

		int b = a1 ^ a2;

		if( fwrite( &b, 1, 1, hout ) != 1 )
			hiba( "78946682354" );
	}

	fclose( holdres );
}
#endif


// Letrehoz egy elmaupg.res file-t, aminek elso negy byte-ja elma.res hossza.
// Veszi reg.res es upg.res file-ok 1.600.000-nel kezdodo 40.000 byte-jat es
// xor-ozza elma.res elso 40.000 byte-javal. Igy kapott eredmenyt beteszi
// elmaupg.res file tovabbi ket 40.000 byte-jaba (eloszor reg, utana upg),
// majd maradek elma.res file-t bemasolja elmaupg.res file-ba:
// Csak kulonallo exe-kent kell meghivni, ezert kulonben ki kell kommentezni
// belsejet:
void makeupgradefile( void ) {
#ifdef MAKEUPGFILE
	if( Billint )
		hiba( "Billint-et ki kell kapcsolni makeupgradefile eseten!" );

	// Megnezi elma.res hosszat:
	FILE* helma = fopen( "elma.res", "rb" );
	if( !helma )
		hiba( "Nem nyilik elma.res makeupgradefile!" );

	int elmareshossz = 0;
	while( 1 ) {
		int eredm = fgetc( helma );
		if( eredm == EOF )
			break;
		elmareshossz++;
	}

	fseek( helma, 0, SEEK_SET );

	// Megnyitjuk upg file-t:
	FILE* helmaupg = fopen( "elmaupg.res", "wb" );
	if( !helmaupg )
		hiba( "Nem nyilik elmaupg.res makeupgradefile!" );

	// Beirjuk elma.res hosszat elso negy byte-ba:
	if( fwrite( &elmareshossz, 1, 4, helmaupg ) != 4 )
		hiba( "867567325" );

	// Most elma.res elso 40.000 byte-jat xor-olva reg.res 1.600.000-nal
	// kezdodo byte-jaival:
	// Ez a fuggveny seekel helma 40.000-edik byte-jara:
	beirxor40ezret( helmaupg, helma, "reg.res" );

	// Most elma.res elso 40.000 byte-jat xor-olva upg.res 1.600.000-nal
	// kezdodo byte-jaival:
	// Ez a fuggveny seekel helma 40.000-edik byte-jara:
	beirxor40ezret( helmaupg, helma, "upg.res" );

	// Most elma.res 40.000 ezer utani byte-jait beirjuk elmaupg.res-be:
	for( int i = 0; i < elmareshossz - 40000; i++ ) {
		int c = fgetc( helma );
		if( c == EOF )
			hiba( "97824656745" );
		if( fwrite( &c, 1, 1, helmaupg ) != 1 )
			hiba( "65246542674" );
	}

	fclose( helmaupg );
	fclose( helma );

	uzenet( "Eloallitotta elmaupg.res file-t!" );
#endif
}

// Megnezi, hogy letezik-e elma.res file. Ha nem, megnezi hogy letezik-e
// elmaupg.res file. Ha nem hiba hogy nincsen elma.res. Ha igen akkor upgrade:
// Megnezi hogy van-e ervenyes (elso 8 byte) across.res file. Ha nincs, akkor
// hiba hogy upgrade-hez kell egy ilyen file. Megnezi hogy reg vagy upg es
// megfelelo 40.000 byte-tal xor-ozza across.res 1.600.000-nel kezdodo 40.000
// byte-jat es bemasolja elma.res elejere. Vegere meg elmaupg.res veget:
// Csak full verzioban van megvalositva:
void upgradehakell( void ) {
#ifndef SARVARI
	if( access( "files.inf", 0 ) == 0 )
		return;
	if( access( "elma.res", 0 ) == 0 )
		return;
	// Nem letezik elma.res file.

	if( access( "elmaupg.res", 0 ) != 0 )
		uzenet( "ELMA.RES file is missing!" );
	// Letezik elmaupg.res file, tehat upgrade-elni kene.

	if( access( "across.res", 0 ) != 0 )
		uzenet( "This upgrade version needs the across.res file of the Action Supercross 1.2 registered version!",
				"Please copy it to the directory of this game." );
	// Minden file megvan.

	// Ellenorizzuk old res file-t:
	FILE* holdres = fopen( "across.res", "rb" );
	if( !holdres )
		uzenet( "Cannot open for reading file, across.res!" );

	int bytetomb[8] = { 0x6e, 0, 0, 0, 0x41, 0x71, 0x4a, 0x7b };
	for( int i = 0; i < 8; i++ ) {
		int c = 0;
		if( fread( &c, 1, 1, holdres ) != 1 )
			uzenet( "Read error from file, across.res!" );
		if( c != bytetomb[i] )
			uzenet( "The version of the across.res file is invalid.",
					"This upgrade version needs this file from the registered 1.2 version of Action Supercross to run properly!" );
	}

	// Megallapitjuk, hogy registered, vagy upgrade old res file:
	fseek( holdres, 2020, SEEK_SET );
	int c = 0;
	if( fread( &c, 1, 1, holdres ) != 1 )
		uzenet( "Read error from file, across.res!" );
	if( c != 61 && c != 58 )
		uzenet( "The version of the across.res file is invalid.",
				"This upgrade version needs this file from the registered 1.2 version of Action Supercross to run properly!" );
	int upgres = 0;
	if( c == 58 )
		upgres = 1;

	// Elso 40.000 byte-ot eloallitjuk:
	FILE* helma = fopen( "elma.res", "wb" );
	if( !helma )
		uzenet( "Cannot open for writing file, elma.res!",
				"While upgrading." );

	FILE* helmaupg = fopen( "elmaupg.res", "rb" );
	if( !helmaupg )
		uzenet( "Cannot open for reading file, elmaupg.res!",
				"While upgrading." );

	int elmareshossz = 0;
	if( fread( &elmareshossz, 1, 4, helmaupg ) != 4 )
		uzenet( "Read error from file, elmaupg.res!",
				"While upgrading." );

	if( upgres )
		fseek( helmaupg, 40000, SEEK_CUR );

	// Itt tortenik 40.000 byte masolasa:
	fseek( holdres, 1600000, SEEK_SET );
	for( int i = 0; i < 40000; i++ ) {
		int a1 = fgetc( helmaupg );
		if( a1 == EOF )
			uzenet( "Read error from file, elmaupg.res!",
					"While upgrading." );
		int a2 = fgetc( holdres );
		if( a2 == EOF )
			uzenet( "Read error from file, across.res!",
					"While upgrading." );


		int b = a1 ^ a2;

		if( fwrite( &b, 1, 1, helma ) != 1 )
			uzenet( "Write error on file, elma.res!",
					"While upgrading." );
	}

	if( !upgres )
		fseek( helmaupg, 40000, SEEK_CUR );

	// Most tortenik elmaupg.res hatralevo reszenek masolasa elma.res-be:
	for( int i = 0; i < elmareshossz - 40000; i++ ) {
		int c = fgetc( helmaupg );
		if( c == EOF )
			uzenet( "Read error from file, elmaupg.res!",
					"While upgrading." );
		if( fwrite( &c, 1, 1, helma ) != 1 )
			uzenet( "Write error on file, elma.res!",
					"While upgrading." );
	}

	fclose( helmaupg );
	fclose( helma );
	fclose( holdres );
#endif
}
