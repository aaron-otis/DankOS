#include "serial.h"
#include "io.h"
#include "interrupts.h"
#include "pic.h"
#include "../lib/string.h"
#include "../lib/stdlib.h"

#define DLAB_BIT 0x80
#define DIV_LOW_BITS 0x3
#define DIV_HI_BITS 0

/* Interrupt masks. */
#define COM1_MASK 4
#define COM2_MASK 3
#define COM3_MASK COM1_MASK
#define COM4_MASK COM2_MASK

/* IIR */
#define FIFO_ENABLED (0x3 << 6)
#define INT_PENDING 1
#define RLS_INT (0x3 << 1)
#define TRANSMIT_REG_EMPTY (1 << 1)

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

/* LSR */
#define EMPTY_TRANSMIT_REG (1 << 5)
#define EMPYT_DATA_REG (1 << 6)

static struct UART uart;

static void buffer_init(struct UART *uart) {
    uart->head = uart->tail = uart->buff;
    uart->hw_buf_status = 0;
}

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

    buffer_init(&uart); /* Initialize buffer. */

    /* Register COM 1 interrupt handler. */
    IRQ_set_handler(COM1_MASK + PIC_MASTER_OFFSET, SER_int_handler, &uart);
    /* Register COM 2 interrupt handler. */
    IRQ_set_handler(COM1_MASK + PIC_MASTER_OFFSET, SER_int_handler, &uart);

    PIC_clear_mask(COM1_MASK); /* Clear mask for UART (COM 1). */
    PIC_clear_mask(COM2_MASK); /* Clear mask for UART (COM 2). */
}

static int transmit_buf_status() {
    return inb(COM1 + COM_LINE_STATUS_REG) & EMPTY_TRANSMIT_REG;
}

static void hw_write(uint8_t byte) {
    /* Check for empty hardware buffer. */
    if (uart.hw_buf_status) {
        while (!transmit_buf_status())
                ; /* Poll until transmit buffer it empty. */
    }

    outb(COM1, byte); /* Write byte to hardware buffer. */

    /* Update hardware buffer flag. */
    uart.hw_buf_status = HW_BUF_BUSY;
}


static int queue_byte(uint8_t byte, struct UART *uart) {
    int int_enabled = 0;

    if (are_interrupts_enabled()) {
        int_enabled = 1;
        CLI;
    }

    if (uart->head == uart->tail) { /* Check if buffer is empty. */

        /* If hardware buffer is empty, write to hardware immediately. */
        hw_write(byte); /* Write byte directly if so. */

        if (int_enabled)
            STI;

        return EXIT_SUCCESS;
    }
    /* Check that buffer is not full. */
    else if (uart->head != uart->tail - 1 && 
     (uart->head != uart->buff || uart->tail != uart->buff - 1)) {

        *uart->tail++ = byte; /* Add byte to queue. */

        /* Wrap tail if at end of queue. */
        if (uart->tail >= uart->buff + UART_BUF_SIZE)
            uart->tail = uart->buff;

        if (int_enabled)
            STI;

        return EXIT_SUCCESS;
    }

    if (int_enabled)
        STI;

    return EXIT_FAILURE; /* Buffer is full. */
}

extern int SER_write(const char *buff, int len) {
    int i, res = 0, int_enabled = 0;

    if (are_interrupts_enabled()) {
        int_enabled = 1;
        CLI;
    }

    /* Buffer each character. */
    for (i = 0; i < len && res == EXIT_SUCCESS; i++)
        res = queue_byte(buff[i], &uart);

    if (int_enabled)
        STI;

    return i;
}

extern int SER_putc(uint8_t c) {
    int res = 0, int_enabled = 0;

    if (are_interrupts_enabled()) {
        int_enabled = 1;
        CLI;
    }

    res = queue_byte(c, &uart);

    if (int_enabled)
        STI;

    return res;
}

static void consume_next(struct UART *uart) {

    if (uart->head != uart->tail) { /* Check if buffer is empty. */
        hw_write(*uart->head++);

        /* Wrap head to beginning if at the end. */
        if (uart->head >= uart->buff + UART_BUF_SIZE)
            uart->head = uart->buff;
    }
}

extern void SER_int_handler(int irq, int error, void *arg) {
    struct UART *uart = (struct UART *) arg; /* Cast to the correct type. */
    uint8_t res;

    res = inb(COM1 + COM_INT_ID);
    if ((res & RLS_INT) == RLS_INT)
        inb(COM1 + COM_LINE_STATUS_REG);
    else if ((res & TRANSMIT_REG_EMPTY) == TRANSMIT_REG_EMPTY) {
        uart->hw_buf_status = HW_BUF_IDLE;
        consume_next(uart);
    }
}
