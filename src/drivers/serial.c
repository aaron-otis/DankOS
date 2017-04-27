#include "serial.h"
#include "io.h"
#include "../lib/string.h"

#define DLAB_BIT 0x80
#define DIV_LOW_BITS 0x3
#define DIV_HI_BITS 0

/* IIR */
#define FIFO_ENABLED (0x3 << 6)
#define INT_PENDING 1
#define RLS_INT (0x3 << 1)

/* FCR */
#define FIFO_INT_TRIG_1 0
#define FIFO_INT_TRIG_4 1
#define FIFO_INT_TRIG_8 (1 << 1)
#define FIFO_INT_TRIG_14 0x3
#define ENABLE_FIFO 1
#define FIFO_CLR_RECV (1 << 1)
#define FIFO_CLR_TRANS (1 << 2)

/* MCR */
#define DATA_READY 1
#define REQ_SEND (1 << 1)
#define AUX_2 (1 << 3)

/* IER */
#define ENABLE_TX (1 << 1)

static struct UART uart;

extern void SER_init(void) {
    outb(COM1 + COM_INTERRUPT_ENABLE, 0); /* Disable interrupts. */
    outb(COM1 + COM_LINE_CTL_REG, DLAB_BIT); /* Enable DLAB. */
    outb(COM1 + COM_DIV_VAL_LOW, DIV_LOW_BITS); /* Divisor low bits. */
    outb(COM1 + COM_DIV_VAL_HI, DIV_HI_BITS); /* Divisor high bits. */
    outb(COM1 + COM_LINE_CTL_REG, COM_CHAR_LEN_8 | COM_PARITY_NONE | 
     COM_STOP_BIT_OFF); /* Set 8 bit char, no parity, one stop bit. */

    /* FIFO enabled. */
    outb(COM1 + COM_INT_ID, FIFO_INT_TRIG_14 | ENABLE_FIFO | FIFO_CLR_TRANS 
     | FIFO_CLR_RECV);
    outb(COM1 + COM_MOD_CTRL, DATA_READY | REQ_SEND | AUX_2);
    outb(COM1 + COM_INTERRUPT_ENABLE, ENABLE_TX); /* Enable TX interrupts. */

    memset(&uart, 0, sizeof(uart));
}

extern int SER_write(const char *buff, int len) {
}

extern void SER_int_handler(int irq, int error, void *arg) {
}
