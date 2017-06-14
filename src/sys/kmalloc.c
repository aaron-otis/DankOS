/** 
 * @file
 */

#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../lib/stdlib.h"
#include "../lib/debug.h"
#include "kmalloc.h"
#include "memory.h"

#define BUF_LEN 100
#define BIG_ENOUGH 1
#define PERFECT_SIZE 2
#define DEFAULT_BLOCK_SIZE 0xFA00

#define ALIGNMENT_CONST 16
#define ALIGNED_BLOCK (sizeof(Block) + ALIGNMENT_CONST - sizeof(Block) % \
    ALIGNMENT_CONST)

#define DEBUG_ENV "DEBUG_MALLOC"
#define DEBUG_MALLOC "MALLOC: malloc(%d)\t=> (ptr=%p, size=%d)\n"
#define DEBUG_FREE "MALLOC: free(%p)\n"
#define DEBUG_CALLOC "MALLOC: calloc(%d,%d)\t=> (ptr=%p, size=%d)\n"
#define DEBUG_REALLOC "MALLOC: realloc(%p,%d)\t=> (ptr=%p, size=%d)\n"

static void *head = NULL;
static char debug = 0;

/* Struct for storing metadata about blocks */
typedef struct Block {
    size_t size;
    struct Block *next;
    char free;
} Block;

/* find_header() finds the associated allocation unit for |ptr|. If |prev| is
 * nonnull, find_header() will set it to the previous allocation unit.
 */
Block *find_header(void *ptr, Block **prev) {
    Block *header = (Block *) head;

    /* Find the allocation unit that |ptr| falls in */
    while (header->next && header->next <= (Block *) ptr) {
        if (prev)
            *prev = header; /* Store the previous allocation unit */
        header = header->next; /* Advance to next allocation unit */
    }

    return header;
}

/* align_size() ensures that any value passed to it will be divisible by 16 */
size_t align_size(size_t size) {

    if (size % ALIGNMENT_CONST)
        size += ALIGNMENT_CONST - size % ALIGNMENT_CONST;

    return size;
}

/* malloc() will first check to see if there already exits a free block of
 * appropriate size, and will segment that free block if necessary to not waste
 * space, and will return a pointer to the unused segment. If no suitable block
 * exists, kbrk() will be called to allocate more space.
 */

void *kmalloc(size_t size) {
    void *brk, *ret;
    Block *header, *temp;
    short found = 0;
    size_t oldsize, newsize;

    /* Check if |size| is zero and return NULL if so */
    if (size == 0) {
        if (debug) {
            printk(DEBUG_MALLOC, size, NULL, 0);
        }

        return NULL;
    }

    /* Round up to a multiple of 16 if not already a multiple of 16 */
    size = align_size(size);

    header = (Block *) head; /* Make the head of the linked list accessible */
    while (header && !found) {
        /* Check if the block is big enough */
        if (header->free && header->size > size + ALIGNED_BLOCK) {
            found = BIG_ENOUGH; /* Found a block bigger than what is needed */
        }
        else if (header->free && header->size == size) {
            found = PERFECT_SIZE; /* Found a block exactly the right size */
        }

        /* Advance to the next item in the list if usable block is not found */
        if (!found)
            header = header->next;
    }

    if (header) { /* An appropriately sized block was found */

        if (found == BIG_ENOUGH) {
            header->free = 0; /* Mark block as not free */
            oldsize = header->size; /* Store old size */
            header->size = size; /* Set correct size */

            /* Create a space for the next header */
            temp = (Block *) (((char *) header) + size + ALIGNED_BLOCK);
            temp->free = 1; /* Mark as free */
            /* Set size as remaining size of the block */

            temp->size = oldsize - size - ALIGNED_BLOCK; /* Set size */
            /* Adjust the |next| pointers appropriately */
            temp->next = header->next;
            header->next = temp;

            ret = (void *) ((char *) header + ALIGNED_BLOCK);
        }
        else if (found == PERFECT_SIZE) {
            header->free = 0; /* Mark block as not free */
            ret = (void *) ((char *) header + ALIGNED_BLOCK);
        }
        else {
            printk("Nonnull header when block not found");
            ret = NULL;
        }
    }
    else { /* An appropriate block was not found */

        /* Find the last block in the list */
        temp = head;
        while (temp && temp->next)
            temp = temp->next;

        /* Check to see if |size| is larger than DEFAULT_BLOCK_SIZE and request
         * an appropriately sized block if so
         */
        if (size > DEFAULT_BLOCK_SIZE)
            newsize = (size / DEFAULT_BLOCK_SIZE + 1) * DEFAULT_BLOCK_SIZE;
        else /* Otherwise request the default sized block */
            newsize = DEFAULT_BLOCK_SIZE;


        brk = kbrk(newsize); /* Create a new free block */
        if (brk == (void *) -1) {
            printk("kbrk() error");
            return NULL;
        }

        /* If the head of the linked list is NULL, set it equal to |brk|.
         * This should only happen once the first time malloc() is called.
         */
        if (!head) {
            head = brk;

            if (debug) {
                printk("Setting |head| at %p\n", brk);
            }
        }

        header = (Block *) brk; /* Set header for new allocation unit */
        header->free = 0;
        header->size = size; 
        /* Set the address for the next allocation unit */
        header->next = (Block *) ((char *) brk + size + ALIGNED_BLOCK);
        if (temp) /* Make sure the previous block can access this block */
            temp->next = header; 

        temp = header->next; /* Set |temp| to the last allocation unit */
        temp->free = 1;
        temp->size = newsize - (size + ALIGNED_BLOCK * 2);
        temp->next = NULL; /* No allocations units exist after this one */


        ret = (void *) ((char *) header + ALIGNED_BLOCK);
    }

    /* Check to see if the debugging environmental variable is set */
    if (debug) {
        printk(DEBUG_MALLOC, size, ret, size);
    }

    return ret;
}

