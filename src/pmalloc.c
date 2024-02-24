//
// pmalloc - A Heap based memory manager
//
// Author: Tom Cully <mail@tomcully.com>
// Date: 10th Dec 2022
//
// pmalloc can be used to manage memory using a familiar malloc/free pattern in embedded 
// systems, or to manage sub-allocation within an area of memory given by the OS.
//
// To get started, call pmalloc_addblock with the address and size of the memory to be used for 
// allocation.
//

#include "pmalloc.h"

#ifdef DEBUG
	#include <stdio.h>
#endif

void pmalloc_init(pmalloc_t *pm) {
	#ifdef DEBUG
		printf("pmalloc: DEBUG Enabled\r");
	#endif

	pm->available = NULL;
	pm->assigned = NULL;
	pm->freemem = 0;
	pm->totalmem = 0;
	pm->totalnodes = 0;
}

void pmalloc_addblock(pmalloc_t *pm, void __far *ptr, uint32_t size)
{
	// Get the usable size of the block
	((pmalloc_item_t __far *)ptr)->size = size - sizeof(pmalloc_item_t);

	// Update freemem and totalmem
	pm->freemem += ((pmalloc_item_t __far *)ptr)->size;
	pm->totalmem += pm->freemem;

	// Add it to the available heap, update totalnodes
	pmalloc_item_insert(&pm->available, ptr);
	pm->totalnodes++;
}

void __far *pmalloc_malloc(pmalloc_t *pm, uint32_t size)
{
	// Find a suitable block
	pmalloc_item_t __far *current = pm->available;
	while(current != NULL && current->size < size) current = current->next;

	// If there's nothing suitable, we're either out of memory or fragged.
	if(current == NULL) return NULL;

	// Remove it from pm->available
	pmalloc_item_remove(&pm->available, current);

	// Add to pm->assigned
	pmalloc_item_insert(&pm->assigned, current);

	// If it's not the exact size..
	if(current->size != size) {
		// Add a free block that's the remainder size
		pmalloc_item_t __far *newfree = (pmalloc_item_t __far *)((char __far *)current + sizeof(pmalloc_item_t) + size);
		newfree->size = current->size - sizeof(pmalloc_item_t) - size;
		newfree->next = current->next;
		current->next = NULL;
		
		// Change pm->assigned size
		current->size = size;
		pmalloc_item_insert(&pm->available, newfree);

		// We've lost a bit of overhead making the new node
		pm->freemem -= sizeof(pmalloc_item_t);
		pm->totalnodes++;

		// Merge around newfree
		pmalloc_merge(pm, newfree);
	}

	// Reduce the amount of free memory
	pm->freemem -= current->size;

	// Return the user memory
	return ((char __far *)current) + sizeof(pmalloc_item_t);
}

void pmalloc_free(pmalloc_t *pm, void __far *ptr)
{
	// Match stdlib free() NULL interface
	if(ptr == NULL) return;

	// Get the node of this memory
	pmalloc_item_t __far *node = (pmalloc_item_t __far *)(((char __far *)ptr) - sizeof(pmalloc_item_t));
	#ifdef DEBUG
	printf("ptr  = %p\r", ptr);
	printf("node = %p\r", node);
	#endif

	// Remove it from pm->assigned
	pmalloc_item_remove(&pm->assigned, node);

	pm->freemem += node->size;

	// Add to pm->available
	pmalloc_item_insert(&pm->available, node);

	// Merge around current
	pmalloc_merge(pm, node);
}

void pmalloc_merge(pmalloc_t *pm, pmalloc_item_t __far *node) {
	// Scan backward for contiguous blocks
	while (node->prev != NULL && (char __far *)node == (char __far *)node->prev + sizeof(pmalloc_item_t) + node->prev->size)
		node = node->prev;

	// Scan forward and merge free blocks
	while (node->next == (pmalloc_item_t __far *)((char __far *)node + sizeof(pmalloc_item_t) + node->size)) {
		uint32_t nodesize = node->next->size + sizeof(pmalloc_item_t);
		pm->freemem += sizeof(pmalloc_item_t);
		pmalloc_item_remove(&pm->available, node->next);
		pm->totalnodes--;
		node->size += nodesize;
	}
}

