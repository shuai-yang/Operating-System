/* This multithreaded program computes the sum of an array.
 * author: Jidong Xiao
 * */

#include <stdio.h> /* for printf */
#include "cthreads.h"
  
// size of array
#define ARRAY_SIZE 16
  
// maximum number of threads
#define MAX_NUM_OF_THREADS 4
  
int a[] = { 1, 5, 7, 10, 12, 14, 15, 18, 20, 22, 25, 27, 30, 64, 110, 220 };
int total_sum = 0;
int part = 0;
  
void* worker_sum(void* arg)
{
	int i;
	int thread_sum = 0;

    // each thread computes sum of 1/4th of array
    int thread_part = part;
	part = part + 1;
  
    for (i = thread_part * (ARRAY_SIZE/4); i < (thread_part + 1) * (ARRAY_SIZE/ 4); i++){
        thread_sum += a[i];
	}

	total_sum = total_sum + thread_sum;

	cthread_exit(NULL);
	return NULL; // returning here just avoid the compiler warning saying "warning: control reaches end of non-void function"
}
  
// driver code
int main()
{
	int i;
    cthread_t threads[MAX_NUM_OF_THREADS];
  
    // creating 4 threads
    for (i = 0; i < MAX_NUM_OF_THREADS; i++){
        cthread_create(&threads[i], worker_sum, (void*)NULL);
	}
  
    // joining 4 threads i.e. waiting for all 4 threads to complete
    for (i = 0; i < MAX_NUM_OF_THREADS; i++){
        cthread_join(threads[i], NULL);
	}
  
	printf("sum is %d\n", total_sum);
	return 0;
}

/* vim: set ts=4: */
