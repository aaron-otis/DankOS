#include "kernel.h"
#include "kernel_tests.h"
#include "lib/debug.h"
#include "init.h"

static void halt_cpu() {

    for (;;) /* Infinite loop. */
        HALT_CPU
}

int kernel_main() {

    /* 
     * Initializations.
     */

    if (init() == EXIT_FAILURE)
        halt_cpu();

    /* 
     * Run kernel tests. 
     */

    //run_all_tests();

    /* Halt CPU at end for testing. */
    halt_cpu();

    return 0;
}
