#ifndef M_V_H
#define M_V_H

void message_box(const char* text);

// Ezt neha meg kell hivni!:
void handle_events(/*char* hivta,*/ int nemkellnyomas = 0);
void mv_exit(); // Ezzel kell kilepni!

// MOUSE:
void get_mouse_position(int* x, int* y);
void set_mouse_position(int x, int y);
bool left_mouse_clicked();
bool right_mouse_clicked();
void show_cursor();
void hide_cursor();

// 182*sec-et adja vissza idot tortresszel egyutt!
// d_stoppe.cpp-ben vannak megvalositva:
double mv_stopperido(void);
void mv_startstopper(void);

void hiba(const char* text1, const char* text2 = NULL, const char* text3 = NULL);
void uzenet(const char* text1, const char* text2 = NULL, const char* text3 = NULL);

int s_random(int); // Ugyanaz mint random, de az nincs WATCOM-ban

#endif
