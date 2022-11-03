#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEV_NAME_SIZE  20
//#define DEBUG 1

char *buf;

static void run_read_test(char *device, int bufsize)
{    
	int src;
	int in;

	src = open(device, O_RDONLY);
    if (src < 0) {
		printf("open failed:");
		exit(1);
	}

	in = read(src, buf, bufsize);
    if (in < 0) {
		printf("read failed:");
		exit(2);
	} else {
		#ifdef DEBUG
		printf("toyota returned %d characters\n", in);
		#endif
	}

	buf[bufsize]='\0';
	printf("read %d bytes: %s\n", in, buf);

	close(src);

}/* run_read_test */

static void run_write_test(char *device, int bufsize)
{
	int src;
	int out;

	src = open(device, O_WRONLY);
    if (src < 0) {
		printf("open for write failed:");
		exit(1);
	}
	#ifdef DEBUG
	printf("attempting to write to toyota device\n");
	#endif
	out = write(src, buf, bufsize);
	printf("wrote %d bytes: %s\n", out, buf);
	close(src);

}/* run_write_test */

int main(int argc, char **argv)
{
	int bufsize; 
	char *device;
	int minor;

	device = (char *)malloc(sizeof(char)*DEV_NAME_SIZE);
	strcpy(device, "/dev/toyota0");

	bufsize=5;
	buf = (char *)malloc(sizeof(char)*(bufsize+1));
	strncpy(buf, "bcabc", bufsize);
	run_write_test(device, bufsize);
	free(buf);
	bufsize=6;
	buf = (char *)malloc(sizeof(char)*(bufsize+1));
	run_read_test(device, bufsize);
	free(buf);

	bufsize=8;
	buf = (char *)malloc(sizeof(char)*(bufsize+1));
	strncpy(buf, "cbacdcbc", bufsize);
	run_write_test(device, bufsize);
	free(buf);
	bufsize=6;
	buf = (char *)malloc(sizeof(char)*(bufsize+1));
	run_read_test(device, bufsize);
	free(buf);

	bufsize=11;
	buf = (char *)malloc(sizeof(char)*(bufsize+1));
	strncpy(buf, "bcbcbcababa", bufsize);
	run_write_test(device, bufsize);
	free(buf);
	bufsize=6;
	buf = (char *)malloc(sizeof(char)*(bufsize+1));
	run_read_test(device, bufsize);
	free(buf);

	bufsize=86;
	buf = (char *)malloc(sizeof(char)*(bufsize+1));
	strncpy(buf, "tknvntfipavdqjiyslpdlokuymbutpynnxqekoktlqzrhoyvbewklzuamhwtqygsiakymyqwqiqtouynaiowwf", bufsize);
	run_write_test(device, bufsize);
	free(buf);
	bufsize=25;
	buf = (char *)malloc(sizeof(char)*(bufsize+1));
	run_read_test(device, bufsize);
	free(buf);

    exit(0);
} /* main */

/* vim: set ts=4: */
