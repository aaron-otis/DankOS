#ifndef _GDT_H
#define _GDT_H

#include "lib/stdint.h"

#define KERN_MODE 0
#define USER_MODE 3
#define CODE_SEG 1
#define DATA_SEG 0
#define LONG_MODE 1
#define OP_SIZE_LEGACY 0
#define PRESENT 1
#define CONFORMING 1
#define NON_CONFORMING 0

/* GDT indexes. */
#define KERN_CS_INDEX 1
#define KERN_DS_INDEX 2
#define TSS_INDEX 3
#define USER_CS_INDEX 5
#define USER_DS_INDEX 6

/* GDT offsets in bytes. */
#define KERN_CS_OFFSET 0x08
#define KERN_DS_OFFSET 0x10
#define TSS_OFFSET 0x18
#define USER_CS_OFFSET 0x28
#define USER_DS_OFFSET 0x30

#define GDT_SIZE 6
#define IDT_SIZE 256
#define TSS_TYPE 0x9
#define GRANULARITY 0
#define PAGE_SIZE 4096
#define INT_GATE 0xE

/* Macros for storing base addresses in descriptors. */
#define OFF_1_MASK 0xFFFF
#define OFF_2_MASK 0xFFFF
#define TSS_OFF_2_MASk 0xFF
#define TSS_OFF_3_SHIFT 48
#define OFF_2_SHIFT 16
#define OFF_3_SHIFT 32

#define NUM_IST_STACKS 5
#define IST_STACK_SIZE 256

struct segment_selector {
    uint16_t rpi:2; /* Privilege level (CPL). */
    uint16_t ti:1; /* Must be 0 to indicate the GDT is used. */
    uint16_t index:13;
};

typedef struct descriptor {
    uint16_t limit;
    uint16_t address1;
    uint8_t address2;
    uint8_t accessed:1; /* Ignored in 64 bit mode. */
    uint8_t read_write:1; /* Ignored in 64 bit mode. */
    uint8_t conform_expand:1; /* Expand ignored in 64 bit mode. */
    uint8_t code_data:1; /* 1 for CS, 0 for DS. */
    uint8_t one:1;
    uint8_t dpl:2;
    uint8_t present:1; /* The only field not ignored in DS. */
    uint8_t seg_limit:4;
    uint8_t avl:1;
    uint8_t long_mode:1;
    uint8_t op_size:1; /* In 64 bit mode, must be 0. */
    uint8_t granularity:1;
    uint8_t address3;
} __attribute__ ((packed)) segment_descriptor;

typedef struct {
    uint16_t offset1;
    uint16_t selector; /* Should be selector for kernel's code segment. */
    uint16_t ist:3;
    uint16_t reserved1:5;
    uint16_t type:4; /* Type of interrupt. Should be 0xE for interrupt gate. */
    uint16_t zero:1;
    uint16_t dpl:2; /* Protection level. */
    uint16_t present:1; /* Indicates a valid table entry. Should be 1. */
    uint16_t offset2;
    uint32_t offset3;
    uint32_t reserved2;
} __attribute__ ((packed)) ID; /* Interrupt descriptor. */

typedef struct {
    uint16_t limit1;
    uint16_t base1;
    uint16_t base2:8;
    uint16_t type:4;
    uint16_t zero:1;
    uint16_t dpl:2;
    uint16_t present:1;
    uint16_t limit2:4;
    uint16_t avl:1;
    uint16_t reserved1:2;
    uint16_t granularity:1;
    uint16_t base3:8;
    uint32_t base4;
    uint32_t reserved2:8;
    uint32_t legacy:5;
    uint32_t reserved3:19;
} __attribute__ ((packed)) TD; /* TSS descriptor. */

typedef struct TSS {
    uint32_t reserved1;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved2;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved3;
    uint16_t reserved4;
    uint16_t base;
} __attribute__ ((packed)) TSS; /* Actual TSS. */

extern ID IDT[IDT_SIZE]; /* Interrupt descriptor table. */
extern struct IDTR idtr;
extern void *gdt64; /* GDT from asm label. */
extern uint8_t IST_stacks[NUM_IST_STACKS][IST_STACK_SIZE];

#endif
