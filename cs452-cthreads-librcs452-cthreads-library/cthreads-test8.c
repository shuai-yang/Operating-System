/* This is the solution to leetcode problem No.1117, Building H2O.
 * author: Jidong Xiao
 * */

#include <stdio.h> /* for printf */
#include <stdlib.h> /* for malloc */
#include <string.h> /* for strcat */
#include "cthreads.h"

char str[50];

void releaseHydrogen(){
        strcat(str, "H");
}

void releaseOxygen(){
        strcat(str, "O");
}

typedef struct {
    // User defined data may be declared here.
	/* H waits for H */
    cthread_sem_t semH;
	/* O waits for O */
    cthread_sem_t semO;
	/* H waits for O */
    cthread_sem_t semHO;
	/* O waits for H */
    cthread_sem_t semOH;
} H2O;

H2O* h2oCreate() {
    H2O* obj = (H2O*) malloc(sizeof(H2O));
    
    // Initialize user defined data here.
    cthread_sem_init(&(obj->semH),2);
    cthread_sem_init(&(obj->semO),1);
    cthread_sem_init(&(obj->semHO),0);
    cthread_sem_init(&(obj->semOH),0);
    return obj;
}

void* hydrogen(void* orig_obj) {
	H2O *obj = (H2O *)orig_obj;
    cthread_sem_wait(&(obj->semH)); // semaphore so that only 2 hydrogen can get in
    cthread_sem_wait(&(obj->semHO));
    cthread_sem_post(&(obj->semOH));
    // releaseHydrogen() outputs "H". Do not change or remove this line.
    releaseHydrogen();
    cthread_sem_post(&(obj->semH)); // wake up other hydrogen threads
    cthread_exit(NULL);
    return NULL; // returning here just avoid the compiler warning saying "warning: control reaches end of non-void function"

}

void* oxygen(void* orig_obj) {
	H2O *obj = (H2O *)orig_obj;
    cthread_sem_wait(&(obj->semO)); // lock so that only 1 oxygen can get in
    cthread_sem_post(&(obj->semHO));
    cthread_sem_post(&(obj->semHO));
    cthread_sem_wait(&(obj->semOH)); // wait two hydrogen threads, thus get woken up twice.
    cthread_sem_wait(&(obj->semOH));
    // releaseOxygen() outputs "O". Do not change or remove this line.
    releaseOxygen();
    cthread_sem_post(&(obj->semO)); // wake up other oxygen threads
    cthread_exit(NULL);
    return NULL; // returning here just avoid the compiler warning saying "warning: control reaches end of non-void function"

}

void h2oFree(H2O* obj) {
    // User defined data may be cleaned up here.
    free(obj);
}

int main(int argc, char *argv[]) {
	int i;
	/* warning: the num of h threads must be 2x the num of o threads, otherwise we will get a deadlock situation. */
	int num_hthreads=10;
	int num_othreads=5;
	cthread_t *htids = (cthread_t *)malloc(sizeof(cthread_t)*num_hthreads);
	cthread_t *otids = (cthread_t *)malloc(sizeof(cthread_t)*num_othreads);

    H2O *obj = h2oCreate();

	for(i=0;i<num_hthreads;i++){
		cthread_create(&htids[i], hydrogen, (void *)obj);
	}

	for(i=0;i<num_othreads;i++){
		cthread_create(&otids[i], oxygen, (void *)obj);
	}

	for(i=0;i<num_hthreads;i++){
    	cthread_join(htids[i], NULL);
	}

	for(i=0;i<num_othreads;i++){
    	cthread_join(otids[i], NULL);
	}

    h2oFree(obj);

    printf("main: building H2O:\n");
    printf("%s\n", str);
    printf("main: exiting\n");

    return 0;
}

/* vim: set ts=4: */
