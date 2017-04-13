#include "kernel.h"
#include "kernel_tests.h"
#include "lib/debug.h"
#include "drivers/vga.h"
#include "drivers/ps2.h"
#include "lib/stdio.h"

static void halt_cpu() {

    for (;;) /* Infinite loop. */
        HALT_CPU
}

int kernel_main() {

    /* 
     * Initializations.
     */

    /* Initialize VGA driver. */
    if (VGA_init() == EXIT_FAILURE)
        halt_cpu();

    /* Set VGA attributes so that printing from the kernel can be seen. */
    VGA_set_attr(VGA_WHITE, VGA_BLACK, 0);

    /* Initialize PS2 driver. */
    if (PS2_init() == EXIT_FAILURE) {
        printk("PS2 driver initialization failure\n");
        halt_cpu();
    }

    /* 
     * Run kernel tests. 
     */

    /* VGA driver tests. */
    vga_driver_tests();

    /* stdio tests. */
    stdio_tests();

    /* PS2 tests. */
    ps2_tests();

    /* Halt cpu at end for testing. */
    halt_cpu();

    return 0;
}
