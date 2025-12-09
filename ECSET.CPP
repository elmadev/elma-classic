#include "all.h"

typedef uintptr_t pixelek_t;

// Regen a file vegen volt egy kikomentezett eget keszit nevu fuggveny is
// hegyekhez, de ezt 99.05.19-en kiszedtem.

//#define FFECSET // Ha definialva, fekete-feheren rajzol

//#define KIIRDARABOT

int Kisordarabszam = 0;

//static pic8* Egtmp = NULL;
//static pic8* Hegy1 = NULL;
//static pic8* Hegy2 = NULL;

ecset* Pecsetalso = NULL;
ecset* Pecsetfelso = NULL;
ecset* Pecsetview = NULL;

#define PIXELMASZK    (0xfffff800) // also 2048 hely ures


/*void vizsgal( ecset* pecset ) {

	mdarab* futo = pecset->msorok[1071];
	int xtomb[40];
	int muttomb[40];
	int kurx = 1;
	int i = 0;
	while( futo ) {
		if( i > 38 )
			hiba( "i > 38!" );

		xtomb[i] = kurx;
		muttomb[i] = (int)futo->pixelek;

		kurx += futo->xsize;

		i++;
		futo = futo->pkov;
	}

	semmi();
}*/

static void hibanincsmem( void ) {
	uzenet( "You do not have enough memory to load this level!",
			"Try to set the graphich detail to low at the options." );
}

mdarab* ecset::newmdarab( void ) {
	if( tombbenkov >= TOMBBENMDARAB ) {
		// Ez a tomb betelt:
		tombbenkov = 0;
		kurtomb->kovtomb = new mdarabtomb;
		if( !kurtomb->kovtomb )
			hibanincsmem();
		kurtomb = kurtomb->kovtomb;
		kurtomb->kovtomb = NULL;
		tombbenkov++;
		mdarab* ret = &kurtomb->tomb[tombbenkov-1];
		ret->tavolsag = 0;
		return ret;
	}
	tombbenkov++;
	mdarab* ret = &kurtomb->tomb[tombbenkov-1];
	ret->tavolsag = 0;
	return ret;
}

static void meghelyez( double* pd ) {
	int pixel = *pd * Arany;
	*pd = (pixel+0.5)/Arany;
}

void ecset::getorigoandsize( void ) {
	// Megkeresi burkolokat:
	Pszak->felsorolasresetszak();
	vonal* psz = Pszak->getnextszak();
	if( !psz )
		hiba( "Nincs egyetlen szakasz sem definialva!" );
	double minx = psz->r.x;
	double maxx = psz->r.x;
	double miny = psz->r.y;
	double maxy = psz->r.y;
	while( psz ) {
		if( psz->r.x < minx )
			minx = psz->r.x;
		if( psz->r.x > maxx )
			maxx = psz->r.x;
		if( psz->r.y < miny )
			miny = psz->r.y;
		if( psz->r.y > maxy )
			maxy = psz->r.y;

		if( psz->r.x+psz->v.x < minx )
			minx = psz->r.x+psz->v.x;
		if( psz->r.x+psz->v.x > maxx )
			maxx = psz->r.x+psz->v.x;
		if( psz->r.y+psz->v.y < miny )
			miny = psz->r.y+psz->v.y;
		if( psz->r.y+psz->v.y > maxy )
			maxy = psz->r.y+psz->v.y;

		psz = Pszak->getnextszak();
	}

	if( view ) {
		// VIEW VIEW VIEW VIEW VIEW VIEW VIEW VIEW VIEW VIEW
		// x-re jol ra lehet hagyni, nem noveli memoria foglalast:
		origo = vekt2( minx-10000/Arany, miny-4000/Arany );
		xsizedb = (maxx+10000/Arany)-origo.x;
		ysizedb = (maxy+4000/Arany)-origo.y;
	}
	else {
		// x-re jol ra lehet hagyni, nem noveli memoria foglalast:
		origo = vekt2( minx-10000/Arany, miny-1000/Arany );
		// Beallitjuk ugy, hogy mindig pixel felezo helyen legyen origo:
		meghelyez( &origo.x );
		meghelyez( &origo.y );

		xsizedb = (maxx+10000/Arany)-origo.x;
		ysizedb = (maxy+1000/Arany)-origo.y;
	}
}

void ecset::addszakasz( vonal* psz ) {
	vekt2 r, v;
	r = psz->r-origo;
	r.x *= Arany;
	r.y *= Arany;
	v.x = psz->v.x*Arany;
	v.y = psz->v.y*Arany;
	if( view ) {
		double szorzo = 1.0/Viewzoom;
		r.x *= szorzo;
		r.y *= szorzo;
		v.x *= szorzo;
		v.y *= szorzo;
	}

	// Egy kicsit modositjuk palyat, hogy motoros egyforman erintse talajt,
	// ha kulonbozo iranyokban all:
	r.x -= 0.5;
	r.y -= 0.5;

	int foldjon = v.y < 0;

	// Alulrol felfele mutasson:
	if( v.y < 0 ) {
		r = r+v;
		v = Vekt2null-v;
	}
	// Nem vizszintes-e?:
	if( v.y < 0.001 )
		return;
	int y1 = r.y+1;
	int y2 = r.y+v.y;

	/*double kozelseg = 0.000001;
	if( fabs( double( y1 )-(r.y+1) ) < kozelseg )
			hiba( "Nagyon kozeli pont!" );
	if( fabs( double( y2 )-(r.y+v.y) ) < kozelseg )
			hiba( "Nagyon kozeli pont!" );*/

	// Egyenes egyenletenek parameterei:
	double m = v.x/v.y;
	double dy1 = r.y;
	double dy2 = r.y+v.y;
	double dx1 = r.x;
	double dx2 = r.x+v.x;
	double a = (dx2*dy1-dx1*dy2)/(dy1-dy2);
	for( int y = y1; y <= y2; y++ ) {
		int x = m*y+a+1.0;
		//if( fabs( double(x)-m*y+a+1.0 ) < kozelseg )
		//	hiba( "Nagyon kozeli pont!" );

		// Hozzacsapjuk hatar pontot sorhoz:
		if( y < 10 || y >= sorszam )
			hiba( "ecset::addszakasz y < 10 || y >= sorszam!" );
		if( x < 10 || x > maxx )
			hiba( "ecset::addszakasz x < 10 || x > maxx!" );

		mdarab* pmd = newmdarab();
		pmd->pkov = NULL;
		pmd->xsize = x;
		if( foldjon )
			pmd->pixelek = (unsigned char*)PX_FOLD;
		else
			pmd->pixelek = (unsigned char*)PX_EG;

		mdarab* pfut = msorok[y];
		if( !pfut )
			hiba( "ecset::addszakasz !pfut!" );
		while( pfut->pkov )
			pfut = pfut->pkov;
		pfut->pkov = pmd;
	}
}

static int sizeraallit( mdarab* elso ) {
	int eleje = elso->xsize;
	mdarab* futo = elso;
	while( futo ) {
		if( futo->pkov )
			futo->xsize = futo->pkov->xsize - futo->xsize;
		else
			futo->xsize = 1000000;

		futo = futo->pkov;
	}
	return eleje;
}

static void rendez( mdarab* elso ) {
	// Megszamoljuk elemeket:
	int szam = 0;
	mdarab* futo = elso;
	while( futo ) {
		futo = futo->pkov;
		szam++;
	}
	if( szam <= 0 )
		hiba( "(ecset) rendez-ben szam <= 0!" );

	if( szam < 2 )
		return;

	// Sorba rendezunk:
	for( int j = 0; j < szam+2; j++ ) {
		futo = elso;
		futo = futo->pkov;
		while( futo->pkov ) {
			mdarab* pkov = futo->pkov;
			if( futo->xsize > pkov->xsize ) {
				// Csere:
				int tmp = futo->xsize;
				futo->xsize = pkov->xsize;
				pkov->xsize = tmp;
				unsigned char* ptmp = futo->pixelek;
				futo->pixelek = pkov->pixelek;
				pkov->pixelek = ptmp;
			}
			futo = pkov;
		}
	}
	// Kikuszoboljuk egy x-en fekvo mdarab-okat:
	mdarab* futoelozo = elso;
	futo = futoelozo->pkov;
	while( futo->pkov ) {
		mdarab* pkov = futo->pkov;
		if( futo->xsize == pkov->xsize ) {
			if( futo->pixelek == pkov->pixelek ) {
				// Azonosak, csak az egyiket vesszuk ki (pkov-et):
				futo->pkov = pkov->pkov;
			}
			else {
				// Kulonbozoek, mindkettot kiszedjuk:
				futoelozo->pkov = pkov->pkov;
			}
			// Visszaallunk lanc legelejere:
			futoelozo = elso;
			futo = futoelozo->pkov;
			if( !futo )
				return;
		}
		else {
			// Nem volt x-juk azonos, megyunk tovabb:
			futoelozo = futo;
			futo = pkov;
		}
	}

	// Kikuszoboljuk egymas utan fekvo, ugyanazt jelenot mdarab-okat:
	futo = elso;
	while( futo->pkov ) {
		mdarab* pkov = futo->pkov;
		if( futo->pixelek == pkov->pixelek ) {
			futo->pkov = pkov->pkov;
			// Visszaallunk lanc legelejere:
			futo = elso;
		}
		else {
			// Nem voltak azonosak, megyunk tovabb:
			futo = pkov;
		}
	}
}

