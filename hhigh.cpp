#include "HANGHIGH.H"
#include "main.h"
#include "platform_impl.h"
#include "TOPOL.H"
#include "WAV.H"
#include <cstring>

int SoundInitialized = 0;

int Mute = 1;

#define WAVBANKSZAM (20)
typedef wav* wavramut;
wavramut Wavbank[WAVBANKSZAM];

// wav *Pberreg = NULL;

static wav *Pw1 = NULL, *Pw2 = NULL, *Pw3 = NULL, *Pw4 = NULL, *Psurl = NULL;
static wav2 *Pw42a = NULL, *Pw42b = NULL;

static int Mix = 100, I1 = 7526, I2 = 34648, I3 = 38766, I4 = 14490, I5 = 18906;

static double Egypermix = 1.0 / Mix;

// static int Buffsize = 500;
static int Atmenet = 499;

static int Hanghighbevoltkapcsolva = 0;

static int Wavszam = 0;
#define MAXWAV (5)
static int Ezmegy[MAXWAV];
static int Kovhang[MAXWAV];
static unsigned long Hangerok[MAXWAV];
static wav* Pwavok[MAXWAV];

static int Elsostart = 1;

void sound_engine_init(void) {
    if (!Elsostart) {
        internal_error("sound_engine_init-ban !Elsostart!");
    }
    Elsostart = 0;
    if (!SoundInitialized) {
        return;
    }

    Mute = 1;
    /*if( !starthang( Buffsize ) ) { Ez mi
        SoundInitialized = 0;
        return;
    }*/
    if (Hanghighbevoltkapcsolva) {
        internal_error("Hanghighbevoltkapcsolva igaz sound_engine_init-ban!");
    }
    Hanghighbevoltkapcsolva = 1;
    for (int i = 0; i < WAVBANKSZAM; i++) {
        Wavbank[i] = NULL;
    }
    Wavbank[WAV_BUMP] = new wav("utodes.wav", 0.25);
    Wavbank[WAV_DEAD] = new wav("torik.wav", 0.34);
    Wavbank[WAV_WIN] = new wav("siker.wav", 0.8);
    Wavbank[WAV_FOOD] = new wav("eves.wav", 0.5);
    Wavbank[WAV_TURN] = new wav("fordul.wav", 0.3);
    Wavbank[WAV_RIGHT_VOLT] = new wav("ugras.wav", 0.34);
    Wavbank[WAV_LEFT_VOLT] = Wavbank[WAV_RIGHT_VOLT];
    Psurl = new wav("dorzsol.wav", 0.44);
    Psurl->loopol(Mix);

    double berregero = 0.26;
    Pw1 = new wav("harl.wav", berregero, 0, I1 + Mix);
    Pw2 = new wav("harl.wav", berregero, I1, I2);
    Pw3 = new wav("harl.wav", berregero, I2 - Mix, I3);
    Pw4 = new wav("harl2.wav", berregero, I4, I5);

    Pw2->loopol(Mix);
    // Pw2->save( "alacsony.wav" );
    Pw4->loopol(Mix);
    Pw3->vegereilleszt(Pw4, Mix);

    Pw42a = new wav2(Pw4);
    Pw42a->reset();
    Pw42b = new wav2(Pw4);
    Pw42b->reset();

    for (int i = 0; i < MAXWAV; i++) {
        Ezmegy[i] = 0;
        Kovhang[i] = 0;
        Hangerok[i] = 0;
        Pwavok[i] = NULL;
    }
    Mute = 0;

    Pw2->hangero(0.4);
}

enum allapot { A_INDIT = 0, A_ALACSONY, A_ATMENETBE, A_ATMENET, A_MAGAS, A_VISSZAMENET };

struct mothangstruct {
    int jarmotor;
    double frekvenciamost;
    double frekvenciakell;
    int gaz;
    int allapot;
    int i_indit;
    int i_alacsony;
    int i_atmenet;
};

static mothangstruct Moth1, Moth2;

