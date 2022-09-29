#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "harness.h"

#define MAX_REQUEST 4096
#define MAX_ITEMS 100

struct element {
	char *ptr;
	size_t size;
};

int main(int argc, char *argv[])
{
	int i;
	char ch;
	int count;
	unsigned long int seed;
	size_t size;
	struct element x[MAX_ITEMS];
	int loc;

	count = 100;
	seed = 1234;
	srandom(seed);

	init_infiniti();	
	
	for (i = 0; i < MAX_ITEMS; i++) {
		x[i].ptr = NULL;
		x[i].size = 0;
	}
	
	for (i=0; i < count; i++) {
		loc = random() % MAX_ITEMS; // where to put in our table
		if (x[loc].ptr) {
			infiniti_free(x[loc].ptr);
			printf("infiniti_freed address %p of size %lu in x[%d]\n", x[loc].ptr, x[loc].size, loc);
			x[loc].ptr = NULL;
			x[loc].size = 0;
		} else {
			size = random() % MAX_REQUEST + 1; // how big a request
			x[loc].ptr = (char *) infiniti_malloc(size*sizeof(char));
			if (x[loc].ptr == NULL) {
				printf("infiniti_malloc failed\n:");
				exit(1);
			}
			x[loc].size = size*sizeof(char);
			memset(x[loc].ptr, '1', x[loc].size);
			printf("infiniti_malloced %zd bytes and stored address %p at x[%d]\n", size*sizeof(char), x[loc].ptr, loc);
		}
		
	}
	
	for (i = 0; i < MAX_ITEMS; i++) {
	        if (x[i].ptr){ 
			infiniti_free(x[i].ptr);
			infiniti_dump();
		}
    }
	exit(0);	
}

/* vim: set ts=4: */
