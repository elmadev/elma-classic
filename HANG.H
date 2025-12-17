#ifndef HANG_H
#define HANG_H

// 0-nem kell hang, 1 8bit mono, 2 16bit mono:
int starthang(unsigned buffsize);
void stophang(void);

// Ez h_.cpp-ben van kozvetlenul:
// DS eseten dsound2-ben van megirva:
// 0.0 - 1.0 kozott:
void setvolume(double hangero);

// Ezt meg kell irni hozza:
void callbackhang(short* sbuff, int buffsize);

#endif
