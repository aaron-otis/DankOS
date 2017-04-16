#include <limits.h>
#include "drivers/vga.h"
#include "lib/string.h"
#include "lib/stdio.h"

#define LIGHTS_ON 0xFF

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
    unsigned long test = 123;
    int i;

    VGA_set_attr(VGA_BRIGHT_MAGENTA, VGA_BLACK, 0);

    /* Testing various parameters of printk. */
    printk("\n\nBeginning printk tests...\n");
    printk("Printing the character 'a': %c\nPrinting the test string: %s\n", 
     'a', str);
    printk("Printing the unsigned integer 123: %lu\n", test);
    printk("Testing literal unsigned integer 456: %u\n", 456);
    printk("Testing pointer cast to unsigned long int: %lu\n", (unsigned long) 
     &test);

    printk("Testing literal integer -789: %d\n", -789);
    printk("Testing literal integer 123 (0x7B) in hex: %x\n", 123);
    printk("Testing literal integer 0xDEADBEEF in hex: %x\n", 0xDEADBEEF);
    printk("Testing pointer converted to hex: %lx\n", (unsigned long) &test);
    for (i = 0; i < 33; i++)
        printk("%x ", i);
    printk("\n");

    printk("Testing pointer 0x123: %p\n", (void *) 0x123);
    printk("Testing pointer &test: %p\n", &test);
}

void ps2_tests() {
}

void keyboard_tests() {
}

void run_all_tests() {
    vga_driver_tests();
    stdio_tests();
    ps2_tests();
    keyboard_tests();
}
