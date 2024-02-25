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
	#ifdef FMX
	pmalloc_addblock(pm, (void __far *)0x300000, 0x100000);	
	#endif
	#ifdef JR
	//pmalloc_addblock(pm, (void __far *)0x300000, 0x100000);	
	#endif
	#ifdef DEBUG
	pmalloc_dump_stats(pm);
	#endif	

	#define TESTCOUNT 32

	uint32_t len = 1024;
	void __far * mem[TESTCOUNT];

	// Allocate some memory 
	for(uint32_t i = 0; i<TESTCOUNT; i++) {
		printf("Allocating %lu bytes...\r", len*(i+1));
		mem[i] = pmalloc_malloc(pm, len*(i+1));
		printf("%p\r",mem[i]);
		#ifdef DEBUG
		pmalloc_dump_stats(pm);
		#endif
	}

	// ...use the memory...

	printf("Deallocaing\r");
	// Deallocate the memory
	for(uint32_t i = 0; i<TESTCOUNT; i+=2) {
		pmalloc_free(pm, mem[i]);
		#ifdef DEBUG
		pmalloc_dump_stats(pm);
		#endif
	}
	for(uint32_t i = 1; i<TESTCOUNT; i+=2) {
		pmalloc_free(pm, mem[i]);
		#ifdef DEBUG
		pmalloc_dump_stats(pm);
		#endif
	}
	
	printf("Done\r");
	return 0;
}