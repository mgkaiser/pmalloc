#include <stdio.h>
#include "pmalloc.h"

int main() {
	printf("pmalloc: Basic Example\r\r");	
	
	pmalloc_t pmblock;
	pmalloc_t *pm = &pmblock;
	
	// Initialise our pmalloc
	pmalloc_init(pm);
	#ifdef DEBUG
	pmalloc_dump_stats(pm);
	#endif
	
	// Add memory to the heap
	pmalloc_addblock(pm, (void __far *)0x300000, 0x100000);	
	#ifdef DEBUG
	pmalloc_dump_stats(pm);
	#endif	

	#define TESTCOUNT 8

	uint32_t len = 1024;
	void* mem[TESTCOUNT];

	// Allocate some memory 
	for(uint32_t i = 0; i<TESTCOUNT; i++) {
		printf("Allocating %lu bytes...\r", len*(i+1));
		mem[i] = pmalloc_malloc(pm, len*(i+1));
		#ifdef DEBUG
		pmalloc_dump_stats(pm);
		#endif
	}

	// ...use the memory...

	printf("Deallocaing\r");
	// Deallocate the memory
	for(uint32_t i = 0; i<TESTCOUNT; i++) {
		pmalloc_free(pm, mem[i]);
		#ifdef DEBUG
		pmalloc_dump_stats(pm);
		#endif
	}
	
	printf("Done\r");
	return 0;
}