// Rovid folddarabokat atalakitja konkret mutatova:
void ecset::foldmutatocsere( void ) {
	for( int i = 0; i < sorszam; i++ ) {
		mdarab* fut = msorok[i];
		int xpos = kurxposok_A[i];
		if( xpos > 100 )
			hiba( "ecset::foldmutatocsere xpos > 100!" );
		while( fut ) {
			if( (pixelek_t)fut->pixelek == PX_FOLD && fut->xsize < 640 ) {
				int x = xpos%Plgr->foldxmodulus;
				int y = i%Plgr->pfold->getysize();
				fut->pixelek = Plgr->pfold->getptr( y ) + x;
				//fut->pixelek = Plgr->pfold->getptr( 0 ); igy latszik
			}

			xpos += fut->xsize;
			fut = fut->pkov;
		}
	}
}

// Egymas utani azonos texturakat es ureseket lecsereli egy hosszabbra:
void ecset::duplaeliminacio( void ) {
	for( int i = 0; i < sorszam; i++ ) {
		int voltvaltozas = 1;
		while( voltvaltozas ) {
			voltvaltozas = 0;
			mdarab* fut = msorok[i];
			while( fut && fut->pkov) {
				mdarab* kov = fut->pkov;
				if( ((pixelek_t)fut->pixelek & PIXELMASZK) == 0 &&
					fut->pixelek == kov->pixelek ) {
					fut->pkov = kov->pkov;
					fut->xsize += kov->xsize;
					voltvaltozas = 1;
				}

				fut = fut->pkov;
			}
		}
	}
}

void ecset::textura2mutato( void ) {
	for( int y = 0; y < sorszam; y++ ) {
		if( y == 992 )
			semmi();
		mdarab* fut = msorok[y];
		int xpos = kurxposok_A[y];
		if( xpos > 100 )
			hiba( "9865fd" );
		while( fut ) {
			mdarab* kov = fut->pkov;
			int hozzaad = fut->xsize;

			int kepindex = (pixelek_t)fut->pixelek;
			if( ((kepindex & PIXELMASZK) == 0) && (kepindex >= 10) ) {
				kepindex -= 10;
				// Itt van maszk, le kell cserelni:
				#ifdef TEST
					if( kepindex >= Plgr->texturaszam )
						hiba( "ecset::maszkokkitoltese kepindex >= Plgr->kepszam!" );
					if( Plgr->texturak[kepindex].origxsize <= 0 )
						hiba( "7ywijohg!" );
				#endif
				textura* pt = &Plgr->texturak[kepindex];
				pic8* ppic = pt->ppic;
				int megvan = fut->xsize;
				int x = xpos;
				int elso = 1;
				while( megvan > 0 ) {
					if( !elso ) {
						// Beillesztunk egy uj mdarab-ot:
						mdarab* ujdarab = newmdarab();
						ujdarab->pkov = fut->pkov;
						fut->pkov = ujdarab;
						fut = ujdarab;
					}
					elso = 0;

					x %= pt->origxsize;
					int size = megvan;
					if( x + size > ppic->getxsize() )
						size = ppic->getxsize() - x;

					fut->xsize = size;
					fut->pixelek = ppic->getptr( y%ppic->getysize() ) + x;
					if( ((pixelek_t)fut->pixelek & PIXELMASZK) == 0 )
						uzenet( "Memory address is less than 2048 (723)!" );

					x += size;
					megvan -= size;
				}
			}
			xpos += hozzaad;
			fut = fut->pkov;
		}
	}
}

static int megszamol( mdarab* elso ) {
	// Megszamoljuk elemeket:
	int szam = 0;
	mdarab* futo = elso;
	while( futo ) {
		futo = futo->pkov;
		szam++;
	}
	if( szam <= 0 )
		hiba( "(ecset) megszamol-ban szam <= 0!" );
	return szam;
}

int Osszegszam = 0;

void ecset::mutatotlanit( void ) {
	// Megszamoljuk osszes mdarab-ot:
	int szam = 0;
	for( int i = 0; i < sorszam; i++ )
		szam += megszamol( msorok[i] );
	Osszegszam += szam;
	// Most lefoglaljuk uj egybefuggo tombot:
	nagydarabtomb = new darab[szam+10];
	if( !nagydarabtomb )
		hibanincsmem();
	int szammost = 0;
	for( int i = 0; i < sorszam; i++ ) {
		if( i == 1000 )
			semmi();
		if( i == 7792 )
			semmi();
		mdarab* futo = msorok[i];
		sorok[i] = &nagydarabtomb[szammost];
		curdarabok_A[i] = sorok[i];
		curdarabok_B[i] = sorok[i];
		while( futo ) {
			nagydarabtomb[szammost].xsize = futo->xsize;
			nagydarabtomb[szammost].pixelek = futo->pixelek;

			futo = futo->pkov;
			szammost++;
		}
	}
}

// Ptop kerek-jeinek egesz koordinatait kitolti:
void ecset::kitoltfoodkoordokat( void ) {
	for( int i = 0; i < MAXKEREK; i++ ) {
		kerek* pker = Ptop->kerektomb[i];
		if( !pker )
			continue;
		if( view ) {
			pker->rxint_v = (pker->r.x-origo.x)*Arany/Viewzoom;
			pker->ryint_v = (-pker->r.y-origo.y)*Arany/Viewzoom;
		}
		else {
			pker->rxint = (pker->r.x-origo.x)*Arany-OBJEKTUMSUGAR_I;
			pker->ryint = (-pker->r.y-origo.y)*Arany-OBJEKTUMSUGAR_I;
		}
	}
}

enum { FAZIS_FOLD, FAZIS_EG, FAZIS_NEMFOLDEG, FAZIS_URESRE };

