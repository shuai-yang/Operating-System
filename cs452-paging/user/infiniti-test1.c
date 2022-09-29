#include <stdio.h>
#include "harness.h"

void main(int argc, char ** argv) {
        init_infiniti();
        char * buf;
	infiniti_dump();
        buf = infiniti_malloc(4098);
	infiniti_dump();

        buf[50] = 'H';
        buf[51] = 'e';
        buf[52] = 'l';
        buf[53] = 'l';
        buf[54] = 'o';
        buf[55] = ' ';
        buf[56] = 'B';
        buf[57] = 'o';
        buf[58] = 'i';
        buf[59] = 's';
        buf[60] = 'e';
        buf[61] = '!';
        buf[62] = 0;
	infiniti_dump();

        printf("%s\n", (char *)(buf + 50));
	infiniti_free(buf);
	infiniti_dump();
	return;
}
