#include <stdio.h>
#include <stdlib.h>
#include <time.h> /* for gettimeofday system call */
#include <errno.h> /* for errno */
#include <fcntl.h> /* for O_RDWR */
#include "lexus.h"

int lexus_fd;
//#define DEBUG 1

/* this function calculates the lucas series, the lucas series
 * goes like this: 2, 1, 3, 4, 7, 11, 18, 29, 47, 76, 123... */
int lucas(int n){
	if(n == 0)
		return 2;
	if(n == 1)
		return 1;

	return (lucas(n - 1) + lucas(n - 2));
}

double getMilliSeconds(void)
{
	struct timeval now;
	gettimeofday(&now, (struct timezone *)0);
	return (double) now.tv_sec*1000.0 + now.tv_usec/1000.0;
}

/* when registration is successful, register_process return 0.
 * otherwise, it returns -1. */
int register_process(struct lottery_struct lottery_info) {
	int ret;
	#ifdef DEBUG
	printf("%lu: registering...\n", lottery_info.pid);
	#endif
	ret = ioctl(lexus_fd, LEXUS_REGISTER, &lottery_info);
	/* on success, ioctl returns 0. */
	if (ret!=0){
		printf("ioctl fails\n");
		return -1;
	}
	return 0;
}

/* when unregistration is successful, unregister_process return 0. 
 * otherwise, it returns -1. */
int unregister_process(struct lottery_struct lottery_info) {
	int ret;
	#ifdef DEBUG
	printf("%lu: unregistering...\n", lottery_info.pid);
	#endif
	ret = ioctl(lexus_fd, LEXUS_UNREGISTER, &lottery_info); // the 3rd parameter must have "&".
	/* on success, ioctl returns 0. */
	if (ret!=0){
		printf("ioctl fails\n");
		return -1;
	}
	return 0;
}

/* Main function that takes two arguments from users: numberoftickets and number of jobs. */
int main(int argc, char *argv[]) {
	int ret;
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <tickets> <n>\n", argv[0]);
		exit(1);
	}

	unsigned long pid = getpid();
	unsigned long tickets = atoi(argv[1]);
	unsigned long n  = atoi(argv[2]);
	struct lottery_struct tmp;

	lexus_fd = open("/dev/lexus", O_RDWR);
	if (lexus_fd <= 0) {
		printf("lexus: failed to open lexus device\n");
		ret = -errno;
		return ret;
		}

	tmp.pid = pid;
	tmp.tickets = tickets;
	ret = register_process(tmp);
	if(ret != 0){
		printf("%lu: unable to register. \n", pid);
		exit(1);
	}
	#ifdef DEBUG
	printf("%lu: registered\n", pid);
	#endif

	double start_time;
	double computing_time;

	// sort the input (and time it)
	start_time = getMilliSeconds();
	lucas(n);
	computing_time = getMilliSeconds() - start_time;
	printf("pid %lu, with %lu tickets: computing lucas(%lu) took %4.2lf seconds.\n", pid, tickets, n, computing_time/1000.0);

	ret=unregister_process(tmp);
	if(ret != 0){
		printf("%lu: unable to unregister. \n", pid);
		exit(1);
	}
	#ifdef DEBUG
	printf("%lu: unregistered\n", pid);
	#endif

	close(lexus_fd);
	return 0;
}

/* vim: set ts=4: */
