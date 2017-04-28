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

#define KERN_CS_INDEX 1
#define KERN_DS_INDEX 2
#define KERN_CS_OFFSET 0x08
#define KERN_DS_OFFSET 0x10

#define GDT_SIZE 16
#define IDT_SIZE 256
#define TSS_TYPE 0x9
#define GRANULARITY 0
#define PAGE_SIZE 4096

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
    uint16_t limit;
    uint16_t base1;
    uint16_t base2:8;
    uint16_t type:4;
    uint16_t zero:1;
    uint16_t dpl:2;
    uint16_t present:1;
    uint16_t segment_limit:4;
    uint16_t avl:1;
    uint16_t reserved1:2;
    uint16_t granularity:1;
    uint16_t base3:8;
    uint32_t reserved2;
    uint32_t reserved3:8;
    uint32_t legacy:5;
    uint32_t reserved4:19;
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
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved3;
    uint16_t reserved4;
    uint16_t base;
} __attribute__ ((packed)) TSS;


extern ID IDT[IDT_SIZE]; /* Interrupt descriptor table. */
extern struct IDTR idtr;
extern segment_descriptor GDT[];

#endif
