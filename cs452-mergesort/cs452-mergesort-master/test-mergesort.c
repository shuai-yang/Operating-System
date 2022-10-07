#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h> /* for times system call */
#include <sys/time.h>  /* for gettimeofday system call */
#include <unistd.h>
#include <error.h>
#include "mergesort.h"

/* the number of levels of threads, specified by the user */
int cutoff = 0;

/* the array we want to sort */
int *A;

/* when merge, we need a temp array (named B) to temporarily store elements of the original array */
int *B;

// function prototypes
int check_if_sorted(int A[], int n);
void generate_random_array(int A[], int n, int seed);
double getMilliSeconds(void);

/*
---------------------------------------------------------------------------
clock_t times(struct tms *buffer);
times() fills the structure pointed to by buffer with
time-accounting information.  The structure defined in
<sys/times.h> is as follows:
struct tms {
    clock_t tms_utime;       user time
    clock_t tms_stime;       system time
    clock_t tms_cutime;      user time, children
    clock_t tms_cstime;      system time, children
The time is given in units of 1/CLK_TCK seconds where the
value of CLK_TCK can be determined using the sysconf() function
with the agrgument _SC_CLK_TCK.
---------------------------------------------------------------------------
*/


/* we do not really use this function anymore, but somehow we still keep it here. */
float report_cpu_time(void)
{
	struct tms buffer;
	float cputime;

	times(&buffer);
	cputime = (buffer.tms_utime)/ (float) sysconf(_SC_CLK_TCK);
	return (cputime);
}

/* we do not really use this function anymore, but somehow we still keep it here. */
float report_sys_time(void)
{
	struct tms buffer;
	float systime;

	times(&buffer);
	systime = (buffer.tms_stime)/ (float) sysconf(_SC_CLK_TCK);
	return (systime);
}

double getMilliSeconds(void)
{
    struct timeval now;
    gettimeofday(&now, (struct timezone *)0);
    return (double) now.tv_sec*1000.0 + now.tv_usec/1000.0;
}


/*
 * generate_random_array(int A[], int n, int seed):
 *
 * description: Generate random integers in the range [0,RANGE]
 *              and store in A[0..n-1]
 */

#define RANGE 1000000

void generate_random_array(int A[], int n, int seed)
{
	int i;
	srandom(seed);
	for (i=0; i<n; i++)
        	A[i] = random()%RANGE;
}


/*
 * check_if_sorted(int A[], int n):
 *
 * description: returns TRUE if A[1..n] are sorted in nondecreasing order
 *              otherwise returns FALSE
 */

int check_if_sorted(int A[], int n)
{
	int i;
	for (i=0; i<n-1; i++) {
		if (A[i] > A[i+1]) {
			return FALSE;
		}
	}
	return TRUE;
}

/* this function prints A, but we only print the first 100 elements */
void printA(void){
	int i;
	printf("Array A:");
	/* FIXME: we assume the size of A is at least 100, and this will fail if it is lower than 100 */
	for(i=0;i<100;i++){
		printf(" %d",A[i]);
	}
	printf("\n");
}

/* this function prints B, but we only print the first 100 elements */
void printB(void){
	int i;
	printf("Array B:");
	/* FIXME: we assume the size of B is at least 100, and this will fail if it is lower than 100 */
	for(i=0;i<100;i++){
		printf(" %d",B[i]);
	}
	printf("\n");
}

int main(int argc, char **argv) {

	if (argc < 4) { // there must be at least one command-line argument
			fprintf(stderr, "Usage: %s <input size> <cutoff level> <seed> \n", argv[0]);
			exit(1);
	}

	int n = atoi(argv[1]);
	if(n<=1){
		printf("the input size must be at least 2!");
		exit(1);
	}
	cutoff = atoi(argv[2]);
	int seed = atoi(argv[3]);

	A = (int *) malloc(sizeof(int) * n);
	B = (int *) malloc(sizeof(int) * n);

	// generate random input
	generate_random_array(A, n, seed);

	double start_time;
	double sorting_time;

	// sort the input (and time it)
	start_time = getMilliSeconds();
	/* first index is 0, last index is n-1, we consider the main thread as level 0 */
	struct argument *arg=buildArgs(0, n-1, 0);
	parallel_mergesort(arg);
	sorting_time = getMilliSeconds() - start_time;

	// print the array, for debugging purpose.
	//printA();
	//printB();
	// print results if correctly sorted otherwise cry foul and exit
	if (check_if_sorted(A,n)) {
		printf("Sorting %d elements took %4.2lf seconds.\n", n,  sorting_time/1000.0);
	} else {
		printf("%s: sorting failed!!!!\n", argv[0]);
		free(arg);
		free(B);
		free(A);
		exit(EXIT_FAILURE);
	}
	/* Now we can free the memory for the two arrays, as well as free memory for arg. */
	free(arg);
	free(B);
	free(A);

	exit(EXIT_SUCCESS);
}
