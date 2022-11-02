
/*
   synchronization-part1/safe-bank-balance.c
   Uses Pthread Mutexes (binary semaphores) to protect the bank balance
   variable. This isn't very good coding style. For a better way to code
   this see ch9/monitors/account folder.

   @author amit
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cthreads.h"
#include "cthreads.c"

typedef struct account account;
struct account {
	double balance;
	cthread_mutex_t mutex;
};
account *myacct;

void *threadMain(void *);
cthread_t *tids;
int numThreads;
int count;

int main(int argc, char **argv)
{
	int i;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s <numThreads> <iterations>\n", argv[0]);
		exit(1);
	}

	numThreads  = atoi(argv[1]);
	count = atoi(argv[2]);
	if (numThreads > 32) {
		fprintf(stderr, "Usage: %s Too many threads specified. Defaulting to 32.\n", argv[0]);
		numThreads = 32;
	}

	myacct = (account *) malloc(sizeof(account));
	myacct->balance = 0.0;
	cthread_mutex_init(&(myacct->mutex));
	printf("initial balance = %lf\n", myacct->balance);


    tids = (cthread_t *) malloc(sizeof(cthread_t)*numThreads);
    for (i=0; i<numThreads; i++)
        cthread_create(&tids[i], threadMain, (void *) NULL);

    for (i=0; i<numThreads; i++)
        cthread_join(tids[i], NULL);

    printf("final balance = %lf\n", myacct->balance);
    exit(0);
}

void *threadMain(void *arg)
{
	int i;
	int amount;

	for (i=0; i<count; i++) {
		amount = 1;
		cthread_mutex_lock(&(myacct->mutex));
		//printf("Locked\n");
		myacct->balance += amount;
		//printf("Unlocked\n");
		cthread_mutex_unlock(&(myacct->mutex));
		
	}
	cthread_exit(NULL);
	return NULL;
}
