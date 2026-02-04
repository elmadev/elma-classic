#ifndef TRANSPARENCY_H
#define TRANSPARENCY_H

class pic8;

unsigned char* create_transparency_buffer(pic8* pic, unsigned char transparency,
                                          unsigned short* transparency_length);

#endif
