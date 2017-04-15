#include "ps2.h"

#define PS2_STATUS PS2_COMMAND
#define PS2_DEV_1 0
#define PS2_DEV_2 1

static uint8_t PS2_poll_read(void) {
    char status = inb(PS2_STATUS);

    while (!(status & PS2_STATUS_OUTPUT))
        status = inb(PS2_STATUS);

    return inb(PS2_DATA);
}

static void PS2_poll_write(uint8_t c) {
    char status = inb(PS2_STATUS);

    while (status & PS2_STATUS_INPUT)
        status = inb(PS2_STATUS);

    outb(PS2_DATA, c);
}

void PS2_flush_output_buffer() {

    /* Read from the buffer, weather or not it is ready, to clear buffer. */
    inb(PS2_DATA);
}

int PS2_self_test() {
}

extern int PS2_enable_device(int dev) {

    if (dev == PS2_DEV_1)
        PS2_poll_write(PS2_ENABLE_PORT_1);
    else if (dev == PS2_DEV_2)
        PS2_poll_write(PS2_ENABLE_PORT_2);
    else /* Invalid device was specified. */
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

extern int PS2_disable_device(int dev) {
    
    if (dev == PS2_DEV_1)
        PS2_poll_write(PS2_DISABLE_PORT_1);
    else if (dev == PS2_DEV_2)
        PS2_poll_write(PS2_DISABLE_PORT_1);
    else /* Invalid device was specified. */
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

extern int PS2_reset_device(int dev) {

    if (PS2_disable_device(dev) == EXIT_FAILURE)
        return EXIT_FAILURE;

    return PS2_enable_device(dev);
}

extern int PS2_init(void) {
    uint8_t config = 0;

    /* Disable first device. */
    if (PS2_disable_device(PS2_DEV_1) == EXIT_FAILURE)
        return EXIT_FAILURE;

    /* Disable second device. */
    if (PS2_disable_device(PS2_DEV_2) == EXIT_FAILURE)
        return EXIT_FAILURE;

    PS2_flush_output_buffer(); /* Flush output buffer. */

    /* Enable port 1 interrupt and clock. */
    config = PS2_PORT_1_INTERRUPT | PS2_PORT_1_CLOCK;
    PS2_poll_write(config); /* Write configuration to controller. */

    return EXIT_SUCCESS;
}

extern int PS2_write_data(uint8_t *data, size_t size) {
    int i;

    for (i = 0; i < size; i++)
        PS2_poll_write(data[i]);

    return EXIT_SUCCESS;
}

extern int PS2_read_data(uint8_t **data, size_t size) {
    int i;

    for (i = 0; i < size; i++)
        (*data)[i] = PS2_poll_read();

    return EXIT_SUCCESS;
}

extern int PS2_write(uint8_t c) {
    PS2_poll_write(c);
    return EXIT_SUCCESS;
}

extern uint8_t PS2_read() {
    return PS2_poll_read();
}
