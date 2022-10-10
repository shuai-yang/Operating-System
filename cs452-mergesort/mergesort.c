/**
 * This file implements parallel mergesort.
 * Author: 	Sharon Yang // Change this line to your name.
 */

#include <stdio.h>
#include <string.h> /* for memcpy */
#include <stdlib.h> /* for malloc */
#include <time.h>
#include "mergesort.h"


/* this function will be called by mergesort() and also by parallel_mergesort(). */
void merge(int leftstart, int leftend, int rightstart, int rightend){
	int left = leftstart;
	int right = rightstart;
	int index = leftstart;
	while(left <= leftend && right <= rightend){
		if(A[left] < A[right]){
			B[index] = A[left];
			left ++;
		}else{
			B[index] = A[right];
			right ++;
		}
		index ++;
	}
	//void *memcpy(void *dest, const void *src, size_t n);
	memcpy(B + index, A + left, (leftend - left + 1)*sizeof(int));
	for(int i=0; i<sizeof(B)/sizeof(int); i++){
		printf(" %d", B[i]);
	}
	memcpy(B + index, A + right, (rightend - right + 1)*sizeof(int));
	for(int i=0; i<sizeof(B)/sizeof(int); i++){
		printf(" %d", B[i]);
	}
	memcpy(A, B, sizeof(B));
	//free(B);
} 

/* this function will be called by parallel_mergesort() as its base case. */
void mergesort(int left, int right){
	if(left >= right){return;}
	int mid = (left + right) / 2;
	mergesort(left, mid);
	mergesort(mid + 1, right);
	merge(left, mid, mid + 1, right);
}

/* this function will be called by the testing program. */
void * parallel_mergesort(void *arg){
		prinf("parallel_mergesort called\n");
		struct argument* arg = (struct argument*) arg;
		int startIndx = arg->left;
		int endIndex = arg->right;

		if(startIndx >= endIndex){return;}

		pthread_t p1, p2;
		struct argument* leftArr, rightArr;
		int mid = (startIndx + endIndex) / 2;
		printf("Mid is %d\n", mid);
		leftArr->left = startIndx;
		leftArr->right = mid;
		rightArr->left = mid + 1;
		rightArr->right = endIndex;

		//mergesort(aleftArr->left , rightArr->right);
		pthread_create(&p1, NULL, parallel_mergesort, (void*)leftArr);
		pthread_create(&p2, NULL, parallel_mergesort, (void*)rightArr);
		pthread_join(p1, NULL); 
		pthread_join(p2, NULL); 
		merge(startIndx, mid, mid + 1, endIndex);
}

/* we build the argument for the parallel_mergesort function. */
struct argument * buildArgs(int left, int right, int level){
		struct argument *arg;
		arg = malloc(sizeof(struct argument));
		arg->left = left;
		arg->right = right;
		arg->level = level;
		//printf("%d, %d, %d\n", arg->left, arg->right,arg->level);
		return arg;
}
/* vim: set ts=4: */

/*
void Input_print(void);

int main()
{
    time_t begin_time,end_time;
    begin_time=time(NULL);
    Input_print();
    end_time=time(NULL);
    printf("%lfs", difftime(end_time,begin_time));
    return 0;
}

void Input_print()
{
    int A[100000];
	int B[100000];
    printf("Please enter the length of the array:\n");
    scanf("%d",&n);
	A = (int *) malloc(sizeof(int) * n);
	B = (int *) malloc(sizeof(int) * n)
    printf("Randomly generate random %d numbers in the array:\n",n);
    srand((unsigned )time(NULL));
    for(int i=0;i<n;i++)
    {
        arr[i]=1+rand()%(10000);
    }
    for (int i = 0; i < n; i++)
    {
        printf("%d\t",arr[i]);
    }
    printf("\n");

	pthread_t thread;
    pthread_create(&thread,NULL,Merge_Sort,arg);
    pthread_join(thread,NULL);

    struct argument *arg=buildArgs(0, n-1, 0);
	parallel_mergesort(arg);

    printf("The sorted arry is:\n" );
    for(int i=1;i<=n;i++)
        printf("%4d\n",arr[i]);
 
}*/
