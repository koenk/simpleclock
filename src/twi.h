#ifndef TWI_H
#define TWI_H

#include "types.h"

void twi_init(void);
bool twi_start(u8 addr, bool do_read);
void twi_stop(void);
bool twi_write(u8 data);
u8 twi_read(bool last_read);

#endif
