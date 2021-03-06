/**
 * @file
 */
#include "pic.h"
#include "io.h"
#include "interrupts.h"
#include "../gdt.h"
#include "../lib/stdint.h"
#include "../lib/stdlib.h"
#include "../lib/stdio.h"

/* Command and data ports for pic devices. */
#define PIC_MASTER_CMD 0x0020 /**< @brief Command port for master device. */
#define PIC_MASTER_DATA 0x0021 /**< @brief Data port for master device. */
#define PIC_SLAVE_CMD 0x00A0 /**< @brief Command port for slave device. */
#define PIC_SLAVE_DATA 0x00A1 /**< @brief Data port for slave device. */

#define ICW1_LTIM (1 << 3)
#define ICW1_INIT 0x10
#define ICW1_IC4 0x1
#define ICW4_8086_mode 0x1

/* IRQ numbers. */
#define IRQ_Z 0x0
#define IRQ_1 0x1
#define IRQ_2 (1 << 1)
#define IRQ_3 (1 << 2)
#define IRQ_4 (1 << 3)
#define IRQ_5 (1 << 4)
#define IRQ_6 (1 << 5)
#define IRQ_7 (1 << 6)

/* Cascade identities. */
#define CASCADE_Z 0x0
#define CASCADE_1 0x1
#define CASCADE_2 (1 << 1)

/* Commands. */
#define PIC_EOI 0x20 /**< @brief End of interrupt command code. */

/** @brief Sets the mask for an IRQ.
 *
 * Sets mask for specified IRQ to disable it.
 * @param IRQLine an IRQ number.
 * @post The specified IRQ is disabled it it is maskable.
 */
void PIC_set_mask(unsigned char IRQline) {
    uint16_t port;
    uint8_t value;

    if(IRQline < 8)
        port = PIC_MASTER_DATA;
    else {
        port = PIC_SLAVE_DATA;
        IRQline -= 8;
    }
    value = inb(port) | (1 << IRQline);
    outb(port, value);        
}

/** @brief Clears the mast of an IRQ.
 *
 * Clears the mask for the specified IRQ.
 * @param IRQline an IRQ number to clear the mask on.
 * @pre The mask for the specified IRQ is set.
 * @post The mask for the specified IRQ is cleared
 */
void PIC_clear_mask(unsigned char IRQline) {
    uint16_t port;
    uint8_t value;

    if(IRQline < PIC_NUM_IRQS) {
        port = PIC_MASTER_DATA;
    }
    else {
        port = PIC_SLAVE_DATA;
        IRQline -= PIC_NUM_IRQS;
    }
    value = inb(port) & ~(1 << IRQline);
    outb(port, value);        
}

/** @brief Remaps reserved IRQs to unreserved numbers.
 *
 * Remaps reserved IRQs from IBM's reserved numbers to non-reserved numbers.
 * @param moffset the offset for the master device.
 * @param soffset the offset for the slave device.
 * @post The reserved IRQs are remapped so that they may be received.
 */
void PIC_remap(int moffset, int soffset) {
    uint8_t master_mask, slave_mask;

    /* Save masks. */
    master_mask = inb(PIC_MASTER_DATA);
    slave_mask = inb(PIC_SLAVE_DATA);

    outb(PIC_MASTER_DATA, moffset); /* ICW2 Master PIC vector offset. */
    io_wait();
    outb(PIC_SLAVE_DATA, soffset);
    io_wait();
    outb(PIC_MASTER_DATA, IRQ_2); /* Tell master that slave is at IRQ 2. */
    io_wait();
    outb(PIC_SLAVE_DATA, CASCADE_2); /* Tell slave its PIC cascade identity. */
    io_wait();

    /* Set each controller to 8086 mode. */
    outb(PIC_MASTER_DATA, ICW4_8086_mode);
    io_wait();
    outb(PIC_SLAVE_DATA, ICW4_8086_mode);
    io_wait();

    /* Restore masks. */
    outb(PIC_MASTER_DATA, master_mask);
    outb(PIC_SLAVE_DATA, slave_mask);
}

/** @brief Initializes the PIC.
 * @post The PIC is initialized.
 */
int PIC_init(void) {
    int i;

    CLI;

    /* Start initialization sequence in cascade mode. */
    outb(PIC_MASTER_CMD, ICW1_INIT | ICW1_IC4);
    io_wait();
    outb(PIC_SLAVE_CMD, ICW1_INIT | ICW1_IC4);
    io_wait();

    PIC_remap(PIC_MASTER_OFFSET, PIC_SLAVE_OFFSET);

    for (i = 0; i < IDT_SIZE; i++ )
        PIC_set_mask(i);

    return EXIT_SUCCESS;
}

/** @brief Sends EOI signal.
 *
 * Sends an End Of Interrupt signal to the PIC.
 * @param irq an IRQ number to signal the PIC with.
 * @pre An interrupt with the specified IRQ has occurred.
 * @post The PIC hardware has been notified that the interrupt has been handled.
 */
void PIC_sendEOI(uint8_t irq) { /* Send EOI to PIC controllers. */
    irq -= PIC_MASTER_OFFSET;

    if (irq >= PIC_NUM_IRQS)
        outb(PIC_SLAVE_CMD, PIC_EOI);

    outb(PIC_MASTER_CMD, PIC_EOI);
}
