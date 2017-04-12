#include "kernel.h"
#include "kernel_tests.h"
#include "lib/debug.h"

void sleep() {
    long i = 0;

    while (i >= 0)
        i++;
}

int kernel_main() {

    /* Run kernel tests. */

    /* VGA driver tests. */
    vga_driver_tests();

    /* stdio tests. */
    stdio_tests();

    for (;;) /* Infinite loop. */
        HALT_CPU
}
