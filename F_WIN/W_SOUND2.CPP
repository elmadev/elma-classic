#include "w_all.h"

static LPDIRECTSOUND Pds = NULL;
static LPDIRECTSOUNDBUFFER Pribuffer = NULL;
static int Pribuffersize_byte = 0;
static int Bitmode16 = 0;

// AppCreateWritePrimaryBuffer eleje allitja be,
// State->secondarybuffer alapjan parameterkent kapja:
int Hangsecondaryeloir = 0;

void dsoundfinish(void) {
    if (!Hangenabled) {
        return;
    }

    if (Pds) {
        IDirectSound_Release(Pds);
    }
    // Pds->lpVtbl->Release( Pds );
    Pds = NULL;
}

extern HWND GhWnd;

/*void setvolume( double dvolume ) {
    if( !Hangenabled )
        return;

    if( dvolume < 0.02 )
        Hangeronulla = 1;
    else
        Hangeronulla = 0;

    int ivolume = (DSBVOLUME_MAX - DSBVOLUME_MIN)*dvolume +
                    DSBVOLUME_MIN;
    if( ivolume > DSBVOLUME_MAX )
        ivolume = DSBVOLUME_MAX;
    if( ivolume < DSBVOLUME_MIN )
        ivolume = DSBVOLUME_MIN;
    if( Pribuffer )
        Pribuffer->SetVolume( ivolume );
}*/

static void masodlagosbuffercreate(void) {
    // Itt belepeskor Hangenabled be can allitva 0-ra.

    DSBUFFERDESC dsbdesc;
    HRESULT hr;
    BYTE* pbData = NULL;
    BYTE* pbData2 = NULL;
    DWORD dwLength;
    DWORD dwLength2;
    LPDIRECTSOUNDBUFFER pdsb = NULL;

    Pribuffersize_byte = 30000;

    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));

    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = 0;
    dsbdesc.dwFlags |= DSBCAPS_STATIC;
    // Use new GetCurrentPosition() accuracy (DirectX 2 feature)
    dsbdesc.dwFlags |= DSBCAPS_CTRLDEFAULT | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME;
    // if (pFileInfo->fSticky)
    //     dsbdesc.dwFlags   |= DSBCAPS_STICKYFOCUS;
    dsbdesc.dwBufferBytes = Pribuffersize_byte;

    // char tmp[20];
    // sprintf( tmp, "%d", (int)dsbdesc.dwBufferBytes );
    // uzenet( tmp );

    // Beallitjuk formatumot:
    WAVEFORMATEX formatstruct;
    formatstruct.wFormatTag = 1;
    formatstruct.nChannels = 1;
    formatstruct.nSamplesPerSec = 11025;
    formatstruct.nAvgBytesPerSec = 22050;
    formatstruct.nBlockAlign = 2;
    formatstruct.wBitsPerSample = 16;
    formatstruct.cbSize = 0;
    dsbdesc.lpwfxFormat = &formatstruct;

    hr = Pds->CreateSoundBuffer(&dsbdesc, &pdsb, NULL);
    if (hr != DS_OK) {
        return; // Hangenabled most 0
                // uzenet( "DirectSound CreateSoundBuffer unsuccessfull!" );
    }

    // Ok, lock it down, and copy the memory to it.
    hr = pdsb->Lock(0, Pribuffersize_byte, (void**)&pbData, &dwLength, (void**)&pbData2, &dwLength2,
                    0L);

    if (hr != 0 || pbData == NULL) {
        uzenet("DirectSound Lock unsuccessfull!");
    }

    memset(pbData, 0, dwLength);
    if (pbData2) {
        memset(pbData2, 0, dwLength2);
    }

    // Ok, now unlock the buffer, we don't need it anymore.
    pdsb->Unlock(pbData, dwLength, pbData2, dwLength2);

    pbData = NULL;
    Pribuffer = pdsb;

    hr = IDirectSoundBuffer_Play(Pribuffer, 0, 0, DSBPLAY_LOOPING);
    if (!SUCCEEDED(hr)) {
        uzenet("DirectSound Play unsuccessfull!");
    }

    Hangenabled = 1;
}

