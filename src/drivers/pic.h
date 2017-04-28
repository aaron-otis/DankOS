#ifndef _PIC_H
#define _PIC_H

#include "../lib/stdint.h"

/* Offsets. */
#define PIC_MASTER_OFFSET 0x20
#define PIC_SLAVE_OFFSET 0x2F
#define PIC_NUM_IRQS 8

int PIC_init(void);
void PIC_sendEOI(uint8_t irq);
void PIC_remap(int offset1, int offset2);
void PIC_clear_mask(unsigned char);


#endif