// frekvencia 1.0-tol 2.0-ig valtozik csak:
void set_motor_frequency(int is_motor1, double frequency, int gas) {
    // if( gaz )
    //	internal_error( "set_motor_frequency-ban gaz igaz!" );

    if (!SoundInitialized) {
        return;
    }

    mothangstruct* pms;
    if (is_motor1) {
        pms = &Moth1;
    } else {
        pms = &Moth2;
    }

    pms->gaz = gas;
    if (frequency > 2.0) {
        frequency = 2.0;
    }
    if (frequency < 1.0) {
        frequency = 0.0;
    }

    pms->frekvenciakell = frequency;
    // 2.0 - exp(1.0-frekvencia)
}

static double Surleromost = 0.0;
static double Surlerokell = 0.0;

void set_friction_volume(double volume) {
    if (volume > 1.0) {
        volume = 1.0;
    }
    if (volume < 0) {
        volume = 0;
    }
    Surlerokell = volume;
}

// int Wavevolt = 0;

void start_wav(int event, double volume) {
    // Wavevolt = 1;
    if (!SoundInitialized || Mute) {
        return;
    }

    if (volume <= 0.0 || volume >= 1.0) {
        internal_error("wav: volume <= 0.0 || volume >= 1.0!");
    }

    wav* pwav = Wavbank[event];

    if (Wavszam >= MAXWAV) {
        return;
    }

    //_disable();
    for (int i = 0; i < MAXWAV; i++) {
        if (!Ezmegy[i]) {
            // i. csatorna szabad!
            Wavszam++;
            Ezmegy[i] = 1;
            Kovhang[i] = 0;
            Pwavok[i] = pwav;
            Hangerok[i] = volume * 65536.0;
            //_enable();
            // if( Wavevolt )
            //	external_error( "Wavevolt utan!" );
            return;
        }
    }
    //_enable();
    internal_error("Wavszam szerint meg van csat, de nincs!");
}

// hangero meg van szorozva 65536-tal:
static void wavadd(short* buff, short* tomb, long size, unsigned long hangero) {
    for (unsigned long i = 0; i < size; i++) {
        buff[i] += short((tomb[i] * hangero) >> 16);
    }
}

void start_motor_sound(int is_motor1) {
    mothangstruct* pms;
    if (is_motor1) {
        pms = &Moth1;
    } else {
        pms = &Moth2;
    }

    pms->jarmotor = 1;
    pms->allapot = A_INDIT;
    pms->i_indit = 0;
    pms->frekvenciamost = 1.0;
    pms->frekvenciakell = 1.0;
}

// Leallitja motor hangjat (ha meghalt):
void stop_motor_sound(int is_motor1) {
    mothangstruct* pms;
    if (is_motor1) {
        pms = &Moth1;
    } else {
        pms = &Moth2;
    }

    pms->jarmotor = 0;
    pms->allapot = A_INDIT;
    pms->i_indit = 0;
    pms->frekvenciamost = 1.0;
    pms->frekvenciakell = 1.0;
}

/*static void shortcpy( short* dest, short* sour, int number ) {
    if( number == 0 )
        return;
    memcpy( dest, sour, number*2 );
}*/

static void shortadd(short* dest, short* sour, int number) {
    for (int i = 0; i < number; i++) {
        dest[i] += sour[i];
    }
}