uint32_t pmalloc_sizeof(pmalloc_t *pm, void __far *ptr) {
	// Get the actual pmalloc_item_t of the block
	pmalloc_item_t __far *node = (pmalloc_item_t __far *)(ptr - sizeof(pmalloc_item_t));

	// Return its size
	return node->size;
}

uint32_t pmalloc_totalmem(pmalloc_t *pm) { return pm->totalmem; }
uint32_t pmalloc_freemem(pmalloc_t *pm) { return pm->freemem; }
uint32_t pmalloc_usedmem(pmalloc_t *pm) { return pm->totalmem - pm->freemem; }
uint32_t pmalloc_overheadmem(pmalloc_t *pm) { return pm->totalnodes * sizeof(pmalloc_item_t); }

void pmalloc_item_insert(fp_pmalloc_item_t *root, void __far *ptr)
{
	// No existing root
	if(*root == NULL) {
		*root = (pmalloc_item_t __far *)ptr;
		(*root)->prev = NULL;
		(*root)->next = NULL;
		return;
	}

	// Where is the block in relation to the root?
	if(ptr < (void*)*root) {
		// New block before root
		pmalloc_item_t __far *node = (pmalloc_item_t __far *)ptr;
		pmalloc_item_t *oldroot = *root;
		oldroot->prev = node;
		node->next = oldroot;
		*root = node;
	} else {
		// New block within or at end of list
		pmalloc_item_t __far *current = *root;
		pmalloc_item_t __far *node = (pmalloc_item_t __far *)ptr;
		
		// Skip until we find the right place to insert, or the end of the list
		while(current->next != NULL && node > current->next) current = current->next;

		// We're inserting the block at...
		if(current->next == NULL) {
			// The end of list
			node->prev = current;
			current->next = node;
		} else {
			// Somewhere in the middle
			pmalloc_item_t __far *oldnext = current->next;

			current->next = node;
			node->prev = current;
			node->next = oldnext;
			oldnext->prev = node;
		}
	} 
}

void pmalloc_item_remove(fp_pmalloc_item_t *root, pmalloc_item_t __far *node) 
{
	// Remove the node
	if(node->prev) node->prev->next = node->next;
	if(node->next) node->next->prev = node->prev;
	
	// Fixup root if the node was root
	if(node==*root) {
		if (node->prev) 
			*root = node->prev; 
		else 
			*root = node->next; 
	}

	// Clear the next and previous pointers
	node->next = NULL;
	node->prev = NULL;
}

#ifdef DEBUG
void pmalloc_dump_stats(pmalloc_t *pm) {
	printf("---------------------\r");
	printf(" - freemem: %lu\r", pm->freemem);
	printf(" - totalmem: %lu\r", pm->totalmem);
	printf(" - totalnodes: %lu (sizeof %d)\r", pm->totalnodes, (int)sizeof(pmalloc_item_t));
	printf(" - assigned:\r");
	for(pmalloc_item_t __far * current = pm->assigned; current != NULL; current=current->next) {
		printf("  - (%08llx) %08llx- %08llx-size:%lld (%u sys, %lu usr)\r", (unsigned long long)(char __far *)current, (unsigned long long)(char __far *)current + sizeof(pmalloc_item_t), (unsigned long long)(char __far *)current + current->size + sizeof(pmalloc_item_t), (unsigned long long)(current->size + sizeof(pmalloc_item_t)), sizeof(pmalloc_item_t), current->size);
	} 
	printf(" - available:\r");
	for(pmalloc_item_t __far * current = pm->available; current != NULL; current=current->next) {
		printf("  - (%08llx) %08llx->%08llx-size:%lld (%u sys, %lu usr)\r", (unsigned long long)(char __far *)current, (unsigned long long)(char __far *)current + sizeof(pmalloc_item_t), (unsigned long long)(char __far *)current + current->size + sizeof(pmalloc_item_t), (unsigned long long)(current->size + sizeof(pmalloc_item_t)), sizeof(pmalloc_item_t), current->size);
	} 

	printf("---------------------\r");
}
#endif

