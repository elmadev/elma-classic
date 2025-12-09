#include    "all.h"

typedef unsigned char* puchar;

/*class pic8 {
    friend blt8( pic8* pd, pic8* ps, int x = 0, int y = 0,
        int x1 = -10000, int y1 = -10000, int x2 = -1000, int y2 = -10000 );
    int fizkep;
    int lefoglal( xsize, ysize );
    int xsize, ysize, success;
    puchar* sormuttomb;
    puchar* szegmuttomb;
    int     szegnum;
public:
*/

// Ha igaz, akkor lefoglalt kepek minden masodik sora paratlan cimen
// kezdodik. LGR file beolvasas es betolthattereket alatt aktiv:
int Paratlankepsorok = 0;

int pic8::lefoglal( long xsizep, long ysizep ) {
    // nem szabad hasznalni size-eket, hanem sizep-eket!
    if( sormuttomb || szegmuttomb ) {
        hiba( "Bhiba pic8::lefoglalban!" );
        return 0;
    }
    xsize = short(xsizep);
    ysize = short(ysizep);
	// xsizep-ot 4-el oszthatova valtoztatjuk:
	if( Paratlankepsorok )
		xsizep++;
	if( xsizep % 4 ) {
        xsizep += 4-xsizep%4;
    }
    if( xsizep <= 0 || ysizep <= 0 )  {
        hiba( "sizep tul pici pic8::lefoglalban!" );
        return 0;
    }
    long egyszegben = 0x0ff00l/long(xsizep); // Nehogy szegmens merete
                // megkozelitse 65536-ot
    // szegnum meg inkrementalva lesz a vegen, ha plussz is van!:
    szegnum = unsigned(ysizep)/egyszegben;
    long pluszszegbensorok = ysizep - szegnum*egyszegben;
    int vanplussz = pluszszegbensorok != 0;
    { sormuttomb = new puchar[unsigned(ysizep)]; }
    if( vanplussz ) {
        { szegmuttomb = new puchar[unsigned(szegnum+1)]; }
    }
    else {
        { szegmuttomb = new puchar[unsigned(szegnum)]; }
    }
	if( !sormuttomb || !szegmuttomb ) {
        uzenet( "pic8::alloc memory!" );
        return 0;
    }
    for( int i = 0; i < ysizep; i++ ) {
        sormuttomb[i] = NULL;
    }
    for( int i = 0; i < szegnum; i++ ) {
        szegmuttomb[i] = NULL;
    }
    if( vanplussz )
        szegmuttomb[unsigned(szegnum)] = NULL;
    for( int i = 0; i < szegnum; i++ ) {
        { szegmuttomb[i] = new unsigned char[unsigned(egyszegben*xsizep)]; }
        if( !szegmuttomb[i] ) {
            uzenet( "pic8::alloc memory!" );
            return 0;
        }
        for( int j = 0; j < egyszegben; j++ )
            sormuttomb[unsigned(i*egyszegben+j)] =
							&szegmuttomb[i][unsigned(j*xsizep)];
    }
    if( vanplussz ) {
        { szegmuttomb[unsigned(szegnum)] =
    new unsigned char[unsigned(pluszszegbensorok*xsizep)]; }
        if( !szegmuttomb[unsigned(szegnum)] ) {
            uzenet( "pic8::alloc memory!" );
            return 0;
        }
        for( int j = 0; j < pluszszegbensorok; j++ )
            sormuttomb[unsigned(szegnum*egyszegben+j)] =
                    &szegmuttomb[unsigned(szegnum)][unsigned(j*xsizep)];
    }

    if( vanplussz )
        szegnum++;

	for( int y = 0; y < ysizep; y++ ) {
		memset( sormuttomb[y], 0, unsigned(xsizep) ); // nullazunk
		if( Paratlankepsorok && (y % 2) )
			sormuttomb[y]++;
	}

	return 1;
}

pic8::~pic8( void ) {
    if( nemdestrukt )
        return;
    if( sormuttomb )
        delete sormuttomb;
    if( szegmuttomb )
        for( int i = 0; i < szegnum; i++ )
            delete szegmuttomb[i];
    delete szegmuttomb;
    if( sprite )
        delete sprite;
}

