#ifndef _KERNEL_TESTS_H
#define _KERNEL_TESTS_H

void run_all_tests();
void vga_driver_tests();
void stdio_tests();
void ps2_tests();
void keyboard_tests();
void page_fault_test();
void page_frame_alloc_test();
void page_alloc_test();
void virutal_addr_tests();
void kmalloc_test();

#endif
