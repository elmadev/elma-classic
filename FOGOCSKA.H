#ifndef FOGOCSKA_H
#define FOGOCSKA_H

void flagtag_reset(void);
void flagtag(double time);
// FlagTagAHasFlag alapjan csak idoket allitja:
void flagtag_replay(double time);

extern int FlagTagAHasFlag, FlagTagImmunity;
extern int FlagTagAStarts; // Legkozelebbi jateknal
extern double FlagTimeA, FlagTimeB;

#endif
