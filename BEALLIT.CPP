#include	"all.h"

// Tudom hogy hiba Belsosav-ot adatok.cpp-ben 0.02-re venni, amikor peldaul
// 0.005 is jo lenne. Hiba tovabba biztostalppont regi verziojat hasznalni
// uj helyett, de ezekkel nem ugyanugy viselkedne motor, mint regen.

// Nem engedi vonalon+egykicsi tovabb kereket!:
static void helyigazitas( kor* pk, vekt2* pt ) {
	//Belsosav = 0.02; //adatok.cpp-ben van ez beallitva
	double hossz = abs( pk->r - *pt );
	vekt2 n = (pk->r-*pt)*(1.0/hossz);
	if( hossz < pk->sugar-Belsosav )
		pk->r = pk->r + n*(pk->sugar-Belsosav-hossz);
}

//static double Utodeshatar = 1.5; // Itt mar nem hallatszik szelrol levalas
static double Utodeshatar = 1.5;

// Igazzal ter vissza, ha talppont tenylegesen az:
static int talppontigazitas( kor* pk, vekt2* pt, vekt2 F ) {
	double hossz = abs( pk->r - *pt );
	vekt2 n = (pk->r-*pt)*(1.0/hossz);
	if( n*pk->v > -Elszakadasisebhat && n*F > 0 )
		// Nem talppont:
		return 0;
	// Sebesseg korrigalasa csak n-re meroleges komponensre:
	vekt2 kulonbseg = n*pk->v*n;
	pk->v = pk->v - kulonbseg;
	// Hang:
	double ero = abs( kulonbseg );
	if( ero > Utodeshatar ) {
		//ero = ero/Utodeshatar*0.1;
		ero = ero/0.8*0.1;
		if( ero >= 0.99 )
			ero = 0.99;
		startwavegyujto( WAV_UTODES, ero, -1 );
	}
	return 1;
}

// Igaz, ha talppont t1 (regi modszer):
static int biztostalppont_regi( vekt2 t1, vekt2 t2, kor* pk, vekt2 F, double M ) {
	double hossz = abs( pk->r - t2 );
	vekt2 n = (pk->r-t2)*(1.0/hossz);
	vekt2 n90 = forgatas90fokkal( n );
	double Mtm = M + hossz*n90*F;
	if( (t1-t2)*n90*Mtm < 0 )
		return 0;
	else
		return 1;
} 

// Igaz, ha talppont t1 (uj modszer):
static int biztostalppont_uj( vekt2 t1, vekt2 t2, kor* pk, vekt2 F, double M ) {
	double hossz = abs( pk->r - t2 );
	vekt2 n = (pk->r-t2)*(1.0/hossz);
	vekt2 n90 = forgatas90fokkal( n );
	double Mtm = pk->omega + hossz*n90*pk->v;
	if( (t1-t2)*n90*Mtm < 0 )
		return 0;
	else
		return 1;
}

/*int Voltosszemosas = 0;

int Konvex = 0;
int Konkav = 0;*/

/*int Pnyomva = 0;

kor buf_pk;
vekt2 buf_F;
double buf_M, buf_dt;*/