/* free() deallocates a block and merges it with adjacent free blocks if they 
 * exist.
 */

void free(void *ptr) {
    Block *header, *temp = NULL;

    /* Check to see if |ptr| is NULL and silently return if so */
    if (!ptr)
        return;

    /* |head| should not be NULL. Throw an error and exit if it is. */
    if (!head) {
        printk("Linked list head is NULL");
        HALT_CPU
    }

    /* Search for the header that is at or just before |ptr| */
    header = find_header(ptr, &temp);
    header->free = 1; /* Set the block containing the address in |ptr| free */

    /* Merging free blocks */

    /* Check if the next allocation unit is free */
    if (header->next && header->next->free) {
        /* Add the size of the next block to the current block size */
        header->size += header->next->size + ALIGNED_BLOCK;
        header->next = header->next->next; /* Point to the new next block */
    }

    /* Check if the previous allocation unit is free */
    if (temp && temp->free) {
        temp->size += header->size + ALIGNED_BLOCK;
        temp->next = header->next;
        header = temp;
    }

    /* If the newly freed space is the last allocation unit, and not the 
     * head of the list, give the space to the OS */
    if (!header->next && header != head) {
        /* Call find header to find the previous block */
        header = find_header(header, &temp);

        /* Adjust size with kbrk() and check for errors */
        if (kbrk(-(header->size + ALIGNED_BLOCK)) == (void *) -1) {
            printk("kbrk() error");
        }
        /* The previous block is now the last block and must point to NULL */
        if (temp) {
            temp->next = NULL;
        }
    }

    if (debug) {
        printk(DEBUG_FREE, ptr);
    }
}

/* calloc() allocates |nmemb| blocks of size |size| which are then zeroed
 * and returns a pointer to the beginning of the first block.
 */

void *calloc(size_t nmemb, size_t size) {
    size_t total_size = nmemb * size;
    void *ret = kmalloc(total_size);

    /* Check if |nmemb| or |size| are zero and return NULL if so */
    if (!total_size)
        ret = NULL;

    /* Use memset() to fill the block with zeros and set |ret| to NULL if an
     * error occurs
     */
    if (ret) {
        if (ret != memset(ret, 0, total_size)) {
            printk("memset() error");
            ret = NULL;
        }
    }

    /* Check to see if the debugging environmental variable is set */
    if (debug) {
        /* malloc() will align |size| but it must be done manually here
         * for the debugging output
         */
        total_size = align_size(total_size);
        printk(DEBUG_CALLOC, nmemb, size, ret, total_size);
    }

    return ret;
}

void *realloc(void *ptr, size_t size) {
    Block *header, *temp, new;
    void *ret;

    /* When |ptr| is NULL, function like malloc */
    if (!ptr)
        return kmalloc(size);

    /* When |ptr| is not NULL and |size| is zero, function like free() and
     * return NULL
     */
    if (!size) {
        free(ptr);
        return NULL;
    }

    /* Find the allocation unit that contains |ptr| */
    header = find_header(ptr, NULL);
    size = align_size(size); /* Make sure |size| is aligned */

    /* If a larger size is given, then that allocation unit needs to be
     * expanded if the next block is free and large enough to hold the new
     * size, or a new block will be requested from malloc().
     */
    if (size > header->size) {

        /* Check if the next block is the perfect size */
        if (header->next && header->next->free && header->next->size == size - 
         header->size - ALIGNED_BLOCK) {
            /* Adjust the pointer to the next block to skip over the block 
             * that will be assimilated into this one
             */
            header->next = header->next->next;
            header->size = size; /* Adjust the size of this block */
            ret = ptr;
        }
        /* Check if the next block is larger than needed */
        else if (header->next && header->next->free && header->next->size > 
         size - header->size) {
            /* Set up the new allocation unit using |new| to prevent possibly 
             * overwriting metadata before reading it.
             */
            temp = header->next;
            new.next = temp->next;
            new.size = temp->size - size + header->size;

            /* New start of the next allocation unit after shrinking */
            temp = (void *) ((char *) header + size + ALIGNED_BLOCK);
            temp->next = new.next;
            temp->size = new.size;
            temp->free = 1;

            /* Update current allocation unit */
            header->size = size;
            header->next = temp;
            ret = ptr;
        }
        /* Otherwise allocate new space */
        else {
            ret = kmalloc(size);
            if (ret) {
                /* Copy old data to new space */
                memcpy(ret, (char *) header + ALIGNED_BLOCK, header->size);
                free(ptr);
            }
        }
    }
    else { /* Smaller size equates to shrinking the block if possible */

        if (size > 0 && size + ALIGNED_BLOCK <= header->size) {
            /* Create a new block after the current one */
            temp = (Block *) ((char *) header + size + ALIGNED_BLOCK);
            temp->next = header->next;
            temp->free = 1;
            temp->size = header->size - size - ALIGNED_BLOCK;

            header->size -= size + ALIGNED_BLOCK; /* Shrink size */
            header->next = temp; /* Make sure new next block is accessible */
        }
        ret = ptr;
    }

    /* Check to see if the debugging environmental variable is set */
    if (debug) {
        printk(DEBUG_REALLOC, ptr, size, ret, size);
    }

    return ret;
}