void AppCreateWritePrimaryBuffer(int secondaryeloir_p) {
    Hangsecondaryeloir = secondaryeloir_p;

    // Letiltjuk hangadast, majd vegen engedelyezzuk:
    Hangenabled = 0;
    Bitmode16 = 1;

    // Set up DSBUFFERDESC structure.
    // Zero it out:
    DSBUFFERDESC dsbdesc;
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
    // Buffer size is determined by sound hardware.
    dsbdesc.dwBufferBytes = 0;
    // Must be NULL for primary buffers:
    dsbdesc.lpwfxFormat = NULL;

    // Obtain write-primary cooperative level, vagy normal:
    if (Hangsecondaryeloir ||
        DS_OK != IDirectSound_SetCooperativeLevel(Pds, GhWnd, DSSCL_WRITEPRIMARY)) {
        // Nem tudta beallitani eros cooperative levelt,
        // ezert gyenget fog probalni es masodlagus buffert:
        HRESULT hr = IDirectSound_SetCooperativeLevel(Pds, GhWnd, DSSCL_NORMAL);
        if (DS_OK != hr) {
            return; // Hangenabled most 0
                    // uzenet( "Could not Set Cooperative Level in DS!" );
        }
        masodlagosbuffercreate();
        return;
    }
    // Try to create buffer.
    HRESULT hr = IDirectSound_CreateSoundBuffer(Pds, &dsbdesc, &Pribuffer, NULL);
    if (DS_OK != hr) {
        masodlagosbuffercreate();
        return;
    }

    // Set primary buffer to desired format:
    // Set up wave format structure.
    tWAVEFORMATEX pcmwf;
    memset(&pcmwf, 0, sizeof(tWAVEFORMATEX));
    pcmwf.wFormatTag = WAVE_FORMAT_PCM;
    pcmwf.nChannels = 1;
    pcmwf.nSamplesPerSec = 11025;
    pcmwf.nBlockAlign = 2;
    pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;
    pcmwf.wBitsPerSample = 16;
    hr = Pribuffer->SetFormat(&pcmwf);
    if (DS_OK != hr) {
        // Nem tudta beallitani 16 bites formatumot,
        // most 8 biteset probalja:
        Bitmode16 = 0;
        memset(&pcmwf, 0, sizeof(tWAVEFORMATEX));
        pcmwf.wFormatTag = WAVE_FORMAT_PCM;
        pcmwf.nChannels = 1;
        pcmwf.nSamplesPerSec = 11025;
        pcmwf.nBlockAlign = 1;
        pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;
        pcmwf.wBitsPerSample = 8;
        hr = Pribuffer->SetFormat(&pcmwf);
        if (DS_OK != hr) {
            // 8 bites sem megy, ezert masodlagosat probal:
            masodlagosbuffercreate();
            return;
        }
    }

    // Get the buffer size:
    DSBCAPS dsbcaps;
    dsbcaps.dwSize = sizeof(DSBCAPS);
    Pribuffer->GetCaps(&dsbcaps);
    Pribuffersize_byte = dsbcaps.dwBufferBytes;
    // El is inditjuk:
    hr = IDirectSoundBuffer_Play(Pribuffer, 0, 0, DSBPLAY_LOOPING);
    if (!SUCCEEDED(hr)) {
        hiba("Nem indul dorzsol startmotor-ban!");
    }
    Hangenabled = 1; // Engedelyezzuk hangadast
}

static int Dsoundinited = 0;