void beallit( kor* pk, vekt2 F, double M, double dt, int kelltalppont ) {

	/*int betoltkezd = 0;
	if( betoltkezd ) {
		pk = &buf_pk;
		F = buf_F;
		M = buf_M;
		dt = buf_dt;
	}
	else {
		buf_pk = *pk;
		buf_F = F;
		buf_M = M;
		buf_dt = dt;
	} */

	int talppontszam = 0;
	vekt2 t1, t2;
	vonal *pv1 = NULL, *pv2 = NULL;
	if( kelltalppont ) {
		talppontszam = talppontkereses( pk->r, pk->sugar,
											&t1, &t2, &pv1, &pv2 );
		// Egy kis ellenorzes:
		#ifdef TEST
			switch( talppontszam ) {
				case 0: if( pv1 || pv2 ) hiba( "uiwdyhf" ); break;
				case 1: if( !pv1 || pv2 ) hiba( "uihfwdyhf" ); break;
				case 2: if( !pv1 || !pv2 ) hiba( "uiwoighdyhf" ); break;
			}
		#endif
	}


	//if( talppontszam > 1 && pk->r.x < 14.0 &&
	//	strcmp( pk->nev, "Kerek hatso" ) == 0 )
	//	semmi();

	if( talppontszam > 0 )
		helyigazitas( pk, &t1 );
	if( talppontszam > 1 )
		helyigazitas( pk, &t2 );

	// Csak tanulmanyozasra:
	/*if( talppontszam > 1 ) {
		double tav = abs( t1-t2 );
		if( tav < 1.0 )
			//hiba( "tav < 1.0!" );
			Voltosszemosas = 1;
	} */

	// Megprobalja csokkenteni talppontszamok szamat ugy, hogy figyelembe
	// veszi masik talppont koruli elfordulast is!:
	// Uj modszer azt nezi, hogy sebesseg kiforditja-e
	// talppontbol.
	// Csak egy bizonyos sebesseg felett csinaljuk, hogy
	// Allo helyzetben ne forogjon magatol kerek:
	if( talppontszam == 2 && abs( pk->v ) > 1.0 ) {
		if( !biztostalppont_uj( t1, t2, pk, F, M ) ) {
			talppontszam = 1;
			t1 = t2;
		}
		else {
			if( !biztostalppont_uj( t2, t1, pk, F, M ) )
				talppontszam = 1;
		}
	}
	// Megprobalja csokkenteni talppontszamok szamat ugy, hogy figyelembe
	// veszi masik talppont koruli elfordulast is, de regi
	// modszerrel. Regi modszer azt nezi, hogy erok
	// kiforditjak-e talppontbol.
	// De csak kis sebessegeknel csinalja, mert eloszor egy
	// menetben megvarja amig 2 talppont lenullazza
	// sebesseget:
	if( talppontszam == 2 && abs( pk->v ) < 1.0 ) {
		if( !biztostalppont_regi( t1, t2, pk, F, M ) ) {
			talppontszam = 1;
			t1 = t2;
		}
		else {
			if( !biztostalppont_regi( t2, t1, pk, F, M ) ) 
				talppontszam = 1;
		}
	}
	//if( pk->r.x < 14.0 &&
	//	strcmp( pk->nev, "Kerek hatso" ) == 0 )
	//	Figyeldigazitast = 1;

	if( talppontszam == 2 ) {
		if( !talppontigazitas( pk, &t2, F ) ) 
			talppontszam = 1;
	}
	if( talppontszam >= 1 ) {
		if( !talppontigazitas( pk, &t1, F ) ) {
			if( talppontszam == 2 ) {
				talppontszam = 1;
				t1 = t2;
			}
			else
				talppontszam = 0;
		}
	}

	//Figyeldigazitast = 0;

	// Talppontok most mar be vannak allitva!
	/*if( talppontszam >= 1 )
		pk->utoljaraleert = 1;
	else
		pk->utoljaraleert = 0;
	*/
	if( talppontszam == 0 ) {
		// Szabadon forog a levegoben:
		double beta = M/pk->theta;
		pk->omega += beta*dt;
		pk->alfa += pk->omega*dt;
		vekt2 a = F*(1.0/pk->m);
		pk->v = pk->v + a*dt;
		pk->r = pk->r + pk->v*dt;
		return;
	}
	if( talppontszam == 2 ) {
		// Kerek meg van fogva:
		pk->v = Vekt2null;
		pk->omega = 0;
		//Voltosszemosas = 20; //kirajz-ban egy kis teglalaot rajzol ki
		return;
	}

	//if( Pnyomva && pk == &Pmot1->kor2 )
	//	semmi();

	// Egy talppont korul fordul el kerek:
	double hossz = abs( pk->r-t1 );
	vekt2 n = (pk->r-t1)*(1.0/hossz);
	vekt2 n90 = forgatas90fokkal( n );
	pk->omega = pk->v*n90*(1.0/pk->sugar);
	M += F*n90*pk->sugar; // M-hez hozzajon meg ero is, mivel talpponthoz
						  // vonatkoztatjuk!
	double thetaszelso = pk->theta+pk->m*hossz*hossz;
	double beta = M/thetaszelso;

	pk->omega += beta*dt;
	pk->alfa += pk->omega*dt;
	pk->v = pk->omega*pk->sugar*n90;
	pk->r = pk->r + pk->v*dt;
	return;
}

