
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <sys/ioctl.h> 
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include  "../infiniti.h"
#include "harness.h"

static struct sigaction dfl_segv_action;
static int fd = 0;

void * infiniti_malloc(size_t size) {
    struct alloc_request req;
    memset(&req, 0, sizeof(struct alloc_request));

    req.size = size;
    
    ioctl(fd, LAZY_ALLOC, &req); 
	/* when this ioctl returns, if it's successful, then the allocated memory is pointed by req.addr. */
    return (void *)req.addr;
}

void infiniti_free(void * addr) {
    ioctl(fd, LAZY_FREE, addr);
}

void infiniti_dump() {
    ioctl(fd, DUMP_STATE, NULL);

    return;
}

void infiniti_invlpg(void * addr) {
    ioctl(fd, INVALIDATE_PAGE, addr);
    return;
}

static void segv_handler(int signum, siginfo_t * info, void * context) {
    struct page_fault fault;

    if (signum != SIGSEGV) {
	printf("well that is strange...\n");
	return;
    }  

    //    printf("SIGSEGV\n");

    // si_addr -> fault addr

    // si_code == 1 -> not present: SEGV_MAPERR - address not mapped to object
    // si_code == 2 -> permission error: SEGV_ACCERR - invalid permissions for mapped object

	/* the kernel will deliver si_code and si_addr to us via signals, and
	 * they will be embeded in that siginfo_t struct. struct page_fault is defined by us,
	 * which also has two fields, corresponding to these two fields. and we send this struct
	 * to our kernel module when we issue the PAGE_FAULT command. */
    fault.fault_addr = (unsigned long long)info->si_addr;
    fault.error_code = info->si_code;

	/* it looks like the OS will take care of this, if ioctl returns 0,
	 * the os will make sure the pc counter goes back to that faulting instruction,
	 * basically rerun it, or maybe run the right next instruction? 
	 * this is what the man page of signal says: the kernel performs the necessary preparatory steps for
	 * execution of the signal handler, one of the steps is: various pieces of signal-related context are saved into a
	 * special frame that is created on the stack. the saved information includes:
	 * the program counter register (i.e., the address of the
	 * next instruction in the main program that should be
	 * executed when the signal handler returns); */
    if (ioctl(fd, PAGE_FAULT, &fault)) {
	struct sigaction old_action;
	// if ioctl returns 1, then handler failed
	// we need to turn off our handler, and resignal to crash

	/* for SIGSEGV, save the previous action to &old_action, and
	 * install the new action, which is &dfl_segv_action. note that
	 * this new action is the one we saved in init_infiniti(). */
	sigaction(SIGSEGV, &dfl_segv_action, &old_action);
	kill(getpid(), SIGSEGV);
    }

}

int init_infiniti(void) {
    struct sigaction action;

    action.sa_sigaction = segv_handler;

    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask,SIGSEGV);

    action.sa_flags = SA_RESTART | SA_SIGINFO;
    
	/* for SIGSEGV, save the previous action to &dfl_segv_action, and 
	 * install the new action, which is &action. */
    sigaction(SIGSEGV, &action, &dfl_segv_action);


    fd = open(dev_file, O_RDONLY);

    
    if (fd == -1) {
		fprintf(stderr, "error opening infiniti device file (%s)\n", dev_file);
	return -1;
    }

    return 0;
}

/* vim: set ts=4: */
