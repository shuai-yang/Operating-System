#ifndef __LEXUS__
#define __LEXUS__

#ifndef __KERNEL__
#include <sys/ioctl.h> /* for _IOW, _IO */
#endif

#define LEXUS_MINOR 230

#define LEXUS_REGISTER _IOW(LEXUS_MINOR, 0x01, struct lottery_struct)
#define LEXUS_UNREGISTER _IOW(LEXUS_MINOR, 0x02, struct lottery_struct)

struct lottery_struct {
        unsigned long pid;
        unsigned long tickets;
};

/* task states: in this assignment we only need to support two states */
typedef enum {
   READY,
   RUNNING,
} task_state;

#endif