mdarab* ecset::beepitegymadarabnyit( mdarab* pmd, unsigned char* ujpixelek,
			int x1, int ujx1, int ujx2,
			int ujtavolsag, mdarab* vege, int* pujavege, int fazis ) {

	if( (pixelek_t)ujpixelek == PX_URES )
		hiba( "ecset::beepitegymadarabnyit-ban nem lehet ureset rarakni!" );

	if( (pixelek_t)pmd->pixelek == PX_URES && fazis != FAZIS_URESRE )
		hiba( "ecset::beepitegymadarabnyit-ban",
			  "(int)ujpixelek == PX_URES && fazis != FAZIS_URESRE!" );

	// Eldontjuk, hogy ki kell-e egyaltalan rakni:
	int regitav = pmd->tavolsag;
	int nemrakjaki = 0;
	if( fazis == FAZIS_FOLD ) {
		if( (pixelek_t)pmd->pixelek == PX_FOLD )
			regitav = 1000000;
		if( (pixelek_t)pmd->pixelek == PX_EG )
			nemrakjaki = 1;
	}
	if( fazis == FAZIS_EG ) {
		ujtavolsag += 10000;
		if( (pixelek_t)pmd->pixelek == PX_FOLD )
			nemrakjaki = 1;
		if( (pixelek_t)pmd->pixelek == PX_EG ) {
			regitav = 1000000;
		}
	}
	if( fazis == FAZIS_NEMFOLDEG )
		if( (pixelek_t)pmd->pixelek == PX_FOLD || (pixelek_t)pmd->pixelek == PX_EG )
			regitav = 1000000;

	if( fazis == FAZIS_URESRE ) {
		if( (pixelek_t)pmd->pixelek == PX_FOLD || (pixelek_t)pmd->pixelek == PX_EG )
			hiba( "ecset::beepitegymadarabnyit fazis == FAZIS_LYUKRA, megis fold vagy eg!" );
		if( (pixelek_t)pmd->pixelek == PX_URES )
			regitav = 1000000;
	}

	if( nemrakjaki || ujtavolsag >= regitav ) {
		*pujavege = 0;
		return pmd;
	}

	// Most mar biztos, hogy kirakjuk:
	int x2 = x1 + pmd->xsize-1;
	// Levagdossuk ujx1, ujx2-t:
	if( ujx1 < x1 )
		ujx1 = x1;
	if( ujx2 > x2 )
		ujx2 = x2;
	if( ujx2 < x1 || ujx1 > x2 )
		hiba( "ecset::beepitegymadarabnyit ujx2 < x1 || ujx1 > x2!" );

	// Most kivalasztjuk melyik esettel allunk szemben:

	//  ------------------------------------
	//  I              XXXXXXXXXXXXXXXXXXXXI
	//  ------------------------------------

	if( x1 != ujx1 && x2 == ujx2 ) {
		mdarab* puj = newmdarab();
		puj->xsize = ujx2-ujx1+1;
		puj->pixelek = ujpixelek;
		puj->tavolsag = ujtavolsag;
		puj->pkov = pmd->pkov;

		pmd->xsize -= puj->xsize;
		pmd->pkov = puj;

		*pujavege = 1;
		return puj;
	}

	//  ------------------------------------
	//  IXXXXXXXXXXXXXXXXXXXX              I
	//  ------------------------------------

	if( x1 == ujx1 && x2 != ujx2 ) {
		if( !vege )
			hiba( "8796kuyg76" );
		// Ha minden megfelel, atallitjuk *pujavege-t:
		if( !(*pujavege) &&
			vege->tavolsag == ujtavolsag &&
			!((pixelek_t)vege->pixelek & PIXELMASZK) &&
			vege->pixelek == ujpixelek )
			*pujavege = 1;
		if( *pujavege ) {
			int ujsize = ujx2-ujx1+1;

			vege->xsize += ujsize;
			pmd->xsize  -= ujsize;
			if( (pixelek_t)pmd->pixelek & PIXELMASZK )
				pmd->pixelek += ujsize;

			*pujavege = 0;
			return pmd;
		}
		else {
			mdarab* puj = newmdarab();
			vege->pkov = puj;

			puj->xsize = ujx2-ujx1+1;
			puj->pixelek = ujpixelek;
			puj->tavolsag = ujtavolsag;
			puj->pkov = pmd;

			pmd->xsize -= puj->xsize;
			if( (pixelek_t)pmd->pixelek & PIXELMASZK )
				pmd->pixelek += puj->xsize;

			*pujavege = 0;
			return pmd;
		}
	}

	//  ------------------------------------
	//  IXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXI
	//  ------------------------------------

	if( x1 == ujx1 && x2 == ujx2 ) {
		if( !vege )
			hiba( "6fg67r57fu6t" );
		// Ha minden megfelel, atallitjuk *pujavege-t:
		if( !(*pujavege) &&
			vege->tavolsag == ujtavolsag &&
			!((pixelek_t)vege->pixelek & PIXELMASZK) &&
			vege->pixelek == ujpixelek )
			*pujavege = 1;
		if( *pujavege ) {
			vege->xsize += pmd->xsize;
			vege->pkov = pmd->pkov;

			*pujavege = 1;
			return vege;
		}
		else {
			pmd->pixelek = ujpixelek;
			pmd->tavolsag = ujtavolsag;

			*pujavege = 1;
			return pmd;
		}
	}

	//  ------------------------------------
	//  I             XXXXXXXXXXXX         I
	//  ------------------------------------

	if( x1 != ujx1 && x2 != ujx2 ) {
		mdarab* puj1 = newmdarab();
		mdarab* puj2 = newmdarab();

		puj1->xsize = ujx2-ujx1+1;
		puj1->pixelek = ujpixelek;
		puj1->tavolsag = ujtavolsag;
		puj1->pkov = puj2;

		puj2->xsize = x2-ujx2;
		if( (pixelek_t)pmd->pixelek & PIXELMASZK )
			puj2->pixelek = pmd->pixelek + (ujx2+1 - x1);
		else
			puj2->pixelek = pmd->pixelek;
		puj2->tavolsag = pmd->tavolsag;
		puj2->pkov = pmd->pkov;

		pmd->xsize -= puj1->xsize + puj2->xsize;
		pmd->pkov = puj1;

		*pujavege = 0;
		return puj2;
	}

	hiba( "ecset::beepitegymadarabnyit nem talalta meg esetet!" );
	return NULL;
}

//static mdarab* Pmddd = NULL;

void ecset::addbytesor( unsigned char* ujpixelek, int ujtavolsag,
								int ujx1, int ujx2, int y, int fazis ) {
	//if( y != 1039 )
	//	return;

	// Beallunk pmd, xpos-sal elso szoba jovo madarab-ra:
	// Visszalepunk amig kell:
	mdarab* pmd = msorok[y];
	//Pmddd = pmd;
	int xpos = kurxposok_A[y];
	if( xpos > 10 )
		hiba( "ecset::addbytesor xpos > 10!" );
	// Elorelepunk amig kell:
	mdarab* vege = NULL;
	while( xpos + pmd->xsize - 1 < ujx1 ) {
		xpos += pmd->xsize;
		vege = pmd;
		pmd = pmd->pkov;
		if( !pmd )
			hiba( "ecset::addbytesor !pmd (1)!" );
	}
	// Most rajta allunk elso mdarabon:
	int ujavege = 0;
	while( 1 ) {
		if( !pmd )
			hiba( "ecset::addbytesor !pmd (2)!" );
		int x1 = xpos;
		int x2 = xpos+pmd->xsize-1;
		if( ujx2 < x1 )
			hiba( "ecset::addbytesor ujx2 < x1!" );

		// Beallitjuk kovetkezo ertekeket:
		int kovmarnincs = 0;
		if( x2+1 > ujx2 )
			kovmarnincs = 1;

		unsigned char* kovujpixelek = ujpixelek;
		if( (pixelek_t)kovujpixelek & PIXELMASZK ) {
			int kimaradxsize = 0; // Elso mdarab eseteben lehetseges csak
			if( ujx1 > x1 )
				kimaradxsize = ujx1 - x1;
			kovujpixelek = ujpixelek + (pmd->xsize-kimaradxsize);
		}

		int kovxpos = xpos+pmd->xsize;
		mdarab* kovpmd = pmd->pkov;

		vege = beepitegymadarabnyit( pmd, ujpixelek, xpos, ujx1, ujx2,
						ujtavolsag, vege, &ujavege, fazis );

		if( kovmarnincs )
			return;
		ujpixelek = kovujpixelek;
		xpos = kovxpos;
		pmd = kovpmd;
	}
}

// Visszaadja hany pixel lyuk szinu x-tol kezdve:
int uresszam( int x, int xsize, unsigned char* sor, unsigned char lyuk ) {
	int szam = 0;
	while( x < xsize && sor[x] == lyuk ) {
		x++;
		szam++;
	}
	return szam;
}

// Visszaadja hany pixel nem lyuk szinu x-tol kezdve:
int teliszam( int x, int xsize, unsigned char* sor, unsigned char lyuk ) {
	int szam = 0;
	while( x < xsize && sor[x] != lyuk ) {
		x++;
		szam++;
	}
	return szam;
}

// addspriteok hivja:
void ecset::addegymaszk( sprite* psp, int index_t, int index_m, int fazis ) {
	// Egy kep berakasa:
	int x1 = (psp->r.x-origo.x)*Arany;
	int y1 = (-psp->r.y-origo.y)*Arany;

	textura* pt = &Plgr->texturak[index_t];
	maszk*   pm = &Plgr->maszkok[index_m];

	int tavolsag = psp->tavolsag;

	int xsize = pm->xsize;
	int ysize = pm->ysize;

	if( x1 < 120 || x1+xsize >= maxx-50 )
		return;

	int i = 0;
	for( int y = 0; y < ysize; y++ ) {
		// Kep egy soranak elintezese:
		int x = 0;
		// Maszkos eljaras:
		while( pm->adatok[i].tipus != ME_SOREMELES ) {
			if( pm->adatok[i].tipus == ME_TELI ) {
				addbytesor( (unsigned char*)(index_t+10), tavolsag,
					x1+x, x1+x+pm->adatok[i].hossz-1,
					y1-y, fazis );
			}
			x += pm->adatok[i].hossz;
			i++;
		}
		i++;
	}
}

void ecset::addspriteok( int fazis ) {
	for( int j = 0; j < MAXSPRITE; j++ ) {
		sprite* psp = Ptop->spritetomb[j];
		if( !psp )
			return;

		// Fazis fuggvenyeben csak bizonyos kepeket rakunk be:
		if( fazis == FAZIS_FOLD && psp->hatarol != HATAROL_G )
			continue;
		if( fazis == FAZIS_EG && psp->hatarol != HATAROL_S )
			continue;
		if( fazis == FAZIS_NEMFOLDEG && psp->hatarol )
			continue;

		if( psp->kepnev[0] == 0 ) {
			// Csak maszkos textura lehet, kulon intezzuk:
			if( !psp->texturanev[0] || !psp->maszknev[0] )
				continue;
			int index_t = Plgr->gettexturaindex( psp->texturanev );
			if( index_t < 0 )
				hiba( "tgferg" );
			int index_m = Plgr->getmaszkindex( psp->maszknev );
			if( index_m < 0 )
				hiba( "6t7ffrge" );

			// Ez egy maszk, innentol teljesen kulon intezzuk el:
			addegymaszk( psp, index_t, index_m, fazis );
			continue;
		}

		// Ez egy kep:
		if( psp->texturanev[0] || psp->maszknev[0] )
			hiba( "gkuhlij" );

		int index = Plgr->getkepindex( psp->kepnev );
		if( index < 0 )
			hiba( "kjhhgfchg" );
		kep* pkep = &Plgr->kepek[index];

		// Egy kep berakasa:
		int x1 = (psp->r.x-origo.x)*Arany;
		int y1 = (-psp->r.y-origo.y)*Arany;

		int ujtavolsag = psp->tavolsag;


		int xsize = pkep->xsize;
		int ysize = pkep->ysize;

		if( x1 < 120 || x1+xsize >= maxx-50 )
			return;

		unsigned char* tomb = pkep->adatok;

		if( ((pixelek_t)tomb & PIXELMASZK) == 0 )
			uzenet( "Memory address is less than 2048 (987)!" );

		// Vegigmegyunk kepen:
		int i = 0; // byte index tombben
		for( int y = 0; y < ysize; y++ ) {
			// Kep egy soranak elintezese:
			int x = 0;
			while( 1 ) {
				// Ures elintezes:
				int uresszam = tomb[i]*256+tomb[i+1];
				i += 2;

				if( uresszam > 60000 )
					break;

				x += uresszam;

				// Teli elintezes:
				int teliszam = tomb[i]*256+tomb[i+1];
				i += 2;

				// Negativ iranyban haladunk y-on (y1-y):
				if( y1-y >= 0 && y1-y < sorszam )
					addbytesor( &tomb[i], ujtavolsag, x1+x, x1+x+teliszam-1,
														y1-y, fazis );

				x += teliszam;
				i += teliszam;
			}
		}
	}
}

