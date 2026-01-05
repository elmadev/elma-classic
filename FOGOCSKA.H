#ifndef FOGOCSKA_H
#define FOGOCSKA_H

extern bool FlagTagAHasFlag;
extern bool FlagTagImmunity;
extern bool FlagTagAStarts;
extern double FlagTimeA;
extern double FlagTimeB;

void flagtag_reset();
void flagtag(double time);
void flagtag_replay(double time);

#endif
