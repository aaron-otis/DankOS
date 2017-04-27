#include "init.h"

extern int init() {
    int res;

    IRQ_init(); /* Initialize interrupts. */

    /* 
     * Initialize drivers. 
     */

    /* Initialize VGA driver. */
    if (VGA_init() == EXIT_FAILURE)
        return EXIT_FAILURE;

    /* Set VGA attributes so that printing can be seen. */
    VGA_set_attr(VGA_WHITE, VGA_BLACK, 0);

    printk("Initializing drivers... ");

    /* Initialize PIC driver. */
    PIC_init();
    printk("PIC ");

    /* Initialize interrupt driver. */
    IRQ_init();
    printk("IRQ ");

    /* Initialize PS2 driver. */
    res = PS2_init();
    if (res != EXIT_SUCCESS) {
        VGA_set_attr(VGA_WHITE, VGA_RED, 0);
        printk("PS2 driver initialization failure. Error %d\n", res);
        return EXIT_FAILURE;
    }
    printk("PS2 ");

    /* Initialize keyboard driver. */
    if ((res = KB_init()) != EXIT_SUCCESS) {
        VGA_set_attr(VGA_WHITE, VGA_RED, 0);

        printk("Keyboard initialization failure. ");
        printk("Error %d\n", res);
        return res;
    }
    printk("Keyboard ");

    printk("\ndone\n");
    return EXIT_SUCCESS;
}