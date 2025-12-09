extern int Billint; // Billentyuzet interraptos!

extern int Sajatugras;       // Ha 1, akkor left->pozitiv forgas
extern int Otszoros;         // Ha igaz, mindig otszor rakja ki palyat
extern int Dupladuplabuffer; // Ha igaz duplan huzza az idot (Win95)

// Ha nem igaz, csak memoriaba ir es nem flip-pel:
extern int Waitforretrace; // Csak Windows-os w_v.cpp-ben allitodik

void semmi(void);

void inkremental(void);
extern long Inkremens;
