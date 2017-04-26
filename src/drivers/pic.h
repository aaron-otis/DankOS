#ifndef _PIC_H
#define _PIC_H

#include "../lib/stdint.h"

int PIC_init(void);
void PIC_sendEOI(uint8_t irq);
void PIC_remap(int offset1, int offset2);
void PIC_clear_mask(unsigned char);


#endif
