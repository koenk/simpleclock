#ifndef DISPLAY_H
#define DISPLAY_H

#include "types.h"

#define DISPLAY_NUM_DIGITS 4

void display_init(void);
void display_setsegs(u8 segs[DISPLAY_NUM_DIGITS], u8 brightness);
void display_shownum(u16 num, bool colon, u8 brightness);

#endif
