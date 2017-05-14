#include <limits.h>
#include "drivers/vga.h"
#include "lib/string.h"
#include "lib/stdio.h"
#include "sys/memory.h"


void vga_driver_tests() {
    int i;
    char c;
    char *str = "String printing works!\n";
    char *long_str = "A test string that is longer than eighty characters is \
used to test if the driver will correctly scroll the screen.";

    /*
     * VGA driver tests. 
     */


    /* Set VGA character attributes. */
    VGA_set_attr(VGA_WHITE, VGA_BLACK, 0);

    /* Output characters to screen to test VGA driver. */
    VGA_display_char('I');
    VGA_display_char('t');
    VGA_display_char(' ');
    VGA_display_char('w');
    VGA_display_char('o');
    VGA_display_char('r');
    VGA_display_char('k');
    VGA_display_char('s');
    VGA_display_char('!');
    VGA_display_char('\n');

    /* Test VGA_display_str(). */
    VGA_display_str(str);

    /* Change text attributes. */
    VGA_set_attr(VGA_BRIGHT_GREEN, VGA_BLACK, 0);

    /* Test scrolling. */
    c = 'a';
    for (i = 0; i < 28; i++ ) {
        VGA_display_char(c++);
        VGA_display_char('\n');
    }
    VGA_display_str("Scrolling works!\n");

    /* Test printing a string longer than the screen width. */
    VGA_display_str(long_str);

    /* Test VGA clearing. */
    /*
    VGA_clear();
    */

    printk("\nCursor position: %d\n", VGA_get_cur_pos());
    printk("Buffer position: %d\n", VGA_get_buf_pos());
}

void stdio_tests() {
    char *str = "test string";
    short smin = SHRT_MIN, smax = SHRT_MAX;
    unsigned short usmin = SHRT_MIN, usmax = USHRT_MAX;
    int i, imin = INT_MIN, imax = INT_MAX;
    unsigned int uimin = INT8_MIN, uimax = UINT_MAX;
    long lmin = LONG_MIN, lmax = LONG_MAX;
    unsigned long ulmin = LONG_MIN, ulmax = ULONG_MAX, test = 123;

    VGA_set_attr(VGA_BRIGHT_MAGENTA, VGA_BLACK, 0);

    /* Testing various parameters of printk. */
    printk("\n\nBeginning printk tests...\n");

    /* Test printing characters and strings. */
    printk("Printing the character 'a': %c\nPrinting the test string: %s\n", 
     'a', str);

    /* Test printing shorts. */
    printk("Testing SHRT_MIN: %hd\nTesting SHRT_MAX: %hd\n", smin, smax);
    printk("Testing unsigned SHRT_MIN: %hu\n", usmin);
    printk("Testing unsigned USHRT_MAX: %hu\n\n", usmax);

    /* Test printing integerss. */
    printk("Testing INT_MIN: %d\nTesting INT_MAX: %d\n", imin, imax);
    printk("Testing unsigned INT_MIN: %u\n", imin);
    printk("Testing unsigned UINT_MAX: %u\n", imax);
    printk("Testing literal integer -789: %d\n", -789);

    /* Test hex printing. */
    printk("Testing literal integer 0xDEADBEEF in hex: %x\n", 0xDEADBEEF);
    printk("Testing pointer converted to hex: %lx\n", (unsigned long) &test);

    /* Test printing longs. */
    printk("Testing LONG_MIN: %lu\nTesting LONG_MIN: %lu\n", lmin, lmax);
    printk("Testing unsigned LONG_MIN: %lu\n", ulmin);
    printk("Testing unsigned ULONG_MAX: %lu\n", ulmax);
    printk("Testing literal integer -1: %u\n", -1);
    printk("Testing literal unsigned integer 456: %u\n", 456);

    /* Test printing pointers. */
    printk("Testing pointer 0x123: %p\n", (void *) 0x123);
    printk("Testing pointer &test: %p\n", &test);
}

void ps2_tests() {
}

void keyboard_tests() {
}

void page_fault_test() {
    int debug = 1;
    register intptr_t sp asm ("rsp");
    int *p = (int *) 0xFFFFFFFFFFFFFFFF;

    printk("\nKernel stack: %p\n", sp);
    //while (debug);
    *p = 0;

    printk("\nKernel stack: %p\n", sp);
}

void page_alloc_test() {
    int *page, i;

    for (i = 1; page = MMU_pf_alloc(); i++) {
        *page = i;
        printk("%u\n", *page);
    }
}

void run_all_tests() {
    //vga_driver_tests();
    stdio_tests();
    ps2_tests();
    keyboard_tests();
    page_fault_test();
    page_alloc_test();
}
