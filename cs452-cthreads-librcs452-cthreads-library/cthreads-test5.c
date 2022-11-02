#include <stdio.h>
#include <stdlib.h>
#include "cthreads.h"

#define N 32

/* we have forks 0 to forks N-1. */
cthread_mutex_t forks[N];

/* this function calculates the lucas series, the lucas series
 * goes like this: 2, 1, 3, 4, 7, 11, 18, 29, 47, 76, 123... */
int lucas(int n){
	if(n == 0)
		return 2;
	if(n == 1)
		return 1;

//    printf("lucas number %d is %d\n", n, b);
	return (lucas(n - 1) + lucas(n - 2));
}

/* the two helper functions from the book chapter. */

/* we need %N here, even for the left, because if we don't use %N, then, p, in our situation, 
 * may reach N, which makes us access forks[N], which triggers a seg fault. 
 * what we want is: philosopher 1, has its left fork 1, right fork 2,
 * philosopher 2, has its left fork 2, right fork 3,
 * philosopher 32, has its left fork 0, right fork 1. */
int left(int p)  { return p % N; }
int right(int p) { return (p + 1) % N; }

/* get and put forks function from the bokk chapter */

void get_forks(int p) {
	if(p == N) {
		cthread_mutex_lock(&forks[right(p)]);
		cthread_mutex_lock(&forks[left(p)]);
	} else {
		cthread_mutex_lock(&forks[left(p)]);
		cthread_mutex_lock(&forks[right(p)]);
	}
}

void put_forks(int p) {
	cthread_mutex_unlock(&forks[left(p)]);
	cthread_mutex_unlock(&forks[right(p)]);
}

/* we have philosopher 1 to philosopher N. */
void* philosopher(void *arg) {
    long p;

    p = (long) arg;
    p = p+1;

	/* unlike the book chapter which uses a infinite while(1) loop, we do some computation and exit */
	lucas(30);
	get_forks(p);
	lucas(30);
	put_forks(p);

    printf("thread %ld exiting\n", p);
    cthread_exit(NULL);
    return NULL;
}

int main(int argc, char *argv[]) {
    cthread_t tids[N];
    long i; // it seems we will get a compiler warning if we change this long to int.
    
    for (i = 0; i < N; i++) {
        cthread_mutex_init(&forks[i]);
    }

    for (i = 0; i < N; i++) {
        cthread_create(&tids[i], philosopher, (void *)i);
    }

    for (i = 0; i < N; i++) {
        cthread_join(tids[i], NULL);
    }

    printf("main: exiting\n");

    return 0;
}

/* vim: set ts=4: */