pic8::pic8( int xsizep, int ysizep ) {
    success = 0;
    nemdestrukt = 0;
    sormuttomb = NULL;
    szegmuttomb = NULL;
    fizkep = 0;
    sprite = NULL;
    spritehossz = NULL;
    success = lefoglal( xsizep, ysizep );
}

// Mar meglevo tombre epiti ra pic8-at, destruktor nem torli:
pic8::pic8( int xsizep, int ysizep, unsigned char** ppc ) {
    success = 0;
    nemdestrukt = 1;
    sormuttomb = NULL;
    szegmuttomb = NULL;
    fizkep = 0;
    sprite = NULL;
    spritehossz = NULL;
    if( xsizep == -12333 && ysizep == -12334 ) {
        hiba( "fizkep nemdestruktos pic8 konstruktorban!" );
        return;
    }
    xsize = short(xsizep);
    ysize = short(ysizep);
    if( !ppc )
        hiba( "!ppc nemdestruktos pic8 konstruktorban!" );
    sormuttomb = ppc;
    success = 1;
}

pic8::pic8( char* nev, FILE* h ) {
    nemdestrukt = 0;
    success = 0;
    sormuttomb = NULL;
    szegmuttomb = NULL;
    fizkep = 0;
    sprite = NULL;
    spritehossz = NULL;
	// Kiterjesztes megkeresese:
    int i = strlen( nev ) - 1;
    while( i >= 0 ) {
        if( nev[i] == '.' ) {
			if( strcmpi( nev+i, ".spr" ) == 0 ) {
                spritebeolvas( nev, h );
                return;
            }
            if( strcmpi( nev+i, ".pcx" ) == 0 ) {
                pcxbeolvas( nev, h );
                return;
            }
            hiba( "pic8 konstruktor nem ismeri kiterjesztest nevben!: ", nev );
            return;
        }
        i--;
    }
    hiba( "pic8 konstruktor nem talalt pontot nevben!: ", nev );
}



int pic8::save( char* nev, unsigned char* pal, FILE* h ) {
    if( !success ) {
        hiba( "pic8::save-ben nem success!" );
        return 0;
    }
    if( fizkep ) {
        hiba( "Fizikai kepernyore save hivasa!" );
        return 0;
    }
    // Kiterjesztes megkeresese:
    int i = 0;
    while( nev[i] ) {
        if( nev[i] == '.' ) {
			if( strcmpi( nev+i, ".spr" ) == 0 ) {
                return spritesave( nev, h );
            }
            if( strcmpi( nev+i, ".pcx" ) == 0 ) {
                return pcxsave( nev, pal );
            }
            //if( strcmpi( nev+i, ".mix" ) == 0 ) {
            //  return mixsave( nev );
            //}
            hiba( "pic8::save-ben nem ismeri kiterjesztest nevben!: ", nev );
            return 0;
        }
        i++;
    }
    hiba( "pic8::save-ben nem talalt pontot nevben!: ", nev );
    return 0;
}

int pic8::getxsize( void ) {
    return xsize;
}

int pic8::getysize( void ) {
    return ysize;
}

void pic8::ppixel( int x, int y, unsigned char szin ) {
    #ifdef PIC8TEST
        if( !success ) {
            hiba( "pic8::ppixelben success = 0!" );
            return;
        }
    #endif
    if( x < 0 || x >= xsize || y < 0 || y >= ysize )
        return;

    if( !fizkep ) {
        sormuttomb[y][x] = szin;
        return;
    }
	// Fizikai:
    hiba( "ppixel fizikaira!" );
}

unsigned char pic8::gpixel( int x, int y ) {
    #ifdef PIC8TEST
        if( !success ) {
            hiba( "pic8::ppixelben success = 0!" );
            return 0;
        }
    #endif
    if( x < 0 || x >= xsize || y < 0 || y >= ysize )
        return 0;

    if( !fizkep )
        return sormuttomb[y][x];
    else {
        // Fizikai:
        hiba( "gpixel fizikaira!" );
        //return (unsigned char)gpixel_l( x, y );
		return 0;
	}
}

#ifdef TEST

// Egyebkent inline:

