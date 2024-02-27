#ifndef PMALLOC
#define PMALLOC

#include <stdint.h>
#include <stddef.h>

typedef struct pmalloc_item {
    struct pmalloc_item __far *prev;    // The previous block in the chain
    struct pmalloc_item __far *next;    // The next block in the chain
    uint32_t size;                      // This is the size of the block as reported to the user 
} pmalloc_item_t;

typedef pmalloc_item_t __far * fp_pmalloc_item_t;

typedef struct pmalloc {
    fp_pmalloc_item_t available;        // The linked list of available blocks
    fp_pmalloc_item_t assigned;         // The linked list of allocated blocks
    uint32_t freemem;                   // The current free memory count
    uint32_t totalmem;                  // The total available free memory
    uint32_t totalnodes;                // The number of nodes in the allocated list
} pmalloc_t;

extern pmalloc_t __far *pm;

void pmalloc_init(pmalloc_t __far *pm);
void pmalloc_addblock(pmalloc_t __far *pm, void __far *ptr, uint32_t size);           // Add an area of memory available for allocation
void __far *pmalloc_malloc(pmalloc_t __far *pm, uint32_t size);                       // Allocate size bytes of memory, returns NULL if out of memory
void pmalloc_free(pmalloc_t __far *pm, void __far *ptr);                              // Deallocate a block of previously allocated memory

uint32_t pmalloc_sizeof(pmalloc_t __far *pm, void __far *ptr);                        // Return the size of a block of previously allocated memory
uint32_t pmalloc_freemem(pmalloc_t __far *pm);                                        // Return the amount of free memory 
uint32_t pmalloc_totalmem(pmalloc_t __far *pm);                                       // Return the total amount of memory
uint32_t pmalloc_usedmem(pmalloc_t __far *pm);                                        // Return the amount of used memory
uint32_t pmalloc_overheadmem(pmalloc_t __far *pm);                                    // Return the current memory overhead

// Internals
void pmalloc_merge(pmalloc_t __far *pm, fp_pmalloc_item_t node);                        // Merge free blocks around this block
void pmalloc_item_insert(fp_pmalloc_item_t __far *root, void __far *ptr);               // Insert an item into the linked list
void pmalloc_item_remove(fp_pmalloc_item_t __far *root, pmalloc_item_t __far *node);    // Remove an item from a linked list
void pmalloc_dump_stats(pmalloc_t __far *pm);                                           // Debug Function


#endif