// Kovetok beillesztese:

static int getatlaatszoszam( unsigned char* sor, int x, int xsize,
									unsigned char atlatszo ) {
	int sum = 0;
	for( int i = x; i < xsize; i++ ) {
		if( sor[i] == atlatszo )
			sum++;
		else
			return sum;
	}
	return sum;
}

static int getanyagszam( unsigned char* sor, int x, int xsize,
									unsigned char atlatszo ) {
	int sum = 0;
	for( int i = x; i < xsize; i++ ) {
		if( sor[i] != atlatszo )
			sum++;
		else
			return sum;
	}
	return sum;
}


#define KOVVONALHOSSZ (10000)
static int Kovetoytomb[KOVVONALHOSSZ+10];

static const int Keltolas = 20;

void ecset::kovetotextura( koveto* pkov, int index, int xo, int yo, int fazis ) {
	if( !Buffsima )
		hiba( "6dejuhfe" );

	// Most mindig felso:
	int felso = 1;
	int tavolsag = 600;

	char texturanev[20];
	strcpy( texturanev, "qgrass" );

	// Kikeressuk textura indexet:
	int textindex = Plgr->gettexturaindex( texturanev );
	if( textindex < 0 )
		hiba( "iygtyrt" );

	pic8* ppic = pkov->pkeptomb[index];
	int xsize = ppic->getxsize();
	int ysize = ppic->getysize();
	unsigned char atlatszo = ppic->gpixel( 0, 0 );

	// Toroljuk seged kepet:
	// Buffsima: 0 -> atlatszo, 1 -> van textura:
	for( int y = 0; y < ysize; y++ ) {
		for( int x = 0; x < xsize; x++ ) {
			Buffsima->ppixel( x, y, 0 );
		}
	}
	// Feltoltjuk segedkepet:
	for( int x = 0; x < xsize; x++ ) {
		// Minden oszlopot kitoltunk:
		if( felso ) {
			for( int y = 0; y < ysize; y++ ) {
				if( ppic->gpixel( x, y ) == atlatszo )
					Buffsima->ppixel( x, y, 1 );
				else
					break;
			}
		}
		else {
			for( int y = ysize-1; y >= 0; y-- ) {
				if( ppic->gpixel( x, y ) == atlatszo )
					Buffsima->ppixel( x, y, 1 );
				else
					break;
			}
		}
	}

	// Beadjuk textura darabokat ecset-be:
	for( int y = 0; y < ysize; y++ ) {
		int x = 0;
		unsigned char* sor = Buffsima->getptr( ysize-1-y );
		while( x < xsize ) {
			x += getatlaatszoszam( sor, x, xsize, 0 );
			if( x >= xsize )
				break;
			int anyagszam = getanyagszam( sor, x, xsize, 0 );
			if( anyagszam <= 0 ) 		hiba( "8toiurtg" );
			addbytesor( (unsigned char*)(textindex+10), tavolsag,
						xo+x, xo+x+anyagszam-1, yo+y, fazis );
			x += anyagszam;
		}
	}
	// Most beadjuk alul vagy felul megtoldott reszt is:
	if( felso ) {
		// Felso:
		for( int y = 0; y < FUGGTOLDAS; y++ ) {
			addbytesor( (unsigned char*)(textindex+10), tavolsag,
							xo, xo+xsize-1, yo+ysize+y, fazis );
		}
	}
	else {
		// Also:
		for( int y = 0; y < FUGGTOLDAS; y++ ) {
			addbytesor( (unsigned char*)(textindex+10), tavolsag,
							xo, xo+xsize-1, yo-1-y, fazis );
		}
	}
}

void ecset::kiegykovetokep( koveto* pkov, int index, int xo, int yo, int fazis ) {

	int tavolsag = 600;

	pic8* ppic = pkov->pkeptomb[index];
	int xsize = ppic->getxsize();
	int ysize = ppic->getysize();
	unsigned char atlatszo = ppic->gpixel( 0, 0 );

	// Kiszamoljuk kep tetejenek helyet:
	if( pkov->feltomb[index] )
		yo -= Keltolas;
	else
		yo -= ysize - 1 - Keltolas;

	for( int y = 0; y < ysize; y++ ) {
		int x = 0;
		unsigned char* sor = ppic->getptr( ysize-1-y );
		while( x < xsize ) {
			x += getatlaatszoszam( sor, x, xsize, atlatszo );
			if( x >= xsize )
				break;
			int anyagszam = getanyagszam( sor, x, xsize, atlatszo );
			if( anyagszam <= 0 ) 		hiba( "8toiurtg" );
			addbytesor( &sor[x], tavolsag, xo+x, xo+x+anyagszam-1,
														yo+y, fazis );
			x += anyagszam;
		}
	}

	if( xsize > 640 || ysize > 480 ) {
		#ifdef TEST
			hiba( "7yfg4" );
		#endif
		return;
	}

	kovetotextura( pkov, index, xo, yo, fazis );
	//kovetotextura( 0, pkov, pgy, index, xo, yo, fazis );
}

void ecset::kikovetokepek( koveto* pkov, int* ytomb, int hossz,
													int x0, int fazis ) {
	#ifdef TEST
		if( hossz < 1 )
			hiba( "dd387d3e" );
	#else
		if( hossz < 1 )
			return;
	#endif
	int curx = x0;
	int cury = ytomb[0];
	while( 1 ) {
		if( curx >= x0+hossz )
			return; // Befejezte

		// Megkeressuk legjobb kepet:
		int elteres = 10000;
		int joindex = -1;
		int jody = 0;
		for( int i = 0; i < pkov->kepszam; i++ ) {
			int dy = pkov->pkeptomb[i]->getysize();
			if( dy < 2*Keltolas+1 ) {
				char tmp[60];
				sprintf( tmp, "picture's y size is less than %d!",
							2*Keltolas+1 );
				uzenet( "In grass picture ", tmp );
			}
			dy -= 2*Keltolas+1;
			if( !pkov->feltomb[i] )
				dy = -dy;

			int ujx = curx + pkov->pkeptomb[i]->getxsize();
			int vonaly;
			if( ujx >= x0+hossz )
				vonaly = ytomb[hossz-1];
			else
				vonaly = ytomb[ujx-x0];

			int ujelteres = abs( cury+dy - vonaly );

			if( ujelteres < elteres ) {
				elteres = ujelteres;
				joindex = i;
				jody = dy;
			}
		}

		// Megvan legjobb kep:
		if( joindex < 0 )
			hiba( "7f908ur5tg" );

		kiegykovetokep( pkov, joindex, curx, cury, fazis );

		curx += pkov->pkeptomb[joindex]->getxsize();
		cury += jody;
	}
}

void ecset::addkovetok( int fazis ) {
	if( !Plgr->vanfu )
		return;
	for( int i = 0; i < MAXGYURU; i++ ) {
		gyuru* pgy = Ptop->ptomb[i];
		if( !pgy )
			return;
		if( !pgy->koveto )
			continue;

		int hatarol = HATAROL_G;

		// Fazis fuggvenyeben csak bizonyos kepeket rakunk be:
		if( fazis == FAZIS_FOLD && hatarol != HATAROL_G )
			continue;
		if( fazis == FAZIS_EG && hatarol != HATAROL_S )
			continue;
		if( fazis == FAZIS_NEMFOLDEG && hatarol )
			continue;

		koveto* pkov = Plgr->pkoveto;

		int vonalhossz = 0, x0 = 0;
		if( !makevonal( pgy, Kovetoytomb, &vonalhossz, &x0,
					KOVVONALHOSSZ, &origo ) )
			continue;
		if( vonalhossz > KOVVONALHOSSZ )
			hiba( "uh9fdw" );

		// Kirajzolunk egy minta vonalat kepre:
		/*static unsigned char kovetchar = 0;
		for( int x = x0; x < x0 + vonalhossz; x++ ) {
			addbytesor( &kovetchar, 100, x, x,
						Kovetoytomb[x-x0], fazis );
		}*/

		kikovetokepek( pkov, Kovetoytomb, vonalhossz, x0, fazis );
	}
}