/*
void __far *pmalloc_calloc(pmalloc_t *pm, uint32_t num, uint32_t size)
{
	char *mem = pmalloc_malloc(pm, num * size);
	if(mem==NULL) return NULL;
	for(uint32_t i=0; i<size; i++) mem[i]=0;
	return mem;
}
*/

/*
void __far *pmalloc_realloc(pmalloc_t *pm, void __far *ptr, uint32_t requestedSize)
{
    // Match stdlib realloc() NULL interface
    if (ptr == NULL) return pmalloc_malloc(pm, requestedSize);

    // Get the actual pmalloc_item_t of the block
	pmalloc_item_t *node = (pmalloc_item_t __far *)(ptr - sizeof(pmalloc_item_t));
    
    // If the requested size is equal to the current size, return the original pointer
    if (node->size == requestedSize) return ptr;

    // If the requested size is smaller:
    if (requestedSize < node->size) {
     	// If the difference is less than twice sizeof(pmalloc_item_t), it's not worth doing anything, return the original pointer
     	if(node->size - requestedSize < sizeof(pmalloc_item_t)*2) return ptr;

     	// Otherwise, create a free block for the extra space, truncate the block at the new size, and merge around it
     	pmalloc_item_t *newFree = (pmalloc_item_t __far *)((char*)node + sizeof(pmalloc_item_t) + requestedSize);
     	newFree->size = (node->size - requestedSize) - sizeof(pmalloc_item_t);
     	pmalloc_item_insert(&pm->available, newFree);

     	// Update free memory and node count
     	pm->freemem += (node->size - requestedSize) - sizeof(pmalloc_item_t);
     	pm->totalnodes++;

     	// Resize this block
     	node->size = requestedSize;

     	// Merge around the new free block
     	pmalloc_merge(pm, newFree);

     	// Return original pointer
     	return ptr;
    }

    // // Shortcut if we know there's not enough memory
    if (requestedSize - node->size > pmalloc_freemem(pm)) return NULL;

    // Expand the block if the requested size is larger than the current size
    if (requestedSize > node->size) {
    	// Does the next allocated block have enough space before it for this node to expand?
    	if(node->next == NULL || (char*)(node->next) > (char*)node + sizeof(pmalloc_item_t) + requestedSize) {
    		// Get the existing block of free space in between node and the next
    		pmalloc_item_t *freeBlock = (pmalloc_item_t __far *)((char*)node + sizeof(pmalloc_item_t) + node->size);
    		// Get the free block current size
    		uint32_t freeBlockSize = freeBlock->size;
    		// Remove that block from the free chain
    		pmalloc_item_remove(&pm->available, freeBlock);

    		// Create a new free block with the difference in size, after this node if it was resized
    		freeBlock = (pmalloc_item_t __far *)((char*)node + sizeof(pmalloc_item_t) + requestedSize);
    		freeBlock->size = freeBlockSize - (requestedSize - node->size);

    		// Add it to the free list
    		pmalloc_item_insert(&pm->available, freeBlock);

    		// Update the stats
    		pm->freemem -= requestedSize - node->size;
    		// pm->totalnodes stays the same, we removed one and added one

    		// Resize this block
    		node->size = requestedSize;

    		// Merge around the new free block
     		pmalloc_merge(pm, freeBlock);

    		// Return original pointer
    		return ptr;
    	};
    }

    // If all else fails, completely reallocate the block, copy its contents, and free the old block.

    // Allocate a new block with the requested size
    void *newPtr = pmalloc_malloc(pm, requestedSize);

    // Copy the data from the original block to the new block
    if (newPtr != NULL)
    {
        // Copy the data using memcpy
        for(uint32_t i = 0; i<node->size; i++) *((char*)newPtr + i) = *((char*)ptr + i);

        // Free the original block
        pmalloc_free(pm, ptr);
    }

    return newPtr;
}
*/