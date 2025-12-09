

pic8* lockbackbuffer_pic(int fizikai);
void unlockbackbuffer_pic(void);

void lockfrontbuffer_pic(void);
void unlockfrontbuffer_pic(void);
void ppixelfront(int x, int y, unsigned char szin);

void bltfront(pic8* ppic);
void bltfront(pic8* ppic, int x1, int y1, int x2, int y2);