unsigned char* pic8::getptr( int y ) {
	#ifdef PIC8TEST
		if( !success ) {
			hiba( "pic8::getptr success = 0!" );
			return 0;
		}
		if( y < 0 || y >= ysize ) {
			hiba( "pic8::getptr y kint van!" );
			return 0;
		}
	#endif
	if( !fizkep ) {
		return sormuttomb[y];
	}
	else {
		// Fizikai:
		hiba( "pic8::getptr meghivasa fizikai kepernyore!" );
		return NULL;
	}
}

#endif

void pic8::fillbox( unsigned char szin ) {
	fillbox( 0, 0, getxsize()-1, getysize()-1, szin );
}

void pic8::fillbox( int x1, int y1, int x2, int y2, unsigned char szin ) {
    if( fizkep )
        hiba( "pic8::fillbox meghivasa fizikai kepernyore!" );
    if( x1 > x2 ) {
        int tmp = x1; x1 = x2; x2 = tmp;
    }
    if( y1 > y2 ) {
        int tmp = y1; y1 = y2; y2 = tmp;
    }
    if( x1 < 0 )
        x1 = 0;
    if( y1 < 0 )
        y1 = 0;
    if( x2 >= xsize )
        x2 = xsize-1;
    if( y2 >= ysize )
        y2 = ysize-1;
    int xhossz = x2-x1+1;
    for( int y = y1; y <= y2; y++ ) {
		memset( sormuttomb[y]+x1, szin, xhossz );
	}
}


void pic8::vizszegmens_look( int x, int y, int size, unsigned char* lookup ) {
	#ifdef PIC8TEST
		if( x < 0 || y < 0 || x+size-1 >= xsize || y >= ysize )
			hiba( "pic8::vizszegmens-ben x, y kilog!" );
	#endif
	unsigned char* psor = getptr( y );
	for( int i = 0; i < size; i++ ) {
		psor[x+i] = lookup[psor[x+i]];
	}
	//memset( &psor[x], szin, size );
}

void pic8::fuggszegmens_look( int x, int y, int size, unsigned char* lookup ) {
	#ifdef PIC8TEST
		if( x < 0 || y < 0 || x >= xsize || y+size-1 >= ysize )
			hiba( "pic8::vizszegmens-ben x, y kilog!" );
	#endif
	for( int i = 0; i < size; i++ ) {
		unsigned char* psor = getptr( y+i );
		psor[x] = lookup[psor[x]];
	}

	//for( int i = 0; i < size; i++ )
	//	ppixel( x, y+i, szin );
}


// SPRITE:

/*static void dummy( long l ) {
    l++;
} */

