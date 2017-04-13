#include "ps2.h"
#include "io.h"

static char *data = (char *) PS2_DATA;
static char *command = (char *) PS2_COMMAND;

#define PS2_STATUS PS2_COMMAND
#define PS2_STATUS_OUTPUT 1

static char PS2_poll_read(void) {
    char status = inb(PS2_STATUS);

    while (!(status & PS2_STATUS_OUTPUT))
        status = inb(PS2_STATUS);

    return inb(PS2_DATA);
}

extern int PS2_init(void) {


    return EXIT_SUCCESS;
}
