#include "buddy.h"
#include <unistd.h>
#include <stddef.h>

int lgsize;
int found_pos;
int buddy_init(void) { 
    int i;
    base = (void*)sbrk(DEFAULT_MAX_MEM_SIZE); 
	for(i=0; i<=max_kval; i++){
		avail[i].tag= UNUSED;
		avail[i].kval = i;
		avail[i].prev = &avail[i];
		avail[i].next = &avail[i];
	}
	i--;
    struct block_header* block_header_ptr = (struct block_header*)base;
	block_header_ptr->tag = FREE;
	block_header_ptr->kval = avail[i].kval;
	block_header_ptr->prev =  &avail[max_kval];
	block_header_ptr->next =  &avail[max_kval];
    avail[max_kval].prev= block_header_ptr;
    avail[max_kval].next = block_header_ptr;    
	return TRUE;
}

/*compute lgsize as the least integer such that lgsize >= log2(size) */
int getLgSize(size_t size){ 
	int lgsize = 0;
    size = size - 1;
    while(size > 0){
        size = size >> 1;
        lgsize++;
    }
	return lgsize;
}

void *removeNode(int k){
	struct block_header *block;
	block = avail[k].next;

	avail[k].next = block->next;
	block->next->prev = block->prev;

	block->next = NULL;
	block->prev = NULL;

	return block;
}

void *getBuddy(struct block_header* p){
	struct block_header* buddy;
	ptrdiff_t distance_bytes;
	//b = base + (p-base)^(1ULL<<k)
	long temp = (char*)p - (char*)base;
	temp = temp^(1ULL<<lgsize);
	buddy = (struct block_header*)((char*)base + temp);
	distance_bytes=(char*)buddy-(char*)p;				
	printf("when splitting, the distance between p and its buddy (buddy on the right) is %td\n", distance_bytes);
	return buddy;
}

void *buddy_malloc(size_t size){
	int i = 0;
	struct block_header* p;
	struct block_header* buddy;

	lgsize = getLgSize(size);
	for(i=lgsize; i<=max_kval; i++){
		if(avail[i].next != &avail[i]){
			break;
		}
	}
	//let p points to the block header we just found
	printf("Found node at %p \n",  (char*)&avail[i]);
	p = removeNode(i);
	printf("Removed node at %p \n", (char*)p);
	p->tag = RESERVED;
	printBuddyLists();
	buddy = getBuddy(p)
	/*
	while(i >= lgsize){
		i--; 
		buddy = getBuddy(p);
		//buddy = (struct block_header*)((char*)base+((long)((char*)p - (char*)base))^(1ULL<<lgsize));
		
		buddy->tag = FREE;
		buddy->kval = i;
		buddy->prev = &avail[i];
		buddy->next = p->next;
		p->next = buddy;
		p->prev = buddy;
		buddy->prev = p;
	}	
	
	// move p 24 bytes forward, and malloc(1) returns p  ???*/
	size += sizeof(struct block_header);
	p = (struct block_header*)((char*)p +size);
	//p = (struct block_header*)((char*)p +structof(struct block_header));
	//p = (char*)((char*)p + sizeof(struct block_header));
	return p;
}

void buddy_free(void *ptr) {
	/*
	struct block_header* p;
	struct block_header* buddy;
	strcut block_header* temp;
	int k;
	if(ptr == NULL){return NULL;}

	//move p back 24 bytes
	p = (struct block_header*)((char*)ptr - sizeof(struct block_header));
	buddy = getBuddy(p);

	while (i < found_pos){
		buddy = (struct block_header*)((long)p ^ (1ULL<<lgsize));
		//remove body from list and update kval???
		avail[i].next->next = NULL;
		avail[i].next = &avail[i];
		i++;
	}

	//add one node
	struct block_header* block_header_ptr = (struct block_header*)base;
	block_header_ptr->tag =  FREE;
	block_header_ptr->prev =  &avail[found_pos];
	block_header_ptr->next =  &avail[found_pos];
    avail[found_pos].prev= block_header_ptr;
    avail[found_pos].next = block_header_ptr;
	*/
	return;
}

void printBuddyLists(void){
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


