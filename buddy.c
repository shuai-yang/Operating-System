#include "buddy.h"
#include <unistd.h>
/* blocks in avail[i] are of size 2**i. */
#define BLOCKSIZE(i) (1 << (i))
/* the address of the buddy of a block from avail[i]. 
#define BUDDYOF(p,i) ((struct block_header*)( ((int)p) ^ (1 << (i)) ))*/
#include <stddef.h>

/**
 * Initialize the buddy system to the default size 
 * @return  TRUE if successful, FALSE otherwise.
 */
int buddy_init(void) { 
    int i;
    base = (void*)sbrk(DEFAULT_MAX_MEM_SIZE); // return the addrss of the start of the heap
	for(i=0; i<=max_kval; i++){
		avail[i].tag= UNUSED;
		avail[i].prev = &avail[i];
		avail[i].next = &avail[i];
	}
    
    struct block_header* block_header_ptr = (struct block_header*)base;
	block_header_ptr->tag =  FREE;
	block_header_ptr->prev =  &avail[max_kval];
	block_header_ptr->next =  &avail[max_kval];
    avail[max_kval].prev= block_header_ptr;
    avail[max_kval].next = block_header_ptr;    

	return TRUE;
}

void *buddy_malloc(size_t size)
{
    struct block_header* p;
    struct block_header* buddy;
    ptrdiff_t distance_bytes;
    //search and split
    /*compute logsize as the least integer such that logsize >= log2(size) */
    int lgsize;
    for (lgsize = 0; BLOCKSIZE( lgsize ) < size; lgsize++);
   
    /*int lgsize = 0;
    struct block_header* block_header_ptr;
    
    size = size - 1;
    while(size > 0){
        size = size >> 1;
        lgsize++;
    }*/
    printf("Search starts here: %d \n", lgsize);

    while(avail[lgsize].next == &avail[lgsize]){
        lgsize++;
    }
    printf("Found node here: %d \n", lgsize);
    //let p points to the block header we just found,and get p's buddy
    p = avail[lgsize].next;
    //buddy = BUDDYOF(p, lgsize);
    buddy = ((struct block_header*)((long)p ^ (1ULL<<lgsize)));

    distance_bytes=(char*)buddy-(char*)p;				
    printf("when splitting, the distance between p and its buddy (buddy on the right) is %td\n", distance_bytes);
    //split

          //we need to split a bigger block   
          //struct block_header* ptr, buddy_ptr;
          //ptr = buddy_malloc( BLOCKSIZE( logsize + 1 ) );

          //if (ptr != NULL) {
              // split and put extra on a free list 
            //  buddy_ptr = BUDDYOF( ptr, logsize );
              //(struct block_header*)buddy_ptr = &avail[i];
             // &avail[i] = buddy_ptr;
         // }

   //       return ptr;
    // }
    
   return NULL;
}
void buddy_free(void *ptr) 
{
}

/**
 * Prints out all the lists of available blocks in the Buddy system.
 */
void printBuddyLists(void)
{
	int i;
    int count = 0;
    struct block_header *block_header_ptr;
    for(i=0; i<=max_kval; i++){
		printf("List %d: head = %p", i, &avail[i]);
        block_header_ptr = avail[i].next;
        while(block_header_ptr != &avail[i]) {
            count ++;
            printf(" --> [tag=%d, kval=%d, addr=%p]", block_header_ptr->tag, block_header_ptr->kval, block_header_ptr);
            block_header_ptr = block_header_ptr->next;
        }
        printf(" --> head = %p \n", &avail[i]);
	} 
    printf("\nNumber of available blocks = %d \n", count);
}

/* vim: set ts=4: */

