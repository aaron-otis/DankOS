/** 
 * @file
 */
#include "kernel.h"
#include "kernel_tests.h"
#include "lib/debug.h"
#include "lib/string.h"
#include "drivers/interrupts.h"
#include "sys/multiboot.h"
#include "sys/memory.h"
#include "sys/kmalloc.h"
#include "sys/proc.h"
#include "test/snakes.h"
#include "init.h"
#include "gdt.h"

/* Global variables. */
static TSS tss;

/** 
 * Infinite loop to halt CPU.
 *
 * @pre None.
 * @post The CPU is halted.
 */
static void halt_cpu() {

    for (;;) /* Infinite loop. */
        HALT_CPU
}

/** \brief Initialize TSS.
 *
 * Creates and populates a TSS descriptor and copies it into the correct portion 
 * of the GDT. Creates a TSS segment selector and loads it with the ltr 
 * instruction. Loads a full page for stacks into critical ISTs.
 *
 * \pre None.
 * \post The TSS is initialized.
 */
void tss_init() {
    uint64_t tss_addr = (uint64_t) &tss, tss_index = TSS_INDEX;
    segment_descriptor *gdt = (segment_descriptor *) &gdt64;
    struct segment_selector tss_sel;
    int int_enabled = 0;
    TD tss_desc;

    if (are_interrupts_enabled()) {
        int_enabled = 1;
        CLI;
    }

    memset(&tss, 0, sizeof(tss)); /* Zero TSS. */

    /* Set up TSS descriptor. */
    tss_desc.limit1 = sizeof(tss);
    tss_desc.base1 = tss_addr & OFF_1_MASK;
    tss_desc.base2 = (tss_addr >> OFF_2_SHIFT) & TSS_OFF_2_MASk;
    tss_desc.type = TSS_TYPE;
    tss_desc.dpl = KERN_MODE;
    tss_desc.present = PRESENT;
    tss_desc.base3 = (tss_addr >> TSS_OFF_3_SHIFT) & TSS_OFF_2_MASk;
    tss_desc.base4 = (tss_addr >> OFF_3_SHIFT);

    /* Copy TSS descriptor into the proper place in the GDT. */
    memcpy(&gdt[TSS_INDEX], &tss_desc, sizeof(tss_desc));

    /* Set up TSS selector. */
    tss_sel.rpi = 0;
    tss_sel.ti = 0;
    tss_sel.index = tss_index;
    __asm__("ltr %0": : "m"(tss_sel)); /* Load TSS selector. */

    /* Set up critical ISTs. */
    tss.ist1 = (uint64_t) MMU_pf_alloc();
    tss.ist2 = (uint64_t) MMU_pf_alloc();
    tss.ist3 = (uint64_t) MMU_pf_alloc();
    tss.ist4 = (uint64_t) MMU_pf_alloc();

    if (int_enabled)
        STI;
}

void read_keyboard() {
    while (1)
        printk("%c", getc());
    kexit();
}

/**
 * \brief The main kernel thread.
 *
 * Initializes all subsystems and manages resources.
 * \param mb_tag a pointer to the multiboot 2 header.
 * \returns 0.
 * @pre None.
 * @post None.
 */
int kernel_main(MB_basic_tag *mb_tag) {
    /* 
     * Initializations.
     */

    MMU_pf_init(mb_tag); /* Initialize page allocator. */

    if (init() == EXIT_FAILURE)
        halt_cpu();

    printk("\nD A N K O S\n\n>");

    PROC_create_kthread(read_keyboard, NULL);

    while (1) {
        PROC_run();
        HALT_CPU
    }

    return 0;
}
