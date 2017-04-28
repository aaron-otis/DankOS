#ifndef _SERIAL_H
#define _SERIAL_H

#include "../lib/stdint.h"

#define UART_BUF_SIZE 32
#define HW_BUF_IDLE 0
#define HW_BUF_BUSY 1

struct UART {
    uint8_t buff[UART_BUF_SIZE];
    uint8_t *head;
    uint8_t *tail;
    uint8_t hw_buf_status;
};

extern void SER_init(void);
extern int SER_write(const char *buff, int len);
extern int SER_putc(uint8_t c);
extern void SER_int_handler(int irq, int error, void *arg);

/* COM ports. */
#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

#define COM_DATA_OFFSET 0 /* DLAB must be set to 0. */
#define COM_INTERRUPT_ENABLE 1 /* DLAB must be set to 0. */
#define COM_DIV_VAL_LOW 0 /* DLAB must be set to 1. */
#define COM_DIV_VAL_HI 1  /* DLAB must be set to 1. */
#define COM_INT_ID 2 /* Interrupt Identification and FIFO control registers. */
#define COM_LINE_CTL_REG 3 /* Line Control Register. The most significant bit 
                              of this register is the DLAB. */
#define COM_MOD_CTRL 4 /* Modem control register. */
#define COM_LINE_STATUS_REG 5 /* Line Status Register. */
#define COM_MOD_STATUS 6 /* Modem Status Register. */
#define COM_SCRATCH_REG 7 /* Scratch register. */

/* Line Protocol: consider 8N1 (8 bits, no parity, one stop bit) the default. */

/* 
 * To set the divisor to the controller:
 *
 *  Set the most significant bit of the Line Control Register. This is the DLAB 
 *  bit, and allows access to the divisor registers.
 *
 *  Send the least significant byte of the divisor value to [PORT + 0].
 *
 *  Send the most significant byte of the divisor value to [PORT + 1].
 *
 *  Clear the most significant bit of the Line Control Register. 
 */

/* 
 * Data Bits: the number of bits in a character is variable. 
 * 
 * Set this value by writing to the two least significant bits of the Line 
 * Control Register [PORT + 3]. 
 */
#define COM_CHAR_LEN_5 0
#define COM_CHAR_LEN_6 1
#define COM_CHAR_LEN_7 2
#define COM_CHAR_LEN_8 3 /* Both bits set to 1. */

/* 
 * Step Bits: Used to by the controller to verify that the sending and 
 * receiving devices are in phase. 
 */
#define COM_DATA_AVAIL 0
#define COM_TRANSMIT_EMPTY 1
#define COM_ERROR (1 << 2)
#define COM_STATUS_CHANGE (1 << 3)

#define COM_STOP_BIT_OFF 0 /* Stop bit set to 1. */
#define COM_STOP_BIT_ON (1 << 2) /* Stop bit set to 1.5 or 2. */

/*
 * Parity: To set the port parity, set bits 3, 4 and 5 of the Line Control 
 * Register [PORT + 3].
 */
#define COM_PARITY_NONE 0
#define COM_PARITY_ODD (1 << 3)
#define COM_PARITY_EVEN (0x3 << 3)
#define COM_PARITY_MARK (0x5 << 3)
#define COM_PARITY_SPACE (0x7 << 3)

#endif
