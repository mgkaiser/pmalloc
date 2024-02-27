#include <stdio.h>
#include "pmalloc.h"

pmalloc_t pmblock;

int main() {
	printf("pmalloc: Basic Example\r\r");	
		
	pm = (pmalloc_t __far *)&pmblock;
	
	// Initialise our pmalloc
	pmalloc_init(pm);	
	pmalloc_dump_stats(pm);
	
	// Add memory to the heap
	#ifdef FMX
	pmalloc_addblock(pm, (void __far *)0x300000, 0x100000);	
	#endif
	#ifdef JR
	//pmalloc_addblock(pm, (void __far *)0x300000, 0x100000);	
	#endif	
	pmalloc_dump_stats(pm);
	

	#define TESTCOUNT 32

	uint32_t len = 1024;
	void __far * mem[TESTCOUNT];

	// Allocate some memory 
	for(uint32_t i = 0; i<TESTCOUNT; i++) {
		printf("Allocating %lu bytes...\r", len*(i+1));
		mem[i] = pmalloc_malloc(pm, len*(i+1));
		printf("%p\r",mem[i]);		
		pmalloc_dump_stats(pm);		
	}

	// ...use the memory...

	printf("Deallocaing\r");
	// Deallocate the memory
	for(uint32_t i = 0; i<TESTCOUNT; i+=2) {
		pmalloc_free(pm, mem[i]);		
		pmalloc_dump_stats(pm);		
	}
	for(uint32_t i = 1; i<TESTCOUNT; i+=2) {
		pmalloc_free(pm, mem[i]);		
		pmalloc_dump_stats(pm);		
	}
	
	printf("Done\r");
	return 0;
}