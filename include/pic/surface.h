#ifndef PIC_SURFACE_H
#define PIC_SURFACE_H

class pic8;

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

void on_resolution_change();
void update_resolution(int width, int height);

pic8* lock_backbuffer_pic(bool flipped = true);
void unlock_backbuffer_pic();

void blit_to_screen(pic8* pic);

#endif
