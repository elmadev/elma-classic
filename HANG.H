
// 0-nem kell hang, 1 8bit mono, 2 16bit mono:
int starthang(unsigned buffsize);
void stophang(void);
// DS eseten dsound2-ben van megirva:
// Ezt kell allandoan hivogatni:
void hang(void /*char* hivta*/);
// DS-ben nem csinal semmit:
void nullazhangot(void);

// Ez h_.cpp-ben van kozvetlenul:
// DS eseten dsound2-ben van megirva:
// 0.0 - 1.0 kozott:
void setvolume(double hangero);

// Ezt meg kell irni hozza:
void callbackhang(short* sbuff, unsigned char* cbuff);
