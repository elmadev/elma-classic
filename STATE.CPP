#include "all.h"

int Palyaszam = 55; // Vizittel egyutt

int getstatesoundon(void) { return State->soundon; }

// Van ugyanilyen neven topol.cpp-ben is ket fv.:
static void titkosread(void* mut, int hossz, FILE* h, char* nev) {
    if (fread(mut, 1, hossz, h) != hossz) {
        uzenet("Corrupt file, please delete it!", nev);
    }
    unsigned char* pc = (unsigned char*)mut;
    short a = 23, b = 9782, c = 3391;

    for (int i = 0; i < hossz; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
}

static void titkoswrite(void* mut, int hossz, FILE* h) {
    unsigned char* pc = (unsigned char*)mut;
    short a = 23, b = 9782, c = 3391;

    for (int i = 0; i < hossz; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
    if (fwrite(mut, 1, hossz, h) != hossz) {
        hiba("Nem ment iras state file-ba!");
    }
    a = 23;
    b = 9782;
    c = 3391;

    for (int i = 0; i < hossz; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
}

static int Checknumber_state_s = 123432112;
static int Checknumber_state_r = 123432221;

static char Statenev[] = "state.dat";

state_s::state_s(char* nev) {
    // Beallitjuk default ertekeket:
    memset(palyakidejei, 0, sizeof(palyakidejei));
    memset(jatekosok, 0, sizeof(jatekosok));
    jatekosokszama = 0;
    jatekosa[0] = 0;
    jatekosb[0] = 0;
    soundon = 1;
    secondarybuffer = 0; // ha igaz, kompatibilis mod
    single = 1;
    tag = 0;
    minibike = 0;
    aplayerabike = 1;
    highquality = 1;

    animatedobjects = 1;
    animatedmenus = 1;

    editfilenev[0] = 0;
    playextfilenev[0] = 0;

    resetcontrols();

    // Beolvasas ha kell:
    if (!nev) {
        nev = Statenev;
    }

    if (access(nev, 0) != 0) {
        // Nem letezik state.dat:
        save();
        return;
    }

    // Itt hagyni kell fopen-t!!!!!!!!!!!!!!!!!!!!:
    FILE* h = fopen(nev, "rb");
    if (!h) {
        hiba("Nem nyilik state file!: ", nev);
    }

    int verzio = 0;
    titkosread(&verzio, 4, h, nev);
    if (verzio != 200) {
        uzenet("File version is incorrect!", "Please rename it!", nev);
    }

    titkosread(palyakidejei, sizeof(palyakidejei), h, nev);
    titkosread(jatekosok, sizeof(jatekosok), h, nev);
    titkosread(&jatekosokszama, sizeof(int), h, nev);
    titkosread(jatekosa, sizeof(jatekosa), h, nev);
    titkosread(jatekosb, sizeof(jatekosb), h, nev);

    titkosread(&soundon, 4, h, nev);
    titkosread(&secondarybuffer, 4, h, nev);

    titkosread(&single, sizeof(int), h, nev);
    titkosread(&tag, sizeof(int), h, nev);
    titkosread(&aplayerabike, sizeof(int), h, nev);
    titkosread(&highquality, sizeof(int), h, nev);

    titkosread(&animatedobjects, sizeof(int), h, nev);
    titkosread(&animatedmenus, sizeof(int), h, nev);

    titkosread(&opciok1, sizeof(opciok1), h, nev);
    titkosread(&opciok2, sizeof(opciok2), h, nev);
    titkosread(&billplussz, sizeof(billplussz), h, nev);
    titkosread(&billminusz, sizeof(billminusz), h, nev);
    titkosread(&billsnap, sizeof(billminusz), h, nev);

    titkosread(&editfilenev[0], 20, h, nev);
    titkosread(&playextfilenev[0], 20, h, nev);

    // Magic number:
    long checknumber = 0;
    if (fread(&checknumber, 1, sizeof(long), h) != sizeof(long)) {
        uzenet("Corrupt file, please rename it!", nev);
    }
    if (checknumber != Checknumber_state_s && checknumber != Checknumber_state_r) {
        uzenet("Corrupt file, please rename it!", nev);
    }

    fclose(h);
}

void idothuz(int* pi) { *pi++; }

void state_s::reloadidok(void) {
    char* nev = Statenev;

    // if( access( nev, 0 ) != 0 )
    //	return;

    // Itt hagyni kell fopen-t!!!!!!!!!!!!!!!!!!!!:
    FILE* h = fopen(nev, "rb");
    while (!h) {
        // Var egy kicsit:
        int a = 0;
        for (int i = 0; i < 10000; i++) {
            idothuz(&a);
        }

        FILE* h = fopen(nev, "rb");
    }

    int verzio = 0;
    titkosread(&verzio, 4, h, nev);
    if (verzio != 200) {
        uzenet("File version is incorrect!", "Please rename it!", nev);
    }

    titkosread(palyakidejei, sizeof(palyakidejei), h, nev);

    fclose(h);
}

state_s::~state_s(void) {}

void state_s::save(void) {
    // Itt hagyni kell fopen-t!!!!!!!!!!!!!!!!!!!!:
    FILE* h = fopen(Statenev, "wb");
    if (!h) {
        uzenet("Could not open for write file!: ", Statenev);
    }

    int verzio = 200;
    titkoswrite(&verzio, 4, h);
    titkoswrite(palyakidejei, sizeof(palyakidejei), h);
    titkoswrite(jatekosok, sizeof(jatekosok), h);
    titkoswrite(&jatekosokszama, sizeof(int), h);
    titkoswrite(jatekosa, sizeof(jatekosa), h);
    titkoswrite(jatekosb, sizeof(jatekosb), h);

    titkoswrite(&soundon, 4, h);
    titkoswrite(&secondarybuffer, 4, h);

    titkoswrite(&single, sizeof(int), h);
    titkoswrite(&tag, sizeof(int), h);
    titkoswrite(&aplayerabike, sizeof(int), h);
    titkoswrite(&highquality, sizeof(int), h);

    titkoswrite(&animatedobjects, sizeof(int), h);
    titkoswrite(&animatedmenus, sizeof(int), h);

    titkoswrite(&opciok1, sizeof(opciok1), h);
    titkoswrite(&opciok2, sizeof(opciok2), h);
    titkoswrite(&billplussz, sizeof(billplussz), h);
    titkoswrite(&billminusz, sizeof(billminusz), h);
    titkoswrite(&billsnap, sizeof(billminusz), h);

    titkoswrite(&editfilenev[0], 20, h);
    titkoswrite(&playextfilenev[0], 20, h);

    long checknumber = Checknumber_state_r;
    if (fwrite(&checknumber, 1, sizeof(long), h) != sizeof(long)) {
        hiba("Nem megy beolvasas state file-ba!: ", Statenev);
    }

    fclose(h);
}

static void exportegylevel(FILE* h, palyaegyfeleidok* pidok, int single) {
    for (int i = 0; i < pidok->idokszama; i++) {
        char idostr[40];
        ido2string(pidok->idok[i], idostr, 1);
        fprintf(h, "    ");
        fprintf(h, idostr);
        for (int k = 0; k < (12 - strlen(idostr)); k++) {
            fprintf(h, " ");
        }
        fprintf(h, pidok->nevek1[i]);
        if (!single) {
            fprintf(h, ", %s", pidok->nevek2[i]);
        }
        fprintf(h, "\n");
    }
}

// Ha nincs egy palyan ido, tiz percet szamol helyette:
// Ez itt szazadmasodpercben van megadva:
static int const Nincsmegpalyaido = 100 * 60 * 10;

// Anonymous total times:
void state_s::exportanonim(FILE* h, int single, char* text1, char* text2, char* text3) {
    int sum = 0;
    for (int i = 0; i < Palyaszam - 1; i++) { // ucso palyat nem szamitjuk
        int levelsum = 100000000;
        // Single:
        palyaegyfeleidok* pidok = &palyakidejei[i].singleidok;
        if (pidok->idokszama > 0) {
            levelsum = pidok->idok[0];
        }
        if (!single) {
            // Multiplayer:
            palyaegyfeleidok* pidok = &palyakidejei[i].multiidok;
            if (pidok->idokszama > 0 && levelsum > pidok->idok[0]) {
                levelsum = pidok->idok[0];
            }
        }
        if (levelsum >= Nincsmegpalyaido) {
            sum += Nincsmegpalyaido;
        } else {
            sum += levelsum;
        }
    }

    fprintf(h, "%s\n%s\n%s\n", text1, text2, text3);

    char idostr[40];
    ido2string(sum, idostr, 1);
    fprintf(h, idostr);
    fprintf(h, "\n\n");
}

void state_s::exportjatekossum(FILE* h, char* nev, int single) {
    int sum = 0;
    for (int i = 0; i < Palyaszam - 1; i++) { // ucso palyat nem szamitjuk
        int levelsum = 100000000;

        // Single times:
        palyaegyfeleidok* pidok = &palyakidejei[i].singleidok;
        for (int j = 0; j < pidok->idokszama; j++) {
            if( strcmp( nev, pidok->nevek1[j] ) == 0 /*||
				(!single && strcmp( nev, pidok->nevek2[j] ) == 0)*/ ) {
                // Megvan nev:
                levelsum = pidok->idok[j];
                break;
            }
        }

        if (!single) {
            // Multi player times:
            palyaegyfeleidok* pidok = &palyakidejei[i].multiidok;
            for (int j = 0; j < pidok->idokszama; j++) {
                if (strcmp(nev, pidok->nevek1[j]) == 0 || strcmp(nev, pidok->nevek2[j]) == 0) {
                    // Megvan nev:
                    if (levelsum > pidok->idok[j]) {
                        levelsum = pidok->idok[j];
                    }
                    break;
                }
            }
        }

        if (levelsum >= Nincsmegpalyaido) {
            sum += Nincsmegpalyaido;
        } else {
            sum += levelsum;
        }
    }
    char idostr[40];
    ido2string(sum, idostr, 1);
    fprintf(h, idostr);
    for (int k = 0; k < (12 - strlen(idostr)); k++) {
        fprintf(h, " ");
    }
    fprintf(h, "%s\n", nev);
}

void state_s::export_(void) {
    FILE* h = fopen("stats.txt", "wt");
    if (!h) {
        uzenet("Could not open STATS.TXT for writing!");
    }

    fprintf(h, "This text file is generated automatically each time you quit the\n");
    fprintf(h, "ELMA.EXE program. If you modify this file, you will loose the\n");
    fprintf(h, "changes next time you run the game. This is only an output file, the\n");
    fprintf(h, "best times are stored in the STATE.DAT binary file.\n");
    fprintf(h, "Registered version 1.0\n");

    fprintf(h, "\n");

    // Single palyak:
    fprintf(h, "Single player times:\n");
    fprintf(h, "\n");
    for (int i = 0; i < Palyaszam - 1; i++) { // utolsot nem irjuk ki
        fprintf(h, "Level %d, %s:\n", i + 1, getleveldescription(i));
        palyaegyfeleidok* pidok = &palyakidejei[i].singleidok;
        exportegylevel(h, pidok, 1);
        fprintf(h, "\n");
    }

    fprintf(h, "\n");
    // Multiplayer palyak:
    fprintf(h, "Multiplayer times:\n");
    fprintf(h, "\n");
    for (int i = 0; i < Palyaszam - 1; i++) { // utolsot nem irjuk ki
        fprintf(h, "Level %d, %s:\n", i + 1, getleveldescription(i));
        palyaegyfeleidok* pidok = &palyakidejei[i].multiidok;
        exportegylevel(h, pidok, 0);
        fprintf(h, "\n");
    }

    // Single player total times:
    fprintf(h, "The following are the single player total times for individual players.\n");
    fprintf(h, "If a player doesn't have a time in the top ten for a level, this\n");
    fprintf(h, "will add ten minutes to the total time.\n");

    for (int i = 0; i < jatekosokszama; i++) {
        exportjatekossum(h, State->jatekosok[i].nev, 1);
    }
    fprintf(h, "\n");

    // Multi player total times:
    fprintf(h, "The following are the combined total times for individual players. For each\n");
    fprintf(h, "level the best time is choosen of either the player's single player best\n");
    fprintf(h, "time, or the best multiplayer time where the player was one of the two\n");
    fprintf(h, "players.\n");
    fprintf(h, "If a player doesn't have such a time for a level, this will add ten\n");
    fprintf(h, "minutes to the total time.\n");

    for (int i = 0; i < jatekosokszama; i++) {
        exportjatekossum(h, State->jatekosok[i].nev, 0);
    }
    fprintf(h, "\n");

    // Anonymous total times:
    exportanonim(h, 1, "The following is the anonymous total time of the best single player",
                 "times. If there is no single player time for a level, this will",
                 "add ten minutes to the total time.");

    exportanonim(h, 0, "The following is the anonymous combined total time of the best",
                 "single or multiplayer times. If there is no single or multiplayer",
                 "time for a level, this will add ten minutes to the total time.");

    fclose(h);
}

void state_s::resetcontrols(void) {
    void di_reset_state(state_s * mut);
    di_reset_state(this);
}

int getjatekosindex(char* nev) {
    for (int i = 0; i < MAXJATEKOS; i++) {
        jatekos* pjatekos = &State->jatekosok[i];
        if (strcmp(pjatekos->nev, nev) == 0) {
            return i;
        }
    }
    hiba("getjatekosindex-ben nem talalja nevet!");
    return 0;
}
