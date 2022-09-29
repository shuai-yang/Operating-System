#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "harness.h"

int main(int argc, char ** argv) {
	/* this will setup the signal handler to take care of seg fault */
    init_infiniti();
    int i = 0;
    char * buf = NULL;
    char * real_buf = NULL;
    char * real_buf2 = NULL;
	
	/* these two lines will cause LAZY_ALLOC sent to the kernel module,
	 * but they will succeed even if there is no physical memory allocated.
	 * physical memory will be needed, when we actually access these addresses,
	 * and that's what we do in the next few lines, and that's when page faults
	 * happens, and the page fault handler will be called to handle them. */
    real_buf = infiniti_malloc(10);
	infiniti_dump();
    real_buf2 = infiniti_malloc(10);
	infiniti_dump();

    real_buf2[0] = 's';
    real_buf2[1] = 'u';
    real_buf2[2] = 'c';
    real_buf2[3] = 'c';
    real_buf2[4] = 'e';
    real_buf2[5] = 's';
    real_buf2[6] = 's';
    real_buf2[7] = '\0';
    real_buf[0] = 't';
    real_buf[1] = 'e';
    real_buf[2] = 's';
    real_buf[3] = 't';
    real_buf[4] = '\0';
	/* note: we do not call free for these allocated memory, so that we can show another coalescing situation. */
    while(i < 30){

        buf = infiniti_malloc(10);
        buf[0] ='a';
    	printf("buf is %s\n", buf);
        i++;
    }

    printf("%s\n", real_buf);
    printf("%s\n", real_buf2);
	infiniti_free(buf);
	infiniti_dump();
	infiniti_free(real_buf);
	infiniti_dump();
	infiniti_free(real_buf2);
	infiniti_dump();
    return 0;
}

/* vim: set ts=4: */
