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


/* this function will be called by mergesort() and also by parallel_mergesort(). */
void merge(int leftstart, int leftend, int rightstart, int rightend){  // leetcode 88
	//printf("merge() called\n");
	//int left = leftstart;
	//int right = rightstart;
	//int index = leftstart;
	int index = 0;
	while(leftstart <= leftend && rightstart <= rightend){
		if(A[leftstart] < A[rightstart]){
			B[index] = A[leftstart];
			leftstart ++;
		}else{
			B[index] = A[rightstart];
			rightstart ++;
		}
		index ++;
	}
	//void *memcpy(void *dest, const void *src, size_t n);
	memcpy(B + index, A + leftstart, (leftend - leftstart + 1)*sizeof(int));
	memcpy(B + index, A + rightstart, (rightend - rightstart + 1)*sizeof(int));
	memcpy(A, B, (int)sizeof(B));
	free(B);

	/*
	while (leftstart <= leftend) {
		B[index++] = A[leftstart++];
	}
	while (rightstart <= rightend) {
		B[index++] = A[leftstart++];
	}
	index = 0;
	for (i = leftstart; i <= rightend; i++) {
		A[i] = B[index++];
	}
	*/
	return;
} 

/* this function will be called by parallel_mergesort() as its base case. */
void mergesort(int left, int right){
	//printf("mergesort() called\n");
	if(left >= right){return;}
	int mid = (left + right) / 2;
	mergesort(left, mid);
	mergesort(mid + 1, right);
	merge(left, mid, mid + 1, right);
	return;
}

/* this function will be called by the testing program. */
void * parallel_mergesort(void *arg){
		//printf("parallel_mergesort() called\n");
		struct argument* myarg = (struct argument*) arg;
		int startIndex = myarg->left;
		//printf("startIndex is %d\n", startIndex);
		int endIndex = myarg->right;
		//printf("endIndex is %d\n", endIndex);
		int i;

		//base case
		if(startIndex >= endIndex){return mergesort(startIndex, endIndex);}

		pthread_t p1, p2;
		struct argument *leftArr;
		struct argument *rightArr;
		int midIndex = (startIndex + endIndex) / 2;
		//printf("MidIndex is %d\n", midIndex);
		printf("\nArr:");
		for(i=startIndex ; i<=endIndex; i++){
			printf(" %d", A[i]);
		}
		leftArr = (struct argument*)buildArgs(startIndex, midIndex, myarg->level - 1);
		//leftArr.left = startIndex;
		//leftArr.right = midIndex;
		//leftArr.level = myarg->level - 1;
		rightArr = (struct argument*)buildArgs(midIndex + 1, endIndex, myarg->level - 1);
		//rightArr.left = midIndex + 1;
		//rightArr.right = endIndex;
		//rightArr.level = myarg->level - 1;
		printf("\nleftArr:");
		for(i=leftArr->left; i<=leftArr->right; i++){
			printf(" %d", A[i]);
		}
		printf("\nrightArr: ");
		for(i=rightArr->left; i<=rightArr->right; i++){
			printf(" %d", A[i]);
		}
		//mergesort(aleftArr->left , rightArr->right);
		pthread_create(&p1, NULL, parallel_mergesort, (void*)leftArr);
		//mergesort(leftArr.left, leftArr.right);
		//printf("\np1 is %ld", pthread_self());
		pthread_create(&p2, NULL, parallel_mergesort, (void*)rightArr);
		//mergesort(rightArr.left, rightArr.right);
		//printf("\np2 is %ld", pthread_self());
		pthread_join(p1, NULL); 
		//mergesort(leftArr.left, leftArr.right);
		//merge(leftArr.left, leftArr.right, rightArr.left, rightArr.right);
		pthread_join(p2, NULL); 
		mergesort(rightArr->left, rightArr->right);
		merge(leftArr->left, leftArr->right, rightArr->left, rightArr->right);
		return;  // what to be returned? 
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


void Input_print(void);
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
	int n;
	int arr[100000];
    printf("Please enter the length of the array:\n");
    scanf("%d",&n);
	arr = malloc(sizeof(int) * n);
	B =  malloc(sizeof(int) * n);
    printf("Randomly generate random %d numbers in the array:\n",n);
    srand((unsigned )time(NULL));
    for(i=0;i<n;i++)
    {
        arr[i]=1+rand()%(10);
    }
    for (i = 0; i < n; i++)
    {
        printf("%d\t",arr[i]);
    }
    printf("\n");

    struct argument *arg=buildArgs(0, n-1, 0);
	parallel_mergesort(arg);

    printf("The sorted arry is:\n" );
    for(i=1;i<=n;i++)
        printf("%d\n",arr[i]);
 
}
*/