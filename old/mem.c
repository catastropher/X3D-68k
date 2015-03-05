// C Source File
// Created 2/16/2015; 7:45:17 PM

#include "error.h"
#include "config.h"

#include <tigcclib.h>

#define MAX_ALLOCS 50



#ifdef MANAGED_MEM

typedef struct {
	unsigned long size;
	void *mem;
} AllocEntry;

AllocEntry malloc_tab[MAX_ALLOCS];

void cleanup_mem(void) {
	int i;
	
	for(i = 0; i < MAX_ALLOCS; i++)
		if(malloc_tab[i].mem)
			free(malloc_tab[i].mem);
}

#endif

void free_mem(void* ptr) {
#ifdef MANAGED_MEM
	int i;
	char found = 0;
	
	for(i = 0; i < MAX_ALLOCS; i++) {
		if(malloc_tab[i].mem == ptr) {
			malloc_tab[i].mem = NULL;
			found = 1;
		}
	}
	
	if(!found)
		error("Invalid free");
#endif

#ifdef DEBUG
	if(!ptr)
		error("NULL free");
#endif

	free(ptr);

}


void init_mem() {
#ifdef MANAGED_MEM
	int i;
	
	for(i = 0; i < MAX_ALLOCS; i++)
		malloc_tab[i].mem = NULL;
		
	atexit(cleanup_mem);
#endif
}

void* alloc_mem(unsigned long size) {
	void* ptr = malloc(size);
	int i;
	
	if(!ptr)
		error("Out of memory");
	
#ifdef MANAGED_MEM
	for(i = 0; i < MAX_ALLOCS; i++) {
		if(!malloc_tab[i].mem) {
			malloc_tab[i].mem = ptr;
			malloc_tab[i].size = size;
			
			return ptr;
		}
	}
	
	error("Too many allocs");
#endif
}