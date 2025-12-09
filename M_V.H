// m_v.h

extern int Win_95;

void mv_main(void); // Ennek nem szabad visszajonnie!
// Ezt neha meg kell hivni!:
void mv_check(/*char* hivta,*/ int nemkellnyomas = 0);
void mv_exit(char* text = NULL); // Ezzel kell kilepni!

// MOUSE:
void getmou(int* px, int* py);
void setmou(int x, int y);
int getbutbmou(void);
int getbutjmou(void);

// 182*sec-et adja vissza idot tortresszel egyutt!
// d_stoppe.cpp-ben vannak megvalositva:
double mv_stopperido(void);
void mv_startstopper(void);
// Ezt valojaban csak crc fuggvenyek hasznaljak veletlen generalasara:
void getbiostort(unsigned short* pbios, unsigned short* ptort);

void hiba(char* text1, char* text2 = NULL, char* text3 = NULL);
void uzenet(char* text1, char* text2 = NULL, char* text3 = NULL);

int s_random(int); // Ugyanaz mint random, de az nincs WATCOM-ban