// Parametert State->secondarybuffer alapjan kapja:
void initdsound(int secondaryeloir) {
    if (Dsoundinited) {
        hiba("78egry78yt7");
    }
    Dsoundinited = 1;

    if (!Hangenabled) {
        hiba("7868t68t");
    }

    Hangenabled = 0;

    HRESULT sret = DirectSoundCreate(NULL, &Pds, NULL);

    if (sret != DS_OK) {
        return; // Hangenabled most 0
                // hiba( "Could not create DirectSound onject!" );
    }

    AppCreateWritePrimaryBuffer(secondaryeloir);
}

// Ezt lefoglalas megszorozza kettovel 16 bit eseten:
static const int Maxtelikbyte = 1100; // 1/10 sec
// Eloszor mindig bele fog esni tartomanyba:
static int Apos = 0, Bpos = 0, Atfed = 0, Elsohang = 1;

static int Hanghivasszam = 0;

static char Hivtatomb[100][100];
static int Hivtakov = 0;

void hivtakiir(void) {
    FILE* h = fopen("hivta.txt", "wt");
    if (!h) {
        hiba("Nem nyilik hivta.txt!");
    }

    for (int i = 0; i < 100; i++) {
        fprintf(h, "%s\n", Hivtatomb[Hivtakov]);
        Hivtakov++;
        if (Hivtakov >= 100) {
            Hivtakov = 0;
        }
    }

    fclose(h);
    hiba("hivta.txt kiirva!");
}

int Fordulnyomva = 0;
int Mostvoltegynemcurpos = 0;

void hang(void /*char* hivta*/) {
    /*if( strlen( hivta ) > 90 )
        hiba( "strlen( hivta ) > 90!" );
    strcpy( Hivtatomb[Hivtakov], hivta );
    Hivtakov++;
    if( Hivtakov >= 100 )
        Hivtakov = 0;




    int torolfv( void );
    if( torolfv() ) {
    //	hiba( "Omega nagyobb!" );
        Hanghivasszam++;
    }
    if( Hanghivasszam > 2000 )
        hivtakiir();
    */

    if (!Hangenabled) {
        return;
    }

#ifdef TEST
    if (!Pribuffer) {
        hiba("hang-ban !Pribuffer!");
    }
#endif

    unsigned long pos = 0;
    HRESULT hr;
    DWORD DwDXVersion;
    if (DwDXVersion >= 0x500) {
        hr = Pribuffer->GetCurrentPosition(NULL, &pos);
    } else {
        unsigned long posdummy = 0;
        hr = Pribuffer->GetCurrentPosition(&posdummy, &pos);
    }
    if (DS_OK != hr) {
        Mostvoltegynemcurpos = 1;
        return;
        /*if( DS_OK == DSERR_INVALIDPARAM )
            hiba( "GetCurrentPosition DSERR_INVALIDPARAM" );
        if( DS_OK == DSERR_PRIOLEVELNEEDED )
            hiba( "GetCurrentPosition DSERR_PRIOLEVELNEEDED" );
        hiba( "Nem GetCurrentPosition es nincs meg ok!" );
        */
    }

    if (Fordulnyomva) {
        hiba("Itt van hangban!");
    }

    if (Elsohang) {
        Elsohang = 0;
        Apos = 0;
        Bpos = Maxtelikbyte;
        Atfed = 0;
    }

    /*FILE* htmp = fopen( "ahang.txt", "at" );
    if( !htmp )
        uzenet( "jgfh" );
    fprintf( htmp, "Apos %d Bpos %d pos %d\n",
                    Apos, Bpos, pos );
    fclose( htmp );*/

    // Ha pos nincsen benne Apos-Bpos intervallumban, kilepunk:
    if (Atfed) {
        if (!(pos >= Apos || pos < Bpos)) {
            return;
        }
    } else {
        // Nem fed at:
        if (pos < Apos || pos >= Bpos) {
            return;
        }
    }

    // Kiirunk egy buffernyi adatot:
    LPVOID lpvPtr1;
    DWORD dwBytes1;
    LPVOID lpvPtr2;
    DWORD dwBytes2;
    // Obtain write pointer.
    hr = Pribuffer->Lock(Bpos, Maxtelikbyte, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);
    // If DSERR_BUFFERLOST is returned, restore and retry lock.
    if (DSERR_BUFFERLOST == hr) {
        Pribuffer->Restore();
        hr = Pribuffer->Lock(Bpos, Maxtelikbyte, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);
    }
    if (DS_OK != hr) {
        // Ezt lehet hogy ki kene szedni:
        uzenet("AppMixIntoPrimaryBuffer-ben nem lockolt!");
        return;
    }

    // Mix data into the returned pointers.
    void CustomMixer(void* buffer1, int byteszam1, void* buffer2, int byteszam2, int bitmod16);
    CustomMixer(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2, Bitmode16);
    // Release the data back to DirectSound.
    hr = Pribuffer->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);

    // Beallitjuk uj poziciokat:
    Atfed = 0;
    Apos += Maxtelikbyte;
    Bpos += Maxtelikbyte;
    if (Apos >= Pribuffersize_byte) {
        Apos -= Pribuffersize_byte;
    }
    if (Bpos >= Pribuffersize_byte) {
        Bpos -= Pribuffersize_byte;
        Atfed = 1;
    }

    /*if( DS_OK == hr ) {
       // Success.
       return; // TRUE;
    }
    // Lock or Unlock failed.
    return FALSE; */
}

