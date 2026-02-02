#ifndef PLAY_H
#define PLAY_H

void menu_play(void);

// A jelenleg beolvasott palyan jatszik vissza:
// Csak menu_level menu replay-ebol szabad hasznalni:
// Azt a nevet kerik, amit topol::topol kap:
// BestTime beallitasahoz kell neki:
void replay_from_file(const char* filename);

// Vagy level == -1 es belyeg != 0 es filenev == NULL,
// vagy level >= 0 < 100 es belyeg = 0 es filenev == NULL,
// vagy level == -1 es belyeg = 0 es filenev ervenyes,
// Hamis eseten nem sikerult betolteni palyat (rec filehoz nincs lev):
// int palyabe( int level, unsigned long belyeg = 0, char* filenev = NULL );

// belyeg == 0 eseten internal level file:
void saveplay(long belyeg);

void update_top_ten(long time, char* time_message, int internal_index,
                    const char* external_filename);
// 0-ESC, 1-this level, 2-next level, 3-skip level
// Ha filenev nem NULL, akkor external file:
int menu_level(int internal_index, int nav_on_play_next, const char* time_message,
               const char* external_filename);

void loading_screen(void);

#endif