void pic8::spritebeolvas( char* nev, FILE* h ) {
    int volth = 1;
    if( !h ) {
        volth = 0;
        h = qopen( nev, "rb" );
        if( !h ) {
            hiba( "pic8 spritebeolvaso nem tudta megnyitni file-t!: ", nev );
            return;
        }
    }
    unsigned char c = 0;
    if( fread( &c, 1, 1, h ) != 1 ) {
        hiba( "Nem tudott olvasni file-bol!: ", nev );
        if( !volth ) qclose( h );
        return;
    }
    if( c != 0x2d ) {
        hiba( "SPRITE file elso karaktere nem 0x2d!: ", nev );
        if( !volth ) qclose( h );
        return;
    }

	unsigned short xsize_s = -1, ysize_s = -1;
	if( fread( &xsize_s, 2, 1, h ) != 1 || fread( &ysize_s, 2, 1, h ) != 1 ) {
        hiba( "Nem tudott olvasni SPRITE file-bol!: ", nev );
        if( !volth ) qclose( h );
        return;
	}
	xsize = xsize_s;
	ysize = ysize_s;
    if( xsize < 1 || ysize < 1 ) {
        hiba( "Meretek kicsik SPRITE file-ban!: ", nev );
        if( !volth ) qclose( h );
        return;
    }
    if( !lefoglal( xsize, ysize ) )
        return;

    for( int y = 0; y < ysize; y++ ) {
        // Egy sor beolvasasa:
        if( fread( sormuttomb[y], xsize, 1, h ) != 1 ) {
            hiba( "Nem tudott olvasni SPRITE file-bol!: ", nev );
            if( !volth ) qclose( h );
            return;
        }
    }

    // SPRITE reszek beolvasasa:
    char tmp[10] = "";
    if( fread( tmp, 7, 1, h ) != 1 ) {
        hiba( "Nem tudott olvasni SPRITE file-bol!: ", nev );
        if( !volth ) qclose( h );
        return;
    }
    if( strcmp( tmp, "SPRITE" ) != 0 ) {
        hiba( "Sprite kulcsszo nem stimmel SPRITE file-ban!: ", nev );
        if( !volth ) qclose( h );
        return;
    }

    spritehossz = -1;
    if( fread( &spritehossz, 2, 1, h ) != 1 ) {
        hiba( "Nem tudott olvasni SPRITE file-bol!: ", nev );
        if( !volth ) qclose( h );
        return;
    }
    if( spritehossz < 1 ) {
        hiba( "Sprite hossza tul rovid SPRITE file-ban!: ", nev );
        if( !volth ) qclose( h );
        return;
    }

    //long l = farcoreleft();
    //dummy( l );
    { sprite = new unsigned char[spritehossz]; }
    if( !sprite ) {
        hiba( "Nincs eleg hely SPRITE beolvasoban!: ", nev );
        if( !volth ) qclose( h );
        return;
    }
    if( fread( sprite, spritehossz, 1, h ) != 1 ) {
        hiba( "Nem tudott olvasni SPRITE file-bol sprite-ot!: ", nev );
        if( !volth ) qclose( h );
        return;
    }
    if( !volth )
        qclose( h );
    success = 1;
}

int pic8::spritesave( char* nev, FILE* h ) {
    int volth = 1;
    if( !h ) {
        volth = 0;
        h = fopen( nev, "wb" );
        if( !h ) {
            hiba( "pic8::spritesave-ben nem tudta megnyitni file-t!: ", nev );
            return 0;
        }
    }
    unsigned char c = 0x2d;
    if( fwrite( &c, 1, 1, h ) != 1 ||
        fwrite( &xsize, 2, 1, h ) != 1 ||
        fwrite( &ysize, 2, 1, h ) != 1 ) {
        hiba( "pic8::spritesave-ben nem tud irni file-ba!: ", nev );
        if( !volth ) fclose( h );
        return 0;
    }

    for( int y = 0; y < ysize; y++ ) {
        // Egy sor kiirasa:
        if( fwrite( sormuttomb[y], xsize, 1, h ) != 1 ) {
            hiba( "Nem tudott irni SPRITE file-ba!: ", nev );
			if( !volth ) fclose( h );
			return 0;
		}
	}

	// Sprite resz kiirasa:
	if( fwrite( "SPRITE", 7, 1, h ) != 1 ||
		fwrite( &spritehossz, 2, 1, h ) != 1 ||
		fwrite( sprite, spritehossz, 1, h ) != 1 ) {
		hiba( "Nem tudott irni SPRITE file-ba!: ", nev );
		if( !volth ) fclose( h );
		return 0;
	}
	if( !volth )
		fclose( h );
	return 1;
}

// PCX:
struct  pcxdescriptor {
	unsigned char   ManufactId,
					VersionNum,
					EncodingTech,
					BitsPerPlane;
	short           Xmin, Ymin, Xmax, Ymax;
	short           HorRes, VertRes;
	unsigned char   ColorMap[48],
					Reserved,
					NumberOfBitPlanes;
	short           BytesPerScanLine,
					PaletteInf;
	unsigned char   Padding[127-70+1];
};

