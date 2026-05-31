#ifndef PIC_SURFACE_H
#define PIC_SURFACE_H

class pic8;

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

void on_resolution_change();
void update_resolution(int w, int h);

pic8* lockbackbuffer_pic(bool flipped = true);
void unlockbackbuffer_pic();

void bltfront(pic8* ppic);

#endif
