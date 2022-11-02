#include <stdio.h>
#include <stdlib.h>
#include "cthreads.h"
#include "cthreads.c"

void *mythread(void *arg) {
    printf("%s\n", (char *) arg);
    cthread_exit(NULL);
    return NULL; // returning here just avoid the compiler warning saying "warning: control reaches end of non-void function"
}

int main(int argc, char *argv[]) {                    
    if (argc != 1) {
	fprintf(stderr, "usage: main\n");
	exit(1);
    }

    cthread_t p1, p2;
    printf("main: begin\n");
    cthread_create(&p1, mythread, "A"); 
    cthread_create(&p2, mythread, "B");
    // join waits for the threads to finish
    cthread_join(p1, NULL); 
    cthread_join(p2, NULL); 
    printf("main: end\n");
    return 0;
}

