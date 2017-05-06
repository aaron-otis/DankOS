#ifndef _MULITIBOOT_H
#define _MULITIBOOT_H

#include "../lib/stdint.h"
#include "memory.h"

#define MULTI_MAGIC 0xE85250D6
#define MULTI_ARCH 0
#define MULTI_MMAP 6
#define MULTI_ELF 9
#define MULTI_MEM_USABLE 1

typedef struct {
    uint32_t magic;
    uint32_t arch;
    uint32_t length; /* Length of of entire header. */
    uint32_t checksum; /* Results in 0 when added to other fields. */
    void *tags;
} __attribute__((packed)) MB_header;

typedef struct {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
} __attribute__((packed)) MB_basic_header_tag;

typedef struct {
    uint32_t size; /* Total size (bytes) including terminating tag. */
    uint32_t reserved;
} __attribute__((packed)) MB_basic_tag;

typedef struct {
    uint32_t type;
    uint32_t size;
} __attribute__((packed)) MB_fixed_tag_header;

typedef struct {
    uint32_t upper;
    uint32_t lower;
} __attribute__((packed)) MB_mem_info_tag;

typedef struct {
    uint32_t biosdev;
    uint32_t partition;
    uint32_t subparition;
} __attribute__((packed)) MB_boot_dev_tab;

typedef struct {
    uint8_t string;
} __attribute__((packed)) MB_cmd_tag;

typedef struct {
    uint32_t num;
    uint32_t entsize;
    uint32_t shndx;
} __attribute__((packed)) MB_ELF_symb_tag;

typedef struct {
    uint32_t name;
    uint32_t type;
    uint64_t flags;
    uint64_t address;
    uint64_t offset;
    uint64_t size; /* In bytes. */
    uint32_t table_index_link;
    uint32_t extra_info;
    uint64_t addr_alignment; /* In powers of 2. */
    uint64_t IFF;
} __attribute__((packed)) MB_ELF_section_header;

typedef struct {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed)) MB_mmap_entry;

typedef struct {
    uint32_t entry_size;
    uint32_t entry_version;
} __attribute__((packed)) MB_mmap_tag;

typedef struct {
    uint64_t address;
    uint64_t size;
} MB_mem_block;

typedef struct {
    MB_mem_block low;
    MB_mem_block high;
    long kern_start;
    long kern_size;
} MB_mem_info;

MB_mem_info MB_parse_tags(MB_basic_tag *);

#endif
