/**
 * This file implements parallel mergesort.
 * Author: 	Sharon Yang // Change this line to your name.
 */

#include <stdio.h>
#include <string.h> /* for memcpy */
#include <stdlib.h> /* for malloc */
#include "mergesort.h"


/* this function will be called by mergesort() and also by parallel_mergesort(). */
void merge(int leftstart, int leftend, int rightstart, int rightend){  
	int index = leftstart; // not 0
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
		//memcpy(&B[index], &A[leftPos], (leftend - leftPos + 1) * sizeof(int));
		memcpy(B + index, A + leftPos, (leftend - leftPos + 1) * sizeof(int));
	} else {
		//memcpy(&B[index], &A[rightPos], (rightend - rightPos + 1) * sizeof(int));
		memcpy(B + index, A + rightPos, (rightend - rightPos + 1) * sizeof(int));
	}
	//memcpy(&A[leftstart], &B[leftstart], (rightend - leftstart + 1) * sizeof(int));
	memcpy(A + leftstart, B + leftstart, (rightend - leftstart + 1) * sizeof(int)); // not copy the entire A or B arraies
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
		leftArr = (struct argument *)buildArgs(left, mid, level + 1);
		rightArr = (struct argument *)buildArgs(mid + 1, right, level + 1);
		
		//base case
		if(level == cutoff){
			mergesort(left, right);
		//general case
		} else {	
			//leftArr = (struct argument *)buildArgs(left, mid, level + 1);
			//rightArr = (struct argument *)buildArgs(mid + 1, right, level + 1);
			pthread_create(&p1, NULL, parallel_mergesort, (void*)leftArr);
			pthread_create(&p2, NULL, parallel_mergesort, (void*)rightArr);
			pthread_join(p1, NULL); 
			pthread_join(p2, NULL); 
			merge(left, mid, mid+1, right);
			//free(leftArr); // should be moved out of else stmt to the very end
			//free(rightArr); // should be moved out of else stmt to the very end
		}
		free(leftArr);  
		free(rightArr);
		return NULL; //return;
}

/* we build the argument for the parallel_mergesort function. */
struct argument * buildArgs(int left, int right, int level){
		struct argument *arg;
		arg = malloc(sizeof(struct argument));
		arg->left = left;
		arg->right = right;
		arg->level = level;
		//main() calls parallel_mergesort(), where the process itself is the main thread
		return arg;
}
/* vim: set ts=4: */