void pic8::pcxbeolvas( char* nev, FILE* h ) {
	// Gany:
	success = 1;

	int zarando = 0;
	if( !h ) {
		zarando = 1;
		h = qopen( nev, "rb" );
		if( !h )
			hiba( "Nem nyilik PCX file!: ", nev );
	}
	pcxdescriptor   desc;
	if( fread( &desc, sizeof( desc ), 1, h ) != 1 )
		hiba( "Nem olvas PCX file-bol!: ", nev );
	if( (desc.VersionNum != 5)    ||
		(desc.ManufactId != 10)   ||
		(desc.EncodingTech != 1)  ||
		(desc.BitsPerPlane != 8 ) ||
		(desc.NumberOfBitPlanes != 1 ) )
			hiba( "\nNem megfelelo PCX file header!", nev );
	//if( !lefoglal( desc.Xmax-desc.Xmin+1, desc.Ymax-desc.Ymin+1 ) ) {
	if( !lefoglal( desc.Xmax-desc.Xmin+1, desc.Ymax-desc.Ymin+1 ) )
		hiba( "pcx !lefoglal!" );
	for( int y = 0; y < ysize; y++ ) {
		// Egy sor beolvasasa:
		short nnn = 0, ccc, iii;

		do {
			unsigned char szin;
			long l = fread( &szin, 1, 1, h );
			ccc = szin;
			if( l != 1 )
				hiba( "\nNem tud olvasni PCX-ben", nev );

			if( (ccc & 0xc0) == 0xc0 ) {
				iii = ccc & (short)0x3f;
				l = fread( &szin, 1, 1, h );
				ccc = szin;
				if( l != 1 )
					hiba( "\nNem tud olvasni PCX-ben", nev );

				while( iii-- ) {
					if( nnn < xsize )
						ppixel( nnn, y, (unsigned char)ccc );
					nnn++;
				}
			}
			else {
				if( nnn < xsize )
					ppixel( nnn, y, (unsigned char)ccc );
				nnn++;
			}
		} while( nnn < desc.BytesPerScanLine );
	}

	// success az elejen mar 1-re van allitva!
	if( zarando )
		qclose( h );
}

static int numberofrepeats( pic8* ppic, int x, int y, int xsize ) {
	unsigned char szin = ppic->gpixel( x, y );
	x++;
	int db = 1;
    while( x < xsize && ppic->gpixel( x, y ) == szin ) {
        x++;
        db++;
    }
    return db;
}

int pic8::pcxsave( char *nev, unsigned char* pal ) {
    FILE* h = fopen( nev, "wb" );
    if( !h ) {
        hiba( "pcxsave-ben nem nyilik file!: ", nev );
        return 0;
    }
    pcxdescriptor   desc;
    desc.ManufactId         = 10;
    desc.VersionNum         = 5;
    desc.EncodingTech       = 1;
    desc.BitsPerPlane       = 8;
    desc.Xmin = desc.Ymin   = 0;
    desc.Xmax               = (short)(xsize-1);
    desc.Ymax               = (short)(ysize-1);
    desc.HorRes             = 10;
    desc.VertRes            = 10;
    desc.Reserved           = 0;
    desc.NumberOfBitPlanes  = 1;
	desc.BytesPerScanLine   = (unsigned short)xsize;
    desc.PaletteInf         = 1;
    if( fwrite( &desc, sizeof( desc ), 1, h ) != 1 ) {
        hiba( "pcxsave-ben sikertelen iras!: ", nev );
        fclose( h );
        return 0;
    }
    for( int y = 0; y < ysize; y++ ) {
        int x = 0;
        while( x < xsize ) {
            int i = numberofrepeats( this, x, y, xsize );
            if( i > 1 ) {
                if( i > 63 )
                    i = 63;
                unsigned char controll = (unsigned char)(i+192);
                if( fwrite( &controll, 1, 1, h ) != 1 ) {
                    hiba( "pcxsave-ben nem ir!: ", nev );
                    fclose( h );
                    return 0;
                }
                unsigned char szin = gpixel( x, y );
                if( fwrite( &szin, 1, 1, h ) != 1 ) {
                    hiba( "pcxsave-ben nem ir!: ", nev );
                    fclose( h );
                    return 0;
                }
                x += i;
            }
            else {
                unsigned char szin = gpixel( x, y );
                if( szin < 64 ) {
                    if( fwrite( &szin, 1, 1, h ) != 1 ) {
                        hiba( "pcxsave-ben nem ir!: ", nev );
                        fclose( h );
                        return 0;
                    }
                }
                else {
                    unsigned char controll = 193;
                    if( fwrite( &controll, 1, 1, h ) != 1 ) {
                        hiba( "pcxsave-ben nem ir!: ", nev );
                        fclose( h );
                        return 0;
                    }
                    szin = gpixel( x, y );
                    if( fwrite( &szin, 1, 1, h ) != 1 ) {
                        hiba( "pcxsave-ben nem ir!: ", nev );
                        fclose( h );
                        return 0;
                    }
                }
                x++;
            }
        }
    }
    unsigned char magikus = 0x0c;
    if( fwrite( &magikus, 1, 1, h ) != 1 ) {
        hiba( "pcxsave-ben nem ir!: ", nev );
        fclose( h );
        return 0;
    }
    if( pal ) {
        // Van paletta!:
        for( int i = 0; i < 768; i++ ) {
            unsigned char c = (unsigned char)(pal[i]<<2);
            if( fwrite( &c, 1, 1, h ) != 1 ) {
                hiba( "pcxsave-ben nem ir!: ", nev );
                fclose( h );
                return 0;
            }
        }
    }
    else {
        // Nincs paletta megadva!
        for( int i = 0; i < 256; i++ ) {
            unsigned char c = (unsigned char)i;
            for( int j = 0; j < 3; j++ ) {
                if( fwrite( &c, 1, 1, h ) != 1 ) {
                    hiba( "pcxsave-ben nem ir!: ", nev );
                    fclose( h );
                    return 0;
                }
            }
        }
    }

    fclose( h );
    return 1;
}

