#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEV_NAME_SIZE  20

static void run_read_test(char *device, int bufsize)
{    
	char *buf;
	int src;
	int in;

	src = open(device, O_RDONLY);
    if (src < 0) {
		printf("Open failed:\n");
		exit(1);
	}

	buf = (char *) malloc(sizeof(char)*(bufsize+1));
	in = read(src, buf, bufsize);
    if (in < 0) {
		printf("Read failed:\n");
	} else {
		printf("toyota returned %d characters\n", in);
	}

	buf[bufsize]='\0';
	printf("%s\n",buf);
	free(buf);
	close(src);
}/* run_read_test */

static void run_write_test(char *device, int bufsize)
{
	char *buf;
	int src;
	int out;

	src = open(device, O_WRONLY);
    if (src < 0) {
		printf("Open for write failed:\n");
		exit(1);
	}
	buf = (char *) malloc(sizeof(char)*(bufsize+1));
	printf("Attempting to write to toyota device\n");
	/* buffer size: 11 */
	out = write(src, "bcbcbcababa", bufsize);
	printf("Wrote %d bytes.\n", out);
	free(buf);
	close(src);
}/* run_write_test */

int main(int argc, char **argv)
{
	int bufsize; 
	char *device;

	device = (char *)malloc(sizeof(char)*DEV_NAME_SIZE);

	strcpy(device, "/dev/toyota1");
	bufsize = 1000;
	run_write_test(device, bufsize);
	run_read_test(device, bufsize);

	strcpy(device, "/dev/toyota2");
	bufsize = 1000;
	run_write_test(device, bufsize);
	run_read_test(device, bufsize);

	strcpy(device, "/dev/toyota3");
	bufsize = 1000;
	run_write_test(device, bufsize);
	run_read_test(device, bufsize);

	free(device);
    exit(0);
} /* main */

/* vim: set ts=4: */
