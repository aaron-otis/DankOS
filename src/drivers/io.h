#ifndef _IO_H
#define _IO_H

#include "../lib/stdint.h"


/* Sends a 8/16/32-bit value on a I/O location. */
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

/* Receives a 8/16/32-bit value from an I/O location. */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;

    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );

    return ret;
}

/* 
 * Forces the CPU to wait for an I/O operation to complete. Only use this when 
 * there's nothing like a status register or an IRQ to tell you the info has 
 * been received. 
 */
static inline void io_wait(void) {

    asm volatile ( "outb %%al, $0x80" : : "a"(0) );
}

#endif
