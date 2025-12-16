#ifndef BEST_TIMES_H
#define BEST_TIMES_H

void menu_internal_topten(int level, int single);
void menu_external_topten(topol* top, int single);
void menu_best_times(void);
void centiseconds_to_string(long time, char* text, int show_hours = 0);

#endif
