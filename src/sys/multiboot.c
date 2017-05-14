/**
 * @file
 */
#include "multiboot.h"
#include "../lib/stdio.h"
#include "../lib/stdlib.h"
#include <limits.h>

#define HIGH_ADDR_START 0x9fc00

/** @brief Parses multiboot tags.
 *
 * Parses multiboot 2 tags. Finds relevant information for page frame allocator.
 * @param *mb_tag a MB_basic_tag pointer.
 * @returns MB_mem_info struct.
 * @pre Correct pointer to multiboot 2 headers is found.
 * @post Multiboot 2 tags parsed and necessary information for page frame 
 * allocator about memory returned.
 */
MB_mem_info MB_parse_tags(MB_basic_tag *mb_tag) {
    int i, tag_size;
    uint8_t *ptr = (uint8_t *) mb_tag;
    uint8_t *mb_tag_end, *mmap_tag_end, *elf_tag_end;
    MB_fixed_tag_header *fixed_header;
    MB_mmap_tag *mmap;
    MB_ELF_symb_tag *elf;
    MB_mmap_entry *mmap_entry;
    MB_ELF_section_header *elf_section;
    MB_mem_info mem_info;

    mb_tag_end = (uint8_t *) (ptr + mb_tag->size);
    ptr += sizeof(MB_basic_tag);

    /* Parse all multiboot 2 tags. */
    while (ptr < mb_tag_end) {
        fixed_header = (MB_fixed_tag_header *) ptr;

        tag_size = fixed_header->size; /* Determine tag size. */
        if (tag_size % CHAR_BIT) /* Ensure size is 8 byte aligned. */
            tag_size += CHAR_BIT - (tag_size % CHAR_BIT);

        /* Check for relevant tags (mmap and elf). */
        if (fixed_header->type == MULTI_ELF) {
            elf = (MB_ELF_symb_tag *) (ptr + sizeof(MB_basic_tag));
            elf_tag_end = ptr + tag_size;
        }
        else if (fixed_header->type == MULTI_MMAP) {
            mmap = (MB_mmap_tag *) (ptr + sizeof(MB_basic_tag));
            mmap_tag_end = ptr + tag_size;
        }

        ptr += tag_size; /* Advance to the next tag. */
    }


    /* Set up pointer to first ELF section header. */
    ptr = (uint8_t *) elf;
    ptr += sizeof(MB_ELF_symb_tag);
    elf_section = (MB_ELF_section_header *) ptr;

    for (i = 0; elf_section < (MB_ELF_section_header *) elf_tag_end && 
     i < elf->num; elf_section++, i++) {

        if (i == 1)
            mem_info.kern_start = elf_section->address;
        if (i == elf->num - 1)
            mem_info.kern_size = (elf_section->address + elf_section->size) - 
             mem_info.kern_start;
    }

    /* Set up pointer to first mmap entry. */
    ptr = (uint8_t *) mmap;
    ptr += sizeof(MB_mmap_tag);
    mmap_entry = (MB_mmap_entry *) ptr;

    for (i = 0;mmap_entry <  (MB_mmap_entry *) mmap_tag_end; mmap_entry++) {

        if (mmap_entry->type == MULTI_MEM_USABLE) {
            if (!i) {
                mem_info.low.address = mmap_entry->base_addr;
                mem_info.low.size = mmap_entry->length;
                i++;
            }
            else if (i == 1) {
                mem_info.high.address = mmap_entry->base_addr;
                mem_info.high.size = mmap_entry->length;
            }
        }
    }

    return mem_info;
}