static void motorelintezes(int mot1, short* sbuff, int buffsize) {
    mothangstruct* pms;
    if (mot1) {
        pms = &Moth1;
    } else {
        pms = &Moth2;
    }

    if (!pms->jarmotor) {
        return;
    }

    // for( int j = 0; j < Buffsize; j++ )
    //   sbuff[j] = 0;
    // return;

    // Most mindig erre a fv.-hivasra vonatkozik!:
    int counter = 0;
    int masolando = -1;
    int ucsoutani = -1;
    int novekmeny = -1;
    int i = -1;
    while (1) {
        switch (pms->allapot) {
        case A_INDIT:
            if (pms->i_indit + buffsize > Pw1->size) {
                // Meg most befejezi A_INDIT-ot:
                int masolando = Pw1->size - pms->i_indit;
                shortadd(&sbuff[counter], &Pw1->tomb[pms->i_indit], masolando);
                counter += masolando;
                pms->allapot = A_ALACSONY;
                pms->i_alacsony = Mix;
            } else {
                // Most teljes egeszeben indit-bol megy:
                int masolando = buffsize - counter;
                shortadd(&sbuff[counter], &Pw1->tomb[pms->i_indit], masolando);
                pms->i_indit += masolando;
                return;
            }
            break;
        case A_ALACSONY:
            if (pms->gaz) {
                pms->allapot = A_ATMENETBE;
                pms->i_atmenet = 0;
            } else {
                // Korbe-korbe:
                int masolando = buffsize - counter;
                if (masolando > Pw2->size - pms->i_alacsony) {
                    // Most ujra elejere kell meg ugrani:
                    masolando = Pw2->size - pms->i_alacsony;
                    shortadd(&sbuff[counter], &Pw2->tomb[pms->i_alacsony], masolando);
                    counter += masolando;
                    pms->i_alacsony = 0;
                } else {
                    // Most egy darabban lehet masolni alacsonybol:
                    shortadd(&sbuff[counter], &Pw2->tomb[pms->i_alacsony], masolando);
                    pms->i_alacsony += masolando;
                    return;
                }
            }
            break;
        case A_ATMENETBE:
            // alacsonyt egyre kisebb sullyal veszi,
            // atmenetet pedig egyre nagyobbal:
            masolando = buffsize - counter;
            ucsoutani = pms->i_atmenet + masolando;
            if (ucsoutani > Mix) {
                // Meg most befejezodik mixeles:
                ucsoutani = Mix;
                pms->allapot = A_ATMENET;
            }
            novekmeny = 0;
            for (i = pms->i_atmenet; i < ucsoutani; i++) {
                if (pms->i_alacsony >= Pw2->size) {
                    pms->i_alacsony = 0;
                }
                double arany = i * Egypermix;
                sbuff[counter + novekmeny] +=
                    arany * Pw3->tomb[i] + (1 - arany) * Pw2->tomb[pms->i_alacsony];
                pms->i_alacsony++;
                novekmeny++;
            }
            counter += novekmeny;
            pms->i_atmenet += novekmeny;
            if (counter + novekmeny == buffsize) {
                return;
            }
            break;
        case A_ATMENET:
            masolando = buffsize - counter;
            if (masolando > Pw3->size - pms->i_atmenet) {
                // Mar most at kell terni magas-ra:
                masolando = Pw3->size - pms->i_atmenet;
                shortadd(&sbuff[counter], &Pw3->tomb[pms->i_atmenet], masolando);
                counter += masolando;
                pms->allapot = A_MAGAS;
                if (mot1) {
                    Pw42a->reset(Mix);
                } else {
                    Pw42b->reset(Mix);
                }
                pms->frekvenciamost = 1.0;
            } else {
                // Most meg teljes egeszeben atmenet megy ki:
                shortadd(&sbuff[counter], &Pw3->tomb[pms->i_atmenet], masolando);
                pms->i_atmenet += masolando;
                return;
            }
            break;
        case A_MAGAS:
            int masolando = buffsize - counter;
            if (!pms->gaz && masolando > Mix) {
                pms->allapot = A_ALACSONY;
                pms->i_alacsony = Mix;
                long dtmost = 65536.0 * pms->frekvenciamost;
                for (int i = 0; i < Mix; i++) {
                    short gazshort;
                    if (mot1) {
                        gazshort = Pw42a->getnextsample(dtmost);
                    } else {
                        gazshort = Pw42b->getnextsample(dtmost);
                    }
                    short alapshort = Pw2->tomb[i];

                    double alapsuly = (double)i / Mix;
                    double gazsuly = 1.0 - alapsuly;

                    sbuff[counter + i] += alapsuly * alapshort + gazsuly * gazshort;
                }
                counter += Mix;
                break;
            } else {
                long dtmost = 65536.0 * pms->frekvenciamost;
                long dtkell = 65536.0 * pms->frekvenciakell;
                long ddt = 0;
                if (masolando > 30) {
                    ddt = (dtkell - dtmost) / ((double)masolando);
                }
                for (int i = 0; i < masolando; i++) {
                    if (mot1) {
                        sbuff[counter + i] += Pw42a->getnextsample(dtmost);
                    } else {
                        sbuff[counter + i] += Pw42b->getnextsample(dtmost);
                    }
                    dtmost += ddt;
                }
                pms->frekvenciamost = ((double)dtmost) / 65536.0;
                return;
            }
            break;
        }
    }
}