void ecset::addobjektumok( void ) {
	if( !view )
		return;

	for( int i = 0; i < MAXKEREK; i++ ) {
		kerek* pk = Ptop->kerektomb[i];
		if( !pk )
			return;

		if( pk->tipus == T_KAJA || pk->tipus == T_KEZDO ||
				pk->tipus == T_CEL )
			continue;

		// Egy kep berakasa:
		if( view ) {
			unsigned char* mut = Plgr->viewexit;
			int ujtavolsag = 498;
			if( pk->tipus == T_HALALOS ) {
				mut = Plgr->viewkiller;
				ujtavolsag = 499;
			}
			int x1 = (pk->r.x-origo.x)*(Arany/Viewzoom);
			int y1 = (-pk->r.y-origo.y)*(Arany/Viewzoom);
			// Kirakja kis 3x3-as gyurut:
			addbytesor( mut, ujtavolsag, x1-1, x1+1, y1-1, FAZIS_NEMFOLDEG );
			addbytesor( mut, ujtavolsag, x1-1, x1-1, y1,   FAZIS_NEMFOLDEG );
			addbytesor( mut, ujtavolsag, x1+1, x1+1, y1,   FAZIS_NEMFOLDEG );
			addbytesor( mut, ujtavolsag, x1-1, x1+1, y1+1, FAZIS_NEMFOLDEG );

			continue;
		}


		/* Ez a resz csak nagy objektumokhoz kellene, de most csak view-val
		   foglalkozunk:
		int x1 = (pk->r.x-origo.x)*Arany;
		int y1 = (-pk->r.y-origo.y)*Arany;
		x1 -= OBJEKTUMSUGAR_I;
		y1 += OBJEKTUMSUGAR_I;

		pic8* ppic = Plgr->pexit;
		int ujtavolsag = 498;
		if( pk->tipus == T_HALALOS ) {
			ppic = Plgr->pkiller;
			ujtavolsag = 499;
		}

		int xsize = ppic->getxsize();
		int ysize = ppic->getysize();

		if( x1 < 120 || x1+xsize >= maxx-50 )
			return;

		// Van atlatszosag:
		unsigned char atlatszo = ppic->gpixel( 0, 0 );
		for( int y = 0; y < ysize; y++ ) {
			// Kep egy soranak elintezese:
			unsigned char* sor = ppic->getptr( ysize - 1 - y );
			if( ((int)sor & PIXELMASZK) == 0 )
				uzenet( "Memory address is less than 2048 (654)!" );

			// Atugorja elso ures pixeleket:
			int x = uresszam( 0, xsize, sor, atlatszo );
			while( x <= xsize-1 ) {
				// Keresi kis szakaszokat:
				int size = teliszam( x, xsize, sor, atlatszo );
				if( size <= 0 )
					hiba( "ecset::addspriteok teliszam utan size <= 0!" );

				// Negativ iranyban haladunk y-on (y1-y):
				if( y1-y >= 0 && y1-y < sorszam )
					addbytesor( &sor[x], ujtavolsag, x1+x, x1+x+size-1,
													y1-y, FAZIS_NEMFOLDEG );
				x += size;

				// Atugorja ures pixeleket:
				x += uresszam( x, xsize, sor, atlatszo );
			}
		}*/
	}
}

void ecset::levon10000t( void ) {
	for( int i = 0; i < sorszam; i++ ) {
		mdarab* pmd = msorok[i];
		while( pmd ) {
			if( pmd->tavolsag >= 10000 )
				pmd->tavolsag -= 10000;

			pmd = pmd->pkov;
		}
	}
};

// Ptop es Pszak alapjan:
ecset::ecset( int view_p ) {
	view = view_p;

	for( int i = 0; i < MAXECSETSOR; i++ ) {
		msorok[i] = NULL;
		sorok[i] = NULL;
		kurxposok_A[i] = 0;
		kurxposok_B[i] = 0;
		curdarabok_A[i] = NULL;
		curdarabok_B[i] = NULL;
	}

	// Inicializaljuk elso node tombot:
	elsotomb = kurtomb = NULL;
	tombbenkov = 0;

	nagydarabtomb = NULL;

	xsizedb = ysizedb = 0.0;
	maxx = sorszam = 0;

	if( !Ptop || !Pszak )
		hiba( "ecset::ecset !Ptop || !Pszak!" );

	elsotomb = new mdarabtomb;
	if( !elsotomb )
		hiba( "Nincs eleg memoria ecset::ecset!" );
	elsotomb->kovtomb = NULL;
	kurtomb = elsotomb;

	getorigoandsize();

	if( view ) {
		// VIEW VIEW VIEW:
		maxx = xsizedb*(Arany/Viewzoom);
		sorszam = ysizedb*(Arany/Viewzoom);
	}
	else {
		maxx = xsizedb*Arany;
		sorszam = ysizedb*Arany;
	}

	if( sorszam-10 > MAXECSETSOR )
		hiba( "ecset::ecset sorszam-10 > MAXECSETSOR!" );

	// Inicializaljuk minden sor elejet foldre:
	for( int i = 0; i < sorszam; i++ ) {
		msorok[i] = newmdarab();
		msorok[i]->pkov = NULL;
		msorok[i]->xsize = 1;
		msorok[i]->tavolsag = 0;
		msorok[i]->pixelek = (unsigned char*)PX_FOLD;
	}

	// Vegigmegyunk osszes szakaszon:
	Pszak->felsorolasresetszak();
	vonal* psz = Pszak->getnextszak();
	if( !psz )
		hiba( "Nincs egyetlen szakasz sem definialva!" );
	while( psz ) {
		addszakasz( psz );
		psz = Pszak->getnextszak();
	}

	// Sorrendbe rendezzuk egy sor elemeit:
	for( int i = 0; i < sorszam; i++ ) {
		rendez( msorok[i] );
		kurxposok_A[i] = kurxposok_B[i] = sizeraallit( msorok[i] );
	}

	// Spriteok beadasa:
	if( !view && State->highquality ) {
		addspriteok( FAZIS_FOLD );
		addkovetok( FAZIS_FOLD );
		addspriteok( FAZIS_EG );
		addkovetok( FAZIS_EG );
		levon10000t();
		addspriteok( FAZIS_NEMFOLDEG );
		addkovetok( FAZIS_NEMFOLDEG );
	}

	// Itt adtuk be objektumokat:
	addobjektumok(); // Ez most nem view eseten visszater

	// Ezeket vegen kivulrol hivja loadecsetek:

	// Rovid folddarabokat atalakitjuk konkret mutatova:
	//foldmutatocsere();

	//mutatotlanit();
	//deletemdarabok();
}

// URES sorokat tesz bele:
// Ptop es Pszak alapjan:
ecset::ecset( ecset* pold ) {
	for( int i = 0; i < MAXECSETSOR; i++ ) {
		msorok[i] = NULL;
		sorok[i] = NULL;
		kurxposok_A[i] = 0;
		kurxposok_B[i] = 0;
		curdarabok_A[i] = NULL;
		curdarabok_B[i] = NULL;
	}

	// Inicializaljuk elso node tombot:
	elsotomb = kurtomb = NULL;
	tombbenkov = 0;

	nagydarabtomb = NULL;

	view = 0;
	xsizedb = ysizedb = 0.0;
	maxx = sorszam = 0;

	if( !Ptop || !Pszak )
		hiba( "ecset::ecset !Ptop || !Pszak!" );

	elsotomb = new mdarabtomb;
	if( !elsotomb )
		hiba( "Nincs eleg memoria ecset::ecset!" );
	elsotomb->kovtomb = NULL;
	kurtomb = elsotomb;

	//getorigoandsize(); ehelyett:
	origo = pold->origo;
	xsizedb = pold->xsizedb;
	ysizedb = pold->ysizedb;

	maxx = pold->maxx;
	sorszam = pold->sorszam;

	// Inicializaljuk minden sor elejet uresre:
	for( int i = 0; i < sorszam; i++ ) {
		kurxposok_A[i] = 1;
		kurxposok_B[i] = 1;

		msorok[i] = newmdarab();
		msorok[i]->pkov = NULL;
		msorok[i]->xsize = 1000000;
		msorok[i]->tavolsag = 0;
		msorok[i]->pixelek = (unsigned char*)PX_URES;
	}
}

ecset::~ecset( void ) {
	if( elsotomb )
		hiba( "ecset::~ecset elsotomb" );
	if( !nagydarabtomb )
		hiba( "ecset::~ecset !nagydarabtomb!" );
	delete nagydarabtomb;
	nagydarabtomb = NULL;
}

void ecset::deletemdarabok( void ) {
	if( !elsotomb )
		hiba( "ecset::deletemdarabok !elsotomb!" );
	mdarabtomb* cur = elsotomb;
	while( cur ) {
		mdarabtomb* kov = cur->kovtomb;
		delete cur;
		cur = kov;
	}
	elsotomb = NULL;
	for( int i = 0; i < MAXECSETSOR; i++ )
		msorok[i] = NULL;
}

static int Xe1, Xe2, Xe2plussz1;

unsigned char *Foldsor = NULL, *Egsor = NULL;

