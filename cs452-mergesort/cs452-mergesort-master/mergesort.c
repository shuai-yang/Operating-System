/**
 * This file implements parallel mergesort.
 * Author: Jidong Xiao // Change this line to your name.
 */

#include <stdio.h>
#include <string.h> /* for memcpy */
#include <stdlib.h> /* for malloc */
#include "mergesort.h"

/* this function will be called by mergesort() and also by parallel_mergesort(). */
void merge(int leftstart, int leftend, int rightstart, int rightend){
}

/* this function will be called by parallel_mergesort() as its base case. */
void mergesort(int left, int right){
}

/* this function will be called by the testing program. */
void * parallel_mergesort(void *arg){
		return NULL;
}

/* we build the argument for the parallel_mergesort function. */
struct argument * buildArgs(int left, int right, int level){
		return NULL;
}

/* vim: set ts=4: */
