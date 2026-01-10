#ifndef BEST_TIMES_H
#define BEST_TIMES_H

class topol;

void menu_internal_topten(int level, bool single);
void menu_external_topten(topol* top, bool single);
void menu_best_times();
void centiseconds_to_string(long time, char* text, bool show_hours = false);

#endif
