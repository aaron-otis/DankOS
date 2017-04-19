#include "ps2.h"

#define PS2_STATUS PS2_COMMAND
#define PS2_DEV_1 0
#define PS2_DEV_2 1

static uint8_t PS2_poll_read(int port) {
    char status = inb(PS2_STATUS);

    while (!(status & PS2_STATUS_OUTPUT))
        status = inb(PS2_STATUS);

    return inb(port);
}

static void PS2_poll_write(int port, uint8_t c) {
    uint8_t status = inb(PS2_STATUS);

    while (status & PS2_STATUS_INPUT)
        status = inb(PS2_STATUS);

    outb(port, c);
}

static void PS2_flush_output_buffer() {

    /* Read from the buffer, weather or not it is ready, to clear buffer. */
    inb(PS2_DATA);
}

static int PS2_self_test() {

    PS2_poll_write(PS2_STATUS, PS2_TEST_CONTROLLER);
    return PS2_poll_read(PS2_DATA); /* Get response. */

}

static int PS2_port_test(int dev) {
    uint8_t cmd;

    if (dev == PS2_DEV_1)
        cmd = PS2_TEST_PORT_1;
    else if (dev == PS2_DEV_2)
        cmd = PS2_TEST_PORT_2;
    else
        return EXIT_FAILURE;

    PS2_poll_write(PS2_STATUS, cmd);
    return PS2_poll_read(PS2_DATA);
}

extern int PS2_enable_device(int dev) {

    if (dev == PS2_DEV_1)
        PS2_poll_write(PS2_STATUS, PS2_ENABLE_PORT_1);
    else if (dev == PS2_DEV_2)
        PS2_poll_write(PS2_STATUS, PS2_ENABLE_PORT_2);
    else /* Invalid device was specified. */
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

extern int PS2_disable_device(int dev) {
    
    if (dev == PS2_DEV_1)
        PS2_poll_write(PS2_STATUS, PS2_DISABLE_PORT_1);
    else if (dev == PS2_DEV_2)
        PS2_poll_write(PS2_STATUS, PS2_DISABLE_PORT_1);
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
    uint8_t config, res, dual_chan;

    /* Disable first device. */
    if (PS2_disable_device(PS2_DEV_1) == EXIT_FAILURE)
        return EXIT_FAILURE;

    /* Disable second device. */
    if (PS2_disable_device(PS2_DEV_2) == EXIT_FAILURE)
        return EXIT_FAILURE;

    PS2_flush_output_buffer(); /* Flush output buffer. */

    /* Request current configuration. */
    PS2_poll_write(PS2_STATUS, PS2_READ_ZBYTE);
    config = PS2_poll_read(PS2_DATA);  /* Read current configuration. */

    /* Check if single or dual channel. */
    dual_chan = config & PS2_PORT_2_CLOCK;

    /* Enable port 1 interrupt and clock. */
    config |= PS2_PORT_1_INTERRUPT | PS2_PORT_1_CLOCK;

    /* Write new configuration to controller. */
    PS2_poll_write(PS2_STATUS, config);

    res = PS2_self_test(); /* Perform controller self test. */
    if (res != PS2_SELF_TEST_PASS)
        return res;

    /* Test ports. */
    res = PS2_port_test(PS2_DEV_1); /* Test port 1. */
    if (res != PS2_PORT_TEST_PASS)
        return res;

    if (dual_chan) {
        if (PS2_enable_device(PS2_DEV_2) == EXIT_FAILURE) /* Enable port 2. */
            return EXIT_FAILURE;

        PS2_poll_write(PS2_STATUS, PS2_READ_ZBYTE); /* Get configuration. */
        dual_chan = PS2_poll_read(PS2_DATA) | PS2_PORT_2_CLOCK;

        if (!dual_chan) { /* Port 2 exists. */
            res = PS2_port_test(PS2_DEV_2); /* Run test on port 2. */

            if (res != PS2_PORT_TEST_PASS)
                return res;

            res = PS2_disable_device(PS2_DEV_2);
            if (res != EXIT_SUCCESS)
                return res;
        }
    }

    /* Enable devices. */
    res = PS2_enable_device(PS2_DEV_1);
    if (res != EXIT_SUCCESS)
        return res;

    if (!dual_chan) { /* Enable only if device 2 exists. */
        res = PS2_enable_device(PS2_DEV_2);
        if (res != EXIT_SUCCESS)
            return res;
    }



    return EXIT_SUCCESS;
}

extern int PS2_write_data(uint8_t *data, size_t size) {
    int i;

    for (i = 0; i < size; i++)
        PS2_poll_write(PS2_DATA, data[i]);

    return EXIT_SUCCESS;
}

extern int PS2_read_data(uint8_t **data, size_t size) {
    int i;

    for (i = 0; i < size; i++)
        (*data)[i] = PS2_poll_read(PS2_DATA);

    return EXIT_SUCCESS;
}

extern int PS2_write(uint8_t c) {
    PS2_poll_write(PS2_DATA, c);
    return EXIT_SUCCESS;
}

extern uint8_t PS2_read() {
    return PS2_poll_read(PS2_DATA);
}

extern int PS2_send_command(uint8_t c) {
    PS2_poll_write(PS2_STATUS, c);
    return EXIT_SUCCESS;
}