void ecset::kiegysor_A( unsigned char* pc, int ye ) {
	// Visszalepunk amig kell:
	darab* pda = curdarabok_A[ye];
	int xpos = kurxposok_A[ye];
	while( xpos > Xe1 ) {
		pda--;
		xpos -= pda->xsize;
	}
	// Elorelepunk amig kell:
	while( xpos + pda->xsize <= Xe1 ) {
		xpos += pda->xsize;
		pda++;

	}
	// Elmentjuk kurrens darabot:
	curdarabok_A[ye] = pda;
	kurxposok_A[ye] = xpos;


	// ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
	// ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
	// ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
	// ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
	int ures = Xe1-xpos;
	if( xpos + pda->xsize > Xe2 ) {
		// ATLOG VEGIG:
		int ujpdaxsize = Xe2-xpos+1;
		// Ha eppen vegeig log, akkor is ide jon
		if( (pixelek_t)pda->pixelek & PIXELMASZK ) {
			// Normal kep copi:
			memcpy( pc, &pda->pixelek[Xe1-xpos], ujpdaxsize-ures );
		}
		else {
			// Eg, Fold, vagy ures:
			if( !pda->pixelek ) { // Eg
				memcpy( pc, Egsor, ujpdaxsize-ures );
				return;
			}
			if( (pixelek_t)pda->pixelek == PX_FOLD )
				memcpy( pc, Foldsor, ujpdaxsize-ures );
			else {
				#ifdef TEST
					if( (pixelek_t)pda->pixelek != PX_URES ) hiba( "(pixelek_t)pda->pixelek != PX_URES iueghyreg!" );
				#endif
			}
		}
		return;
	}

	// Elso szakasz nem log at vegig:
	int size = pda->xsize-ures;
	if( (pixelek_t)pda->pixelek & PIXELMASZK ) {
		// Normal kep copi:
		memcpy( pc, &pda->pixelek[Xe1-xpos], size );
	}
	else {
		// Eg, Fold, vagy ures:
		if( !pda->pixelek )  // Eg
			memcpy( pc, Egsor, size );
		else {
			if( (pixelek_t)pda->pixelek == PX_FOLD )
				memcpy( pc, Foldsor, size );
			else {
				#ifdef TEST
					if( (pixelek_t)pda->pixelek != PX_URES )
						hiba( "(pixelek_t)pda->pixelek != PX_URES iygvfrgiyvr!" );
				#endif
			}
		}
	}
	// Atugrunk kovetkezore:
	xpos += pda->xsize;
	pda++;
	pc += size;

	// BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
	// BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
	// BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
	// BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
	while( xpos <= Xe2 ) {
		#ifdef KIIRDARABOT
			Kisordarabszam++;
		#endif
		if( ye == 1071 && xpos == 12020 /*12055*/ ) {
			semmi();
		}

		if( xpos + pda->xsize > Xe2 ) {
			// Ez az utolso darab, ki is loghat:
			if( (pixelek_t)pda->pixelek & PIXELMASZK ) {
				// Normal kep copi:
				memcpy( pc, pda->pixelek, Xe2-xpos+1 );
				return;
			}
			if( !pda->pixelek ) {
				memcpy( pc, Egsor + (xpos-Xe1), Xe2-xpos+1 );
				return;
			}
			if( (pixelek_t)pda->pixelek == PX_FOLD ) {
				memcpy( pc, Foldsor + (xpos-Xe1), Xe2-xpos+1 );
				return;
			}
			#ifdef TEST
				if( (pixelek_t)pda->pixelek != PX_URES )
					hiba( "(pixelek_t)pda->pixelek != PX_URES 653wetgki!" );
			#endif
		}

		// Ez a darab nem log ki:
		if( (pixelek_t)pda->pixelek & PIXELMASZK ) {
			// Normal kep copi:
			memcpy( pc, pda->pixelek, pda->xsize );
		}
		else {
			if( !pda->pixelek ) {
				memcpy( pc, Egsor + (xpos-Xe1), pda->xsize );
			}
			else {
				if( (pixelek_t)pda->pixelek == PX_FOLD ) {
					memcpy( pc, Foldsor + (xpos-Xe1), pda->xsize );
				}
				else {
					#ifdef TEST
						if( (pixelek_t)pda->pixelek != PX_URES )
							hiba( "(pixelek_t)pda->pixelek != PX_URES 653wetgki!" );
					#endif
				}
			}
		}

		xpos += pda->xsize;
		pc += pda->xsize;
		pda++;
	}
}

void ecset::kiegysor_B( unsigned char* pc, int ye ) {
	// Visszalepunk amig kell:
	darab* pda = curdarabok_B[ye];
	int xpos = kurxposok_B[ye];
	while( xpos > Xe1 ) {
		pda--;
		xpos -= pda->xsize;
	}
	// Elorelepunk amig kell:
	while( xpos + pda->xsize <= Xe1 ) {
		xpos += pda->xsize;
		pda++;

	}
	// Elmentjuk kurrens darabot:
	curdarabok_B[ye] = pda;
	kurxposok_B[ye] = xpos;


	// ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
	// ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
	// ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
	// ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
	int ures = Xe1-xpos;
	if( xpos + pda->xsize > Xe2 ) {
		// ATLOG VEGIG:
		int ujpdaxsize = Xe2-xpos+1;
		// Ha eppen vegeig log, akkor is ide jon
		if( (pixelek_t)pda->pixelek & PIXELMASZK ) {
			// Normal kep copi:
			memcpy( pc, &pda->pixelek[Xe1-xpos], ujpdaxsize-ures );
		}
		else {
			// Eg, Fold, vagy ures:
			if( !pda->pixelek ) { // Eg
				memcpy( pc, Egsor, ujpdaxsize-ures );
				return;
			}
			if( (pixelek_t)pda->pixelek == PX_FOLD )
				memcpy( pc, Foldsor, ujpdaxsize-ures );
			else {
				#ifdef TEST
					if( (pixelek_t)pda->pixelek != PX_URES ) hiba( "(pixelek_t)pda->pixelek != PX_URES iueghyreg!" );
				#endif
			}
		}
		return;
	}

	// Elso szakasz nem log at vegig:
	int size = pda->xsize-ures;
	if( (pixelek_t)pda->pixelek & PIXELMASZK ) {
		// Normal kep copi:
		memcpy( pc, &pda->pixelek[Xe1-xpos], size );
	}
	else {
		// Eg, Fold, vagy ures:
		if( !pda->pixelek )  // Eg
			memcpy( pc, Egsor, size );
		else {
			if( (pixelek_t)pda->pixelek == PX_FOLD )
				memcpy( pc, Foldsor, size );
			else {
				#ifdef TEST
					if( (pixelek_t)pda->pixelek != PX_URES )
						hiba( "(pixelek_t)pda->pixelek != PX_URES iygvfrgiyvr!" );
				#endif
			}
		}
	}
	// Atugrunk kovetkezore:
	xpos += pda->xsize;
	pda++;
	pc += size;

	// BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
	// BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
	// BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
	// BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
	while( xpos <= Xe2 ) {
		#ifdef KIIRDARABOT
			Kisordarabszam++;
		#endif

		if( xpos + pda->xsize > Xe2 ) {
			// Ez az utolso darab, ki is loghat:
			if( (pixelek_t)pda->pixelek & PIXELMASZK ) {
				// Normal kep copi:
				memcpy( pc, pda->pixelek, Xe2-xpos+1 );
				return;
			}
			if( !pda->pixelek ) {
				memcpy( pc, Egsor + (xpos-Xe1), Xe2-xpos+1 );
				return;
			}
			if( (pixelek_t)pda->pixelek == PX_FOLD ) {
				memcpy( pc, Foldsor + (xpos-Xe1), Xe2-xpos+1 );
				return;
			}
			#ifdef TEST
				if( (pixelek_t)pda->pixelek != PX_URES )
					hiba( "(pixelek_t)pda->pixelek != PX_URES 653wetgki!" );
			#endif
		}

		// Ez a darab nem log ki:
		if( (pixelek_t)pda->pixelek & PIXELMASZK ) {
			// Normal kep copi:
			memcpy( pc, pda->pixelek, pda->xsize );
		}
		else {
			if( !pda->pixelek ) {
				memcpy( pc, Egsor + (xpos-Xe1), pda->xsize );
			}
			else {
				if( (pixelek_t)pda->pixelek == PX_FOLD ) {
					memcpy( pc, Foldsor + (xpos-Xe1), pda->xsize );
				}
				else {
					#ifdef TEST
						if( (pixelek_t)pda->pixelek != PX_URES )
							hiba( "(pixelek_t)pda->pixelek != PX_URES 653wetgki!" );
					#endif
				}
			}
		}

		xpos += pda->xsize;
		pc += pda->xsize;
		pda++;
	}
}

#ifdef FFECSET

static unsigned char Feketesor[650], Fehersor[650];
static unsigned char* FFsor = Feketesor;
static int FFallapot = 0, FFinited = 0;
static void ffmutvalt( void ) {
	FFallapot = !FFallapot;
	if( FFallapot )
		FFsor = Feketesor;
	else
		FFsor = Fehersor;
}

