
void play(void);

// replays in current level
// can only be used from replay in afterplay menu
// asks for name received by topol::topol
// needed to set ezenlegjobbido
void replayjelenlegi(char* filenev);

// either level == -1  belyeg != 0 and filenev == NULL,
// or level >= 0 < 100 and belyeg = 0 and filenev == NULL,
// or level == -1 and belyeg = 0 and valid filenev,
// if false, could not load lev (no lev for rec):
// int palyabe( int level, unsigned long belyeg = 0, char* filenev = NULL );

// if belyeg == 0 then internal level file:
void saveplay(long belyeg);

void idoelintezes(long ido, char* valasz, int level, char* filenev);
// 0-ESC, 1-this level, 2-next level, 3-skip level
// if filenev is not NULL, then external file:
int afterplay(int level, int nextdefault, char* idoeredmeny, char* filenev);

void kiirloading(void);
