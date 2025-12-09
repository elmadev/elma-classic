
void play(void);

// A jelenleg beolvasott palyan jatszik vissza:
// Csak afterplay menu replay-ebol szabad hasznalni:
// Azt a nevet kerik, amit topol::topol kap:
// Ezenlegjobbido beallitasahoz kell neki:
void replayjelenlegi(char* filenev);

// Vagy level == -1 es belyeg != 0 es filenev == NULL,
// vagy level >= 0 < 100 es belyeg = 0 es filenev == NULL,
// vagy level == -1 es belyeg = 0 es filenev ervenyes,
// Hamis eseten nem sikerult betolteni palyat (rec filehoz nincs lev):
// int palyabe( int level, unsigned long belyeg = 0, char* filenev = NULL );

// belyeg == 0 eseten internal level file:
void saveplay(long belyeg);

void idoelintezes(long ido, char* valasz, int level, char* filenev);
// 0-ESC, 1-this level, 2-next level, 3-skip level
// Ha filenev nem NULL, akkor external file:
int afterplay(int level, int nextdefault, char* idoeredmeny, char* filenev);

void kiirloading(void);