// Fekete-feher-ben rak ki (csak tesztre):
void ecset::kiegysor_FF( unsigned char* pc, int ye ) {
	if( !FFinited ) {
		FFinited = 1;
		for( int i = 0; i < 640; i++) {
			Feketesor[i] = 10;
			Fehersor[i] = 120;
		}
	}

	FFsor = Feketesor;
	FFallapot = 0;
	ffmutvalt();

	// Visszalepunk amig kell:
	darab* pda = curdarabok_A[ye];
	int xpos = kurxposok_A[ye];
	while( xpos > Xe1 ) {
		pda--;
		xpos -= pda->xsize;
	}
	// Elorelepunk amig kell:
	while( xpos + pda->xsize <= Xe1 ) {
		xpos += pda->xsize;
		pda++;

	}
	// Elmentjuk kurrens darabot:
	curdarabok_A[ye] = pda;
	kurxposok_A[ye] = xpos;


	// ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
	// ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
	// ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
	// ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
	int ures = Xe1-xpos;
	if( xpos + pda->xsize > Xe2 ) {
		// ATLOG VEGIG:
		int ujpdaxsize = Xe2-xpos+1;
		// Ha eppen vegeig log, akkor is ide jon
		if( (pixelek_t)pda->pixelek & PIXELMASZK ) {
			// Normal kep copi:
			memcpy( pc, FFsor, ujpdaxsize-ures );
			ffmutvalt();
		}
		else {
			// Eg, Fold, vagy ures:
			if( !pda->pixelek ) { // Eg
				memcpy( pc, FFsor, ujpdaxsize-ures );
				ffmutvalt();
				return;
			}
			if( (pixelek_t)pda->pixelek == PX_FOLD ) {
				memcpy( pc, FFsor, ujpdaxsize-ures );
				ffmutvalt();
			}
			else {
				#ifdef TEST
					if( (pixelek_t)pda->pixelek != PX_URES ) hiba( "(pixelek_t)pda->pixelek != PX_URES iueghyreg!" );
				#endif
			}
		}
		return;
	}

	// Elso szakasz nem log at vegig:
	int size = pda->xsize-ures;
	if( (pixelek_t)pda->pixelek & PIXELMASZK ) {
		// Normal kep copi:
		memcpy( pc, FFsor, size );
		ffmutvalt();
	}
	else {
		// Eg, Fold, vagy ures:
		if( !pda->pixelek ) {
			// Eg:
			memcpy( pc, FFsor, size );
			ffmutvalt();
		}
		else {
			if( (pixelek_t)pda->pixelek == PX_FOLD ) {
				memcpy( pc, FFsor, size );
				ffmutvalt();
			}
			else {
				#ifdef TEST
					if( (pixelek_t)pda->pixelek != PX_URES )
						hiba( "(pixelek_t)pda->pixelek != PX_URES iygvfrgiyvr!" );
				#endif
			}
		}
	}
	// Atugrunk kovetkezore:
	xpos += pda->xsize;
	pda++;
	pc += size;

	// BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
	// BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
	// BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
	// BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
	while( xpos <= Xe2 ) {
		if( xpos + pda->xsize > Xe2 ) {
			// Ez az utolso darab, ki is loghat:
			if( (pixelek_t)pda->pixelek & PIXELMASZK ) {
				// Normal kep copi:
				memcpy( pc, FFsor, Xe2-xpos+1 );
				ffmutvalt();
				return;
			}
			if( !pda->pixelek ) {
				memcpy( pc, FFsor, Xe2-xpos+1 );
				ffmutvalt();
				return;
			}
			if( (pixelek_t)pda->pixelek == PX_FOLD ) {
				memcpy( pc, FFsor, Xe2-xpos+1 );
				ffmutvalt();
				return;
			}
			#ifdef TEST
				if( (pixelek_t)pda->pixelek != PX_URES )
					hiba( "(pixelek_t)pda->pixelek != PX_URES 653wetgki!" );
			#endif
		}

		// Ez a darab nem log ki:
		if( (pixelek_t)pda->pixelek & PIXELMASZK ) {
			// Normal kep copi:
			memcpy( pc, FFsor, pda->xsize );
			ffmutvalt();
		}
		else {
			if( !pda->pixelek ) {
				memcpy( pc, FFsor, pda->xsize );
				ffmutvalt();
			}
			else {
				if( (pixelek_t)pda->pixelek == PX_FOLD ) {
					memcpy( pc, FFsor, pda->xsize );
					ffmutvalt();
				}
				else {
					#ifdef TEST
						if( (pixelek_t)pda->pixelek != PX_URES )
							hiba( "(pixelek_t)pda->pixelek != PX_URES 653wetgki!" );
					#endif
				}
			}
		}

		xpos += pda->xsize;
		pc += pda->xsize;
		pda++;
	}
}

#endif

void ecset::getbalalso_int( vekt2 balalso, int* px, int* py ) {
	vekt2 relat = balalso-origo;
	if( view ) {
		*px = relat.x*(Arany/Viewzoom); // Ecset egesz koordinatak
		*py = relat.y*(Arany/Viewzoom); // Ecset egesz koordinatak
	}
	else {
		*px = relat.x*Arany; // Ecset egesz koordinatak
		*py = relat.y*Arany; // Ecset egesz koordinatak
	}
}

/*static int Elsoegkeszit = 1;
static pic8* Pegbuffer = NULL;
static pic8* Felho = NULL;
static pic8* Felho2 = NULL;

static void egetkeszit( pic8* peg, int xhely, int xkezd, int xmodulus ) {
	if( Elsoegkeszit ) {
		Elsoegkeszit = 0;
		Pegbuffer = new pic8( peg->getxsize(), peg->getysize() );
		blt8( Pegbuffer, peg );

		Felho = new pic8( "felho.pcx" );
		forditkepet( Felho );
		spriteosit( Felho );

		Felho2 = new pic8( "felho2.pcx" );
		forditkepet( Felho2 );
		spriteosit( Felho2 );
	}

	// Resetaljuk eget:
	blt8( peg, Pegbuffer );

	// Felho2:
	double lassitas = 2.5;
	int modulus = 1000;
	int dx = -xhely/lassitas;
	while( dx < 0 )
		dx += modulus;
	dx -= modulus;
	int x = xkezd+dx;
	blt8( peg, Felho2, x, -80 );
	x += modulus;
	blt8( peg, Felho2, x, -80 );

	// Felho1:
	lassitas = 1.5;
	modulus = 1000;
	dx = -xhely/lassitas;
	while( dx < 0 )
		dx += modulus;
	dx -= modulus;
	x = xkezd+dx;
	blt8( peg, Felho, x, 0 );
	x += modulus;
	blt8( peg, Felho, x, 0 );
} */

void ecset::kitesz( int ajatekos, pic8* ppic, vekt2 balalso, int x1, int y1, int x2, int y2 ) {
	if( x1 >= x2 || y1 >= y2 )
		hiba( "ecset::kitesz x1 >= x2 || y1 >= y2!" );

	Kisordarabszam = 0;

	int ye1 = 0;
	getbalalso_int( balalso, &Xe1, &ye1 );
	Xe2 = Xe1 + x2-x1;
	Xe2plussz1 = Xe2+1;
	int ye2 = ye1 + y2-y1;

	if( Xe1 < 20 || Xe2 > maxx-20 || ye1 < 20 || ye2 > sorszam-20 )
		hiba( "ecset::kitesz Xe1 < 20 || Xe2 > maxx-20 || ye1 < 20 || ye2 > sorszam-20!" );

	int foldymodulus = Plgr->pfold->getysize();
	int egymodulus = Plgr->peg->getysize();

	int folddx = Xe1 % Plgr->foldxmodulus;
	//int eglassitas = 100;
	int eglassitas = 2;
	int egdx = (Xe1/eglassitas) % Plgr->egxmodulus;
	int egdy = ye1/eglassitas;

	//egetkeszit( Plgr->peg, Xe1, egdx, Plgr->egxmodulus );

	int yadd = ye1-y1;
	if( ajatekos ) {
		for( int y = y1; y <= y2; y++ ) {
			int ye = y + yadd;
			Foldsor = Plgr->pfold->getptr( ye % foldymodulus ) +
							folddx;
			// Eg fol-le is scrollozodik:
			//Egsor = Plgr->peg->getptr( (y-y1+egdy) % egymodulus ) + egdx;

			// Eg fol-le nem scrollozodik:
			Egsor = Plgr->peg->getptr( (y-y1) % egymodulus ) + egdx;

			#ifdef FFECSET
				kiegysor_FF( ppic->getptr(y)+x1, ye );
			#else
				kiegysor_A( ppic->getptr(y)+x1, ye );
			#endif
		}
	}
	else {
		for( int y = y1; y <= y2; y++ ) {
			int ye = y + yadd;
			Foldsor = Plgr->pfold->getptr( ye % foldymodulus ) +
							folddx;
			//Egsor = Plgr->peg->getptr( (ye+egdy) % egymodulus ) +
			//				egdx;
			Egsor = Plgr->peg->getptr( (y-y1) % egymodulus ) +
							egdx;

			#ifdef FFECSET
				kiegysor_FF( ppic->getptr(y)+x1, ye );
			#else
				kiegysor_B( ppic->getptr(y)+x1, ye );
			#endif
		}
	}
}