static void vezeto_hatarolas( motorst* pmot, vekt2 i, vekt2 j ) {
	// Konverzio i, j koordinata rendszerbe:
	double x, y;
	if( pmot->hatra_f ) {
		x = i*(pmot->kor1.r-pmot->vezetor);
		y = j*(pmot->vezetor-pmot->kor1.r);
	}
	else {
		x = i*(pmot->vezetor-pmot->kor1.r);
		y = j*(pmot->vezetor-pmot->kor1.r);
	}
	vekt2 r( x, y );

	// Ulessel hatarolja vezetot:
	static vekt2 ar( -0.35, 0.13 );
	//static vekt2 av( 0.2-(-0.35), 0.22-(0.13) );
	static vekt2 av( 0.14-(-0.35), 0.36-(0.13) );

	static vekt2 avnorm( -av.y, av.x );
	static vekt2 avnormegys = egys( avnorm );

	if( (r-ar)*avnormegys < 0.0 ) {
		// Vezeto ules alatt van:
		double lelogas = (r-ar)*avnormegys;
		r = r - avnormegys*lelogas;
	}

	// Felulrol hatarolja vezetot:
	static const double felhat = 0.48; // 0.5
	if( r.y > felhat )
		r.y = felhat;

	// Oldalrol hatarolja vezetot:
	if( r.x < -0.5 )
		r.x = -0.5;

	static const double jobbhat = 0.26;
	if( r.x > jobbhat )
		r.x = jobbhat;

	static const double tavmax_negyz = felhat*felhat;
	static const double tavmax = sqrt( tavmax_negyz );
	if( r.x > 0 && r.y > 0 ) {
		static const double rxszorzo = (felhat/jobbhat)*(felhat/jobbhat);
		double tav_negyz = r.x*r.x*rxszorzo + r.y*r.y;
		if( tav_negyz > tavmax_negyz ) {
			double tav = sqrt( tav_negyz );
			double szorzo = tavmax/tav;
			r.x *= szorzo;
			r.y *= szorzo;
		}
	}

	//r = vekt2( 0.2, 0.22 );
	//r = r + vekt2( -(0.06), (0.14) );

	// Konverzio i, j koordinata rendszerbol:
	if( pmot->hatra_f )
		pmot->vezetor = j*r.y - i*r.x + pmot->kor1.r;
	else
		pmot->vezetor = i*r.x + j*r.y + pmot->kor1.r;
}

void beallitvezeto( motorst* pmot, vekt2 gravitacio, vekt2 i, vekt2 j,
															double dt ) {
	//if( pmot->hatra && j1.x > 0 )
	//	semmi();

	vezeto_hatarolas( pmot, i, j );

	// Rugoero szamitas:
	vekt2 rugor = pmot->kor1.r + j*Kord5y;
	vekt2 rugoeroirany = rugor - pmot->vezetor;
	double rugoerohossz = abs( rugoeroirany );
	if( rugoerohossz < 0.0000001 )
		rugoerohossz = 0.0000001;
	vekt2 rugoeroegyseg = rugoeroirany*(1.0/rugoerohossz);
	double ero = rugoerohossz*Drsugar*5.0;
	vekt2 Frugo = rugoeroegyseg*ero;

	// Surlodasi ero szamitas:
	// Kiszamoljuk motor testenek sebesseget vezetor helyen:
	vekt2 tangens = forgatas90fokkal( pmot->vezetor - pmot->kor1.r );
	vekt2 motseb = tangens*pmot->kor1.omega + pmot->kor1.v;
	vekt2 relv = pmot->vezetov - motseb;
	vekt2 Fsurl = relv*Sr*3.0;

	vekt2 F = Frugo - Fsurl + gravitacio*pmot->kor1.m*G;
	vekt2 a = F * (1.0/pmot->kor1.m); // Egyenlore kerek tomegevel szamolunk
	pmot->vezetov = pmot->vezetov + a*dt;
	pmot->vezetor = pmot->vezetor + pmot->vezetov*dt;
}