static int Surltart = 0;

// Ennek hozza kell adnia eddigi ertekhez:
static void surlodaselintezes(short* sbuff, int buffsize) {
    /*if( !Ppic8 ) {
        { Ppic8 = new pic8( 320, 2 ); }
        for( int i = 0; i < 320; i++ ) {
            Ppic8->ppixel( i, 0, 0 );
            Ppic8->ppixel( i, 1, 0 );
        }
    }
    int hatar = Surlerokell*320;
    blit8( Pscr8, Ppic8, hatar, 100 ); */

    if (Surlerokell < 0.1 && Surleromost < 0.1) {
        Surleromost = 0.0;
        return;
    }

    long eromost = 65536.0 * Surleromost;
    long erokell = 65536.0 * Surlerokell;
    long dero = (erokell - eromost) / ((double)buffsize);
    int meret = Psurl->size;
    for (int i = 0; i < buffsize; i++) {
        long ertek = Psurl->tomb[Surltart];
        Surltart++;
        if (Surltart >= meret) {
            Surltart = 0;
        }

        ertek *= eromost;
        sbuff[i] += (short)(ertek >> 16);

        eromost += dero;
    }
    Surleromost = ((double)eromost) / 65536.0;
}

// buffsize valojaban minta szam, vagyis = 2*byteszam:
void sound_mixer(short* buffer, int buffer_length) {
    if (!SoundInitialized) {
        internal_error("sound_mixer, pedig !SoundInitialized!");
    }

    memset(buffer, 0, buffer_length * 2);
    if (Mute || !State->sound_on) {
        // Nem kell hang:
        if (Wavszam > 0) {
            Wavszam = 0;
            for (int i = 0; i < MAXWAV; i++) {
                Ezmegy[i] = 0;
            }
        }
        return;
    }

    if (Atmenet >= buffer_length) {
        Atmenet = buffer_length - 1;
    }
    if (Atmenet < 20) {
        internal_error("sound_mixer-ban buffer_length < 20!");
    }

    // mothangstruct megymotorbol tudjak melyik jar meg:
    motorelintezes(1, buffer, buffer_length);
    motorelintezes(0, buffer, buffer_length);
    surlodaselintezes(buffer, buffer_length);

    // Wavok lejatszasa:
    for (int i = 0; i < MAXWAV; i++) {
        if (Ezmegy[i]) {
            int darab = buffer_length;
            if (darab > Pwavok[i]->size - Kovhang[i]) {
                darab = Pwavok[i]->size - Kovhang[i];
                Ezmegy[i] = 0;
                Wavszam--;
                if (Wavszam < 0) {
                    internal_error("Wavszam < 0 !");
                }
            }
            wavadd(buffer, &Pwavok[i]->tomb[Kovhang[i]], darab, Hangerok[i]);
            Kovhang[i] += darab;
        }
    }
}

void delay(int milliseconds) {
    // 182*sec-et adja vissza idot tortresszel egyutt!
    double kezdo = stopwatch();
    while (stopwatch() / 182.0 < kezdo / 182.0 + milliseconds / 1000.0) {
        handle_events();
    }
}
