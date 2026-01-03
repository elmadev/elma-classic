#ifndef DOBOZ_H
#define DOBOZ_H

class kiskep;
class pic8;

void dobozki(pic8* ppic8, kiskep* pkiskep, vect2 u, vect2 v, vect2 r);

extern int Affinitas;
// kozepet es iranyt majd o megszorozza arany-okkal:
void setaffinitas(vect2 kozep, vect2 irany, double nagysag, double aranyx, double aranyy);

#endif