//int Debugblt = 0;
void blt8( pic8* pd, pic8* ps, int x, int y,
                int x1, int y1, int x2, int y2 ) {
    #ifdef PIC8TEST
        if( y2 == -10000 && (x1 != -10000 || y1 != -10000 || x2 != -10000 ) ) {
            hiba( "blt8 hivasa y2 == -10000 (default), de elotte nem mind!" );
            return;
        }
		if( !pd || !ps ) {
			hiba( "blt8 hivasa, dest vagy source NULL!" );
			return;
		}
	#endif

	//if( Debugblt ) idejott
	//	hiba( "Itt van 17!" );

	if( pd->sprite ) {
		hiba( "blt8 hivasa, de dest sprite!" );
		return;
	}
	if( x1 == -10000 ) {
		x1 = y1 = 0;
		x2 = ps->xsize-1;
		y2 = ps->ysize-1;
	}
	else {
		// Sorrend beallitasa:
		if( x2 < x1 ) {
			int tmp = x1;
			x1 = x2;
			x2 = tmp;
		}
		if( y2 < y1 ) {
			int tmp = y1;
			y1 = y2;
			y2 = tmp;
		}
	}
	// Levagdosasa reszeknek x iranyban:
	// Eloszor source kepnek megfeleloen:
	if( x1 < 0 ) {
		x += -x1;
		x1 = 0;
	}
	if( x2 >= ps->xsize ) {
		x -= x2-(ps->xsize-1);
		x2 = ps->xsize-1;
	}
	// Most destinacionak megfeleloen:
	if( x < 0 ) {
		x1 += -x;
		x = 0;
	}
	if( x+(x2-x1) >= pd->xsize ) {
		x2 -= x+(x2-x1)-(pd->xsize-1);
	}
	if( x1 > x2 )
		return;

	// Levagdosasa reszeknek y iranyban:
	// Eloszor source kepnek megfeleloen:
	if( y1 < 0 ) {
		y += -y1;
		y1 = 0;
	}
	if( y2 >= ps->ysize ) {
		y -= y2-(ps->ysize-1);
		y2 = ps->ysize-1;
	}
	// Most destinacionak megfeleloen:
	if( y < 0 ) {
		y1 += -y;
		y = 0;
	}
	if( y+(y2-y1) >= pd->ysize ) {
		y2 -= y+(y2-y1)-(pd->ysize-1);
	}
	if( y1 > y2 )
		return;

	if( ps->sprite ) {
		// sprite lejatszas:
		// Itt vegig kell futni teljes sprite kepet es kozben vagni!
		unsigned buf = 0;
		unsigned char* buffer = ps->sprite;
		int desty = y-y1;
		for( int sy = 0; sy < ps->ysize; sy++ ) {
			int ybentvan = 1;
			if( sy < y1 || sy > y2 )
				ybentvan = 0;
			int sx = 0;
			while( sx < ps->xsize ) {
				switch( buffer[buf++] ) {
					case 'K':
						if( ybentvan ) {
							if( sx < x1 || sx+buffer[buf]-1 > x2 ) {
								if( !(sx > x2 || sx+buffer[buf]-1 < x1) ) {
									// Kilog, de van bent resze:
									int xkezd = sx;
									int xveg = sx+buffer[buf]-1;
									if( xkezd < x1 )
										xkezd = x1;
									if( xveg > x2 )
										xveg = x2;
									if( pd->fizkep )
										hiba( "t4ruh5t" );
										//putline8_l( x+xkezd-x1, desty,
										//&ps->sormuttomb[sy][xkezd],
										//xveg-xkezd+1 );
									else
										memcpy( &pd->sormuttomb[desty][x+xkezd-x1],
										&ps->sormuttomb[sy][xkezd],
										xveg-xkezd+1 );
								}
							}
							else {
								// Nem log ki:
								if( pd->fizkep )
									hiba( "68464" );
                                    //putline8_l( x+sx-x1, desty,
                                    //&ps->sormuttomb[sy][sx],
                                    //buffer[buf] );
                                else
									memcpy( &pd->sormuttomb[desty][x+sx-x1],
                                    &ps->sormuttomb[sy][sx],
                                    buffer[buf] );
                            }
                        }
                        sx += buffer[buf++];
                        break;
                    case 'N':
                        sx += buffer[buf++];
                        break;
                    default:

                        hiba( "Sprite blt-ban default parancsertelmezoben!" );
                        return;
                }
			}
            desty++;
        }

        return;
	}

	// Most mar tuti jok koordok!
	if( pd->fizkep ) {
		// Fizikai kepernyore iras:
        hiba( "0895t" );
	}
	if( ps->fizkep ) {
		// Fizikai kepernyorol olvasas:
        hiba( "8795t" );
	}
	// Memoria-memoria atvitel!
	int xmeret = x2-x1+1;
	int dfy = y;

	// if( Debugblt )	idejott
	//	hiba( "Itt van 18!" );

    for( int fy = y1; fy <= y2; fy++ ) {
		memcpy( &pd->sormuttomb[dfy++][x], &ps->sormuttomb[fy][x1], xmeret );
    }
	//if( Debugblt )	
	//	hiba( "Itt van 19!" ); idejott
}