// Szinte ugyanaz, mint kiteszalso:
void ecset::kiteszview( int ajatekos, pic8* ppic, vekt2 balalso, int x1, int y1, int x2, int y2 ) {
	Foldsor = Plgr->viewfoldsor;
	Egsor = Plgr->viewegsor;

	if( x1 >= x2 || y1 >= y2 )
		hiba( "ecset::kitesz x1 >= x2 || y1 >= y2!" );

	int ye1 = 0;
	getbalalso_int( balalso, &Xe1, &ye1 );
	Xe2 = Xe1 + x2-x1+1;
	Xe2plussz1 = Xe2+1;
	int ye2 = ye1 + y2-y1+1;

	if( Xe1 < 20 || Xe2 > maxx-20 || ye1 < 20 || ye2 > sorszam-20 )
		hiba( "ecset::kitesz Xe1 < 20 || Xe2 > maxx-20 || ye1 < 20 || ye2 > sorszam-20!" );

	int yadd = ye1-y1;
	if( ajatekos ) {
		for( int y = y1; y <= y2; y++ ) {
			int ye = y + yadd;
			kiegysor_A( ppic->getptr(y)+x1, ye );
		}
	}
	else {
		for( int y = y1; y <= y2; y++ ) {
			int ye = y + yadd;
			kiegysor_B( ppic->getptr(y)+x1, ye );
		}
	}
}

static mdarab* Glb_pfoltdarab = NULL;

static int ezfolt( int x, int y, mdbiter* piter1 ) {
	if( !Glb_pfoltdarab )
		hiba( "65rw5r7twe" );

	// Felette levo sort vizsgaljuk:
	mdarab* pmd = piter1->getpmd( x, y+1 );

	int kepindex = (pixelek_t)pmd->pixelek;
	if( kepindex < 10 )
		// Ha fold, eg, vagy ures:
		return 0;

	if( pmd->pixelek == Glb_pfoltdarab->pixelek )
		// Ha meg ez is koveto textura:
		return 0;

	if( pmd->tavolsag > 500 )
		// Ha nem takarja el motorost:
		return 0;

	return 1;
}

static int getnemfoltszam( int x1, int xkov, int y, mdbiter* piter1 ) {
	int szam = 0;
	for( int x = x1; x < xkov; x++ ) {
		if( ezfolt( x, y, piter1 ) )
			return szam;
		szam++;
	}
	return szam;
}

static int getfoltszam( int x1, int xkov, int y, mdbiter* piter1 ) {
	int szam = 0;
	for( int x = x1; x < xkov; x++ ) {
		if( !ezfolt( x, y, piter1 ) )
			return szam;
		szam++;
	}
	return szam;
}

// Meg akkor kell hivni, amig nincsenek kettevalasztva also-ra es felsore es
// meg kis folddarabok sincsenek lecserelve:
void ecset::foltoz( void ) {
	mdbiter* piter1 = new mdbiter( this );
	// addbyte-ban tavolsagot valtakozva adjuk meg, nehogy osszeadja:
	int valtakozo = 0;

	// Vegigmegyunk osszes soron alulrol felfele:
	// Azert noveljuk megis y-t, mert fejjel lefele vannak y koordinatak:
	for( int y = 10; y < sorszam-10; y++ ) {
		// Vegigmegyunk osszes mdarabon:
		mdarab* pmd = msorok[y];
		int xpos = kurxposok_A[y];
		if( xpos > 10 )
			hiba( "uih67f" );

		while( pmd ) {
			int pmdorigxsize = pmd->xsize;
			mdarab* pmdorigkov = pmd->pkov;

			Glb_pfoltdarab = pmd;
			// Egy madarab-ot elintezunk:
			int kepindex = (pixelek_t)pmd->pixelek;
			if( ((kepindex & PIXELMASZK) == 0) && (kepindex >= 10) ) {
				kepindex -= 10;
				// Itt van maszk, le kell cserelni:
				#ifdef TEST
					if( kepindex >= Plgr->texturaszam )
						hiba( "egryiegr" );
				#endif
				textura* pt = &Plgr->texturak[kepindex];

				if( pt->foltos && pmd->tavolsag > 500 ) {
					// Ha egy foltos, 500-nal tavolabbi textura mdarabja:
					// Vegigmegyunk mdarab-on:
					int x = xpos;
					// Ez mar kov mdarab elso pixele:
					int xkov = xpos+pmd->xsize;
					while( x < xkov ) {
						int nfszam = getnemfoltszam( x, xkov, y, piter1 );
						x += nfszam;
						int foltszam = getfoltszam( x, xkov, y, piter1 );
						if( foltszam > 0 ) {
							valtakozo = !valtakozo;
							if( valtakozo )
								addbytesor( //(unsigned char*)10,
											pmd->pixelek,
											223,
											x, x+foltszam-1,
											y, FAZIS_NEMFOLDEG );
							else
								addbytesor( //(unsigned char*)10,
											pmd->pixelek,
											224,
											x, x+foltszam-1,
											y, FAZIS_NEMFOLDEG );
						}
						x += foltszam;
					}
				}
			}

			// Ugrunk kovetkezo mdarab-ra:
			xpos += pmdorigxsize;
			pmd = pmdorigkov;
		}
	}
	delete piter1; piter1 = NULL;
}


void betoltecseteket( void ) {
	Plgr->betolthattereket();

	Osszegszam = 0; // Ide osszegezzuk osszes lefoglalt szakaszt

	if( Pecsetalso )
		delete Pecsetalso;
	Pecsetalso = NULL;
	if( Pecsetfelso )
		delete Pecsetfelso;
	Pecsetfelso = NULL;
	if( Pecsetview )
		delete Pecsetview;
	Pecsetview = NULL;

	Pecsetalso = new ecset( 0 );
	if( !Pecsetalso )
		hiba( "Nincs eleg memoria Pecsetalso-nak!" );

	Pecsetalso->foltoz();

	Pecsetfelso = new ecset( Pecsetalso );
	if( !Pecsetfelso )
		hiba( "Nincs eleg memoria Pecsetfelso-nak!" );

	// Most betesszuk felsobe is 500-nal kozelebbi madarabokat:
	// Es egyben uresse tesszuk alsoban helyuket:
	for( int i = 0; i < Pecsetalso->sorszam; i++ ) {
		mdarab* pmd = Pecsetalso->msorok[i];
		int xpos = Pecsetalso->kurxposok_A[i];
		if( xpos > 100 )
			hiba( "betoltecseteket xpos > 100!" );
		while( pmd ) {
			if( pmd->tavolsag < 500 ) {
				if( (pixelek_t)pmd->pixelek >= 10 ) {
					Pecsetfelso->addbytesor( pmd->pixelek, pmd->tavolsag,
									xpos, xpos+pmd->xsize-1, i, FAZIS_URESRE );
					pmd->pixelek = (unsigned char*)PX_URES;
				}
			}
			xpos += pmd->xsize;
			pmd = pmd->pkov;
		}
	}

	// Rovid folddarabokat atalakitja konkret mutatova:
	Pecsetalso->foldmutatocsere();
	// Egymas utani ket azonos texturat
	// lecsereli egyre (mar beepiteskor is megtortenik nemileg):
	Pecsetalso->duplaeliminacio();
	Pecsetalso->textura2mutato(); // Lecsereli textura indexeket mutatokka
	Pecsetalso->mutatotlanit();   // mdarab lancolt listabol tombot csinal
	Pecsetalso->deletemdarabok(); // Torli lancolt listat:

	Pecsetfelso->duplaeliminacio();
	Pecsetfelso->textura2mutato();
	Pecsetfelso->mutatotlanit();
	Pecsetfelso->deletemdarabok();

	// View ecset:
	Pecsetview = new ecset( 1 );
	if( !Pecsetview )
		hiba( "Nincs eleg memoria Pecsetview-nak!" );
	Pecsetview->mutatotlanit();
	Pecsetview->deletemdarabok();

	// Kajak egesz koordjainak kitoltese:
	Pecsetalso->kitoltfoodkoordokat();
	Pecsetview->kitoltfoodkoordokat(); // View koordok
}


void segedfv( void ) {
	ecset* pe = NULL;
	pe->kiegysor_A( NULL, 0 );
}


// Egy iterator class (inkabb cache) ecsethez (foltoz hasznalja oket):
mdbiter::mdbiter( ecset* pecset_p ) {
	xposbent = -1;
	ybent = -1;
	pmd = NULL;

	pecset = pecset_p;
}

mdarab* mdbiter::getpmd( int x, int y ) {
	if( y == ybent && x >= xposbent ) {
		while( x >= xposbent + pmd->xsize ) {
			xposbent += pmd->xsize;
			pmd = pmd->pkov;
		}
	}
	else {
		// Ujra kell olvasni:
		if( y < 0 || y >= pecset->sorszam )
			hiba( "789eygrq" );
		pmd = pecset->msorok[y];
		xposbent = pecset->kurxposok_A[y];
		if( xposbent > 10 )
			hiba( "y7fjksd" );
		while( x >= xposbent + pmd->xsize ) {
			xposbent += pmd->xsize;
			pmd = pmd->pkov;
		}
	}
	return pmd;
}

