/**
 * This file implements parallel mergesort.
 * Author: 	Sharon Yang // Change this line to your name.
 */

#include <stdio.h>
#include <string.h> /* for memcpy */
#include <stdlib.h> /* for malloc */
#include <time.h>
#include <sys/types.h> /* for thread id*/
#include "mergesort.h"

void printArr(int*, int);

/* this function will be called by mergesort() and also by parallel_mergesort(). */
void merge(int leftstart, int leftend, int rightstart, int rightend){  // leetcode 88
	int index = leftstart;
	int leftPos = leftstart;
	int rightPos = rightstart;
	while(leftPos <= leftend && rightPos <= rightend){
		if(A[leftPos] < A[rightPos]){
			B[index] = A[leftPos];
			leftPos ++;
		}else{
			B[index] = A[rightPos];
			rightPos ++;
		}
		index++;
	}
	if (leftPos <= leftend) {
		memcpy(&B[index], &A[leftPos], (leftend - leftPos + 1) * sizeof(int));
	} else {
		memcpy(&B[index], &A[rightPos], (rightend - rightPos + 1) * sizeof(int));
	}
	memcpy(&A[leftstart], &B[leftstart], sizeof(int) * (rightend - leftstart + 1));
	return;
} 

/* this function will be called by parallel_mergesort() as its base case. */
void mergesort(int left, int right) {
	if(left >= right){return;}
	int mid = (left + right) / 2;
	mergesort(left, mid);
	mergesort(mid + 1, right);
	merge(left, mid, mid + 1, right);
	return;
}

/* this function will be called by the testing program. */
void * parallel_mergesort(void *arg){
		struct argument* myarg = (struct argument*) arg;
		int left = myarg->left;
		int right = myarg->right;
		int level = myarg->level;
		pthread_t p1, p2;
		struct argument *leftArr;
		struct argument *rightArr;

		int mid = (left +  right) / 2;
		
		//base case
		if(level == cutoff){
			mergesort(left, right);
			return;
		}
		//if (level == cutoff) {
		//	mergesort(left, right);
		//} else {
			leftArr = (struct argument *)buildArgs(left, mid, level + 1);
			rightArr = (struct argument *)buildArgs(mid + 1, right, level + 1);
			pthread_create(&p1, NULL, parallel_mergesort, (void*)leftArr);
			pthread_create(&p2, NULL, parallel_mergesort, (void*)rightArr);
			pthread_join(p1, NULL); 
			pthread_join(p2, NULL); 
			merge(left, mid, mid+1, right);

			free(leftArr);
			free(rightArr);
		//}
}

/* we build the argument for the parallel_mergesort function. */
struct argument * buildArgs(int left, int right, int level){
		struct argument *arg;
		arg = malloc(sizeof(struct argument));
		arg->left = left;
		arg->right = right;
		arg->level = level;
		//printf("%d, %d, %d\n", arg->left, arg->right, arg->level);
		//main() calls parallel_mergesort(), where the process itself is the main thread
		return arg;
}
/* vim: set ts=4: */

void printArr(int* arr, int size){
	int i;
 	for (i = 0; i < size; i++){
        printf("%d\t", arr[i]);
    }
    printf("\n");
}

/*
int main(void)
{
    time_t begin_time,end_time;
    begin_time=time(NULL);
    Input_print();
    end_time=time(NULL);
    printf("%lfs", difftime(end_time,begin_time));
    return 0;
}

void Input_print(void)
{
	int n, i;
	int *arr;
    printf("Please enter the length of the array:\n");
    scanf("%d",&n);
	arr = malloc(sizeof(int) * n);
	B =  malloc(sizeof(int) * n);
    printf("Randomly generate random %d numbers in the array:\n",n);
    srand((unsigned )time(NULL));
    for(i=0;i<n;i++){
        arr[i]=1+rand()%(10);
    }
    for (i = 0; i < n; i++){
        printf("%d\t",arr[i]);
    }
    printf("\n");

    struct argument *arg=buildArgs(0, n-1, 0);
	//test serial merge sort
	mergesort(0, n-1);
	//parallel_mergesort(arg);

    printf("The sorted arry is:\n" );
    for(i=1;i<=n;i++)
        printf("%d\n",arr[i]);
 
}
*/