/*void blt8fizrefejjelle( pic8* ps ) {
    int fizxsize = Pscr8->getxsize();
    int fizysize = Pscr8->getysize();
    #ifdef PIC8TEST
        if( ps == Pscr8 ) {
            hiba( "blt8fizrefejjelle hivasa, fizikai kepernyo parral!" );
            return;
        }
        if( ps->sprite ) {
            hiba( "blt8fizrefejjelle hivasa, sprite parral!" );
            return;
        }
        if( fizxsize != ps->getxsize() || fizysize != ps->getysize() ) {
            hiba( "blt8fizrefejjelle: par size != fizikai size!" );
            return;
        }
    #endif
    for( int y = 0; y < fizysize; y++ ) {
        putbytes_l( 0, y, ps->sormuttomb[fizysize-y-1], fizxsize );
    }
} */

int pcxtopal( char* nev, unsigned char* pal ) {
    // Paletta beolvasasa:
    FILE* h = qopen( nev, "rb" );
    if( !h ) {
        hiba( "pcxtopal-ban nem tudta megnyitni file-t!: ", nev );
        return 0;
    }
    long l = -769;
    if( qseek( h, l, SEEK_END ) != 0 ) {
        hiba( "Nem tud visszalepni palettahoz 768-at PCX-ben!: ", nev );
        qclose( h );
        return 0;
    }
    char c;
    l = fread( &c, 1, 1, h );
    if( l != 1 ) {
        hiba( "Nem tud olvasni PCX-ben!:", nev );
        qclose( h );
        return 0;
    }
    if( c != 0x0c ) {
        hiba( "Nem 0x0C a paletta elotti byte PCX-ben!: ", nev );
        qclose( h );
        return 0;
    }
    if( fread( pal, 768, 1, h ) != 1 ) {
        hiba( "Nem tud olvasni PCX-ben!: ", nev );
        qclose( h );
        return 0;
    }
    for( int i = 0; i < 768; i++ )
        pal[i] = (unsigned char)(pal[i] >> 2);
    qclose( h );
    return 1;
}