/*void tesztlock( void ) {
    LPVOID lpvPtr1;
    DWORD dwBytes1;
    LPVOID lpvPtr2;
    DWORD dwBytes2;
    // Obtain write pointer.
    HRESULT hr = Pribuffer->Lock( Bpos, Maxtelikbyte,
                          &lpvPtr1, &dwBytes1,
                          &lpvPtr2, &dwBytes2, 0 );
    // If DSERR_BUFFERLOST is returned, restore and retry lock.
    if( DSERR_BUFFERLOST == hr ) {
        Pribuffer->Restore();
        hr = Pribuffer->Lock( Bpos, Maxtelikbyte,
                              &lpvPtr1, &dwBytes1,
                               &lpvPtr2, &dwBytes2, 0);
    }
    if( DS_OK != hr ) {
        Hangenabled = 0;
        uzenet( "AAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
                "AAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
                "AAAAAAAAAAAAAAAAAAAAAAAAAAAAA" );
    }

    hr = Pribuffer->Unlock( lpvPtr1, dwBytes1,
                               lpvPtr2, dwBytes2);
}*/

// Torli hardware hang buffert,
void nullazhangot(void) {
    if (!Hangenabled) {
        return;
    }

    // Kiirunk egy buffernyi adatot:
    LPVOID lpvPtr1;
    DWORD dwBytes1;
    LPVOID lpvPtr2;
    DWORD dwBytes2;
    // Obtain write pointer.
    HRESULT hr =
        Pribuffer->Lock(0, Pribuffersize_byte, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);
    // If DSERR_BUFFERLOST is returned, restore and retry lock.
    if (DSERR_BUFFERLOST == hr) {
        Pribuffer->Restore();
        hr = Pribuffer->Lock(Bpos, Maxtelikbyte, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);
    }
    if (!SUCCEEDED(hr)) {
        // hiba( "Nullazban nem jo hr!" );
        return;
    }

    int nullabyte = 0;
    if (Bitmode16) {
        nullabyte = 0;
    } else {
        nullabyte = 128;
    }

    if (lpvPtr1) {
        memset(lpvPtr1, nullabyte, dwBytes1);
    }
    if (lpvPtr2) {
        memset(lpvPtr2, nullabyte, dwBytes2);
    }

    hr = Pribuffer->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
}

// 0.0 - 1.0 kozott:
/*void setvolume( double hangero ) {
    if( !Hangenabled )
        return;

    hangero++;
}*/
