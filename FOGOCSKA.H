#ifndef FOGOCSKA_H
#define FOGOCSKA_H

void flagtag_reset(void);
void flagtag(double time);
void flagtag_replay(double time);

extern int FlagTagAHasFlag, FlagTagImmunity;
extern int FlagTagAStarts;
extern double FlagTimeA, FlagTimeB;

#endif