int pcxtopal( char* nev, ddpal** ppddpal ) {
    unsigned char pal[768];
	pcxtopal( nev, pal );
	if( strcmp( nev, "intro.pcx" ) == 0 ) {
		// Egy kicsit gany, teljes.cpp-bol hivodik ilyen parameterrel.
		pal[0] = pal[1] = pal[2] = 0;
	}
    { *ppddpal = new ddpal( pal ); }
    return 1;
}
void mintavetel8( pic8* pdest, pic8* psour, int x1, int y1, int x2, int y2 ) {
    // Ellenorzesek:
    #ifdef PIC8TEST
        if( !pdest || !psour )
        hiba( "mintavetel8-ban !pdest || !psour!" );
    #endif

    if( x1 > x2 ) {
        int tmp = x1; x1 = x2; x2 = tmp;
    }
    if( y1 > y2 ) {
        int tmp = y1; y1 = y2; y2 = tmp;
    }

    #ifdef PIC8TEST
        if( x1 < 0 || y1 < 0 || x2 >= pdest->getxsize() ||
                                            y2 >= pdest->getysize() )
            hiba( "pic8::mintavetel8-ban x1 < 0 || y1 < 0 || x2 >= xsize || y2 >= ysize!" );
    #endif

    int xsd = x2-x1+1;
    int ysd = y2-y1+1;
    int xss = psour->getxsize();
    int yss = psour->getysize();
    double s_per_d_y = (double)yss/ysd;
    double s_per_d_x = (double)xss/xsd;
    for( int y = 0; y < ysd; y++ ) {
        double sy = (y+0.5)*s_per_d_y;
        for( int x = 0; x < xsd; x++ ) {
            double sx = (x+0.5)*s_per_d_x;
            unsigned char c = psour->gpixel( sx, sy );
            pdest->ppixel( x1+x, y1+y, c );
        }
    }
}

void mintavetel8( pic8* pdest, pic8* psour ) {
    mintavetel8( pdest, psour, 0, 0, pdest->getxsize()-1, pdest->getysize()-1 );
}

// Kirakja fizikaira lock es unlockkal (Ez most mindenkeppen 320x200-ba valt):
/*void lassufizre( pic8* ppic, ddpal* ppal ) {
	if( ppic->getxsize() != 320 || ppic->getysize() != 200 )
		hiba( "lassufizre-ben ppic->getxsize != 320 || ppic->getysize != 200!" );

	unsigned char** ppc = lockbackbuffer();
	for( int y = 0; y < 200; y++ )
		memcpy( ppc[y+120]+160, ppic->getptr( 199-y ), 320 );

	unlockbackbuffer();
	if( ppal )
		ppal->set();
} */

// Egyenlore csak vizszintes es fuggoleges vonalak:
void pic8::line( int x1, int y1, int x2, int y2, unsigned char szin ) {
    // Sorrend:
    if( x2 < x1 ) {
        int tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if( y2 < y1 ) {
        int tmp = y1;
		y1 = y2;
        y2 = tmp;
    }
    if( x1 == x2 ) {
        // Fuggoleges:
        for( int y = y1; y <= y2; y++ )
            ppixel( x1, y, szin  );
        return;
    }
    if( y1 == y2 ) {
        // Vizszintes:
        for( int x = x1; x <= x2; x++ )
            ppixel( x, y1, szin  );
        return;
    }
    hiba( "pic8::line egyenlore csak vizszintes es fuggoleges vonalakat tud!" );
}


void pic8::keszitbelsot( int x1, int y1, int x2, int y2, pic8* ppic ) {
	xsize = x2-x1+1;
	ysize = y2-y1+1;
	#ifdef TEST
	if( x1 < 0 || x2 >= 640 || y1 < 0 || y2 >= 480 )
		hiba( "pic8::keszitbelsot!" );
	#endif
	for( int y = 0; y < ysize; y++ )
		sormuttomb[y] = ppic->getptr( y+y1 ) + x1;
}

