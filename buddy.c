/**
 * Implementation of a memory allocator based on the Buddy System.
 */
 
#include "buddy.h"
#include <unistd.h>

int buddy_init(void) { 
	int i;
    base = (void*)sbrk(DEFAULT_MAX_MEM_SIZE); // return the addrss of the start of the heap
	for(i=0; i<=max_kval; i++){
		avail[i].tag= UNUSED;
		avail[i].prev = &avail[i];
		avail[i].next = &avail[i];
	}
    
    struct block_header* bheader_ptr = (struct block_header*)base;
	bheader_ptr->tag =  FREE;
	bheader_ptr->prev =  &avail[max_kval];
	bheader_ptr->next =  &avail[max_kval];
    avail[max_kval].prev= bheader_ptr;
    avail[max_kval].next = bheader_ptr;    

	return TRUE;
}

void *buddy_malloc(size_t size)
{
	return NULL;
}

void buddy_free(void *ptr) 
{
}

void printBuddyLists(void)
{
	int i;
    struct block_header *bheader_ptr;
    for(i=0; i<=max_kval; i++){
		printf("List %d: head = %p", i, &avail[i]);
        bheader_ptr = avail[i].next;
        while(bheader_ptr != &avail[i]) {
            printf(" --> [tag=%d, kval=%d, addr=%p]", bheader_ptr->tag, bheader_ptr->kval, bheader_ptr);
            bheader_ptr = bheader_ptr->next;
        }
        printf(" --> head = %p \n", &avail[i]);
	} 
}

/* vim: set ts=4: */
