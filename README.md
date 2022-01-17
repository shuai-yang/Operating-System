# Overview

In this assignment, we will write a Linux kernel module called lexus. You should still use the cs452 VM which you used for your p1, as loading and unloading the kernel module requires the root privilege.

## Learning Objectives

 - Get familiar with another frequently used system call function: the ioctl() system call.
 - Have a better understanding of the lottery scheduling algorithm.
 - Learn the basics of the Linux CPU Scheduler APIs and learn how priorities affect process performance.

## Important Notes

You MUST build against the kernel version (3.10.0-1160.el7.x86\_64), which is the default version of the kernel installed on the cs452 VM. For this assignment, your should only allocate one single core to your VM.

## Book References

Operating Systems: Three Easy Pieces: [Chapter 9: Lottery Scheduling](https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched-lottery.pdf) (also known as "Scheduling: Proportional Share").

# Specification

You will develop a lottery scheduler for a single core processor in a Linux system. Please refer to the book chapter to have a basic understanding of how lottery scheduling works.

Your scheduler will work as a kernel module. Processes in a Linux system are by default scheduled by the default Linux CFS (Completely Fair Scheduling) scheduler. In this assignment, we do not intend to take over the default CFS scheduler, rather we try to maintain a seperate scheduler and based on our lottery scheduling policy, we choose tasks we want to run and increase their priority, and then dispatch the chosen tasks to the CFS scheduler.

## The Starter Code

The starter code already provides you with the code for a kernel module. 

What this module currently does is: create a file called /dev/lexus, which provides an inteface for applications to communicate with the kernel module. In this assignment, the only way to communicate between applications and the kernel module, is applications issue ioctl() system calls to this device file (i.e., /dev/lexus), and the kernel module will handle these ioctl commands. The two commands we need to support are: LEXUS\_REGISTER and LEXUS\_UNREGISTER. Applications who want to be managed by our lottery scheduling should issue a LEXUS\_REGISTER command to /dev/lexus, so as to register themselves into our lottery scheduling system; registered applications who want to get out should issue a LEXUS\_UNREGISTER command, so that we do not manage them anymore.

The starter code includes a function called dispatch\_timer\_callback(). This function sets a timer which goes off every 200 milliseconds, and when the timer goes off, this function wakes up the lottery scheduling thread, which runs lexus\_schedule() to hold a lottery and choose a new task based on the lottery result. lexus\_schedule() is the function you are going to implement and is the function you are going to spend most of your time on in this assignment.

A testing program (test-lexus.c) and corresponding testing scripts (lexus-test\*.sh) are also provided. The test scripts start a number of the test program simultaneously, and pass different parameters to the testing program. Each testing program will run as a seperate process, which holds a number of tickets. Following is an example - lexus-test1.sh, this script tries to launch three processes, each runs the test program - test-lexus, each of the three processes attempts to accomplish the same task, which is calcuating lucas number 44 - what lucas numbers are is not important to you, you just need to know that it is a CPU-intensive computation, and our intention here is the make sure all processes attempt to do the same computation. The example here shows, process 1 will have 100 tickets, process 2 will have 50 tickets, process 3 will have 250 tickets. 

```console
[cs452@localhost scheduler]$ cat lexus-test1.sh 
#!/bin/bash

./test-lexus 100 44&
./test-lexus 50 44&
./test-lexus 250 44&
```

Based on the lottery scheduling, when every process tries to complete the same task, processes which hold more tickets of course will be more likely to be scheduled, and thus are expected to finish faster. Therefore, when running the above script, process 3 will complete first, process 1 will complete next, process 2 will complete last. The results provided in the "Expected Results" section of this README shows exactly that.

## Functions You Need to Implement

You need to implement the following 4 functions in the kernel module:
  - lexus\_register(): this function will be called when applications want to register themselves into the lottery scheduling system. In this function you should allocate memory for a *struct lexus\_task\_struct* instance, initialize it, and add it into the global list lexus\_task\_struct.list - described below in the "Related Kernel APIs" section.
  - lexus\_unregister(): this function will be called when applications want to unregister themselves from the lottery scheduling system. In this function you should free the memory allocated in lexus\_register(), and remove the task from the global list.
  - lexus\_dev\_ioctl(): this function will be called when applications issue commands via the ioctl system call. Applications do not call lexus\_register()/lexus\_unregister() directly, they send ioctl commands to the kernel module, which handles these commands via this lexus\_dev\_ioctl() function and call the reigster/unregister functions on behalf of applications.
  - lexus\_schedule(): this is the main scheduling function of the lottery scheduling system, this function will be called every 200 milliseconds. Refer to the book chapter for how your lottery scheduling should be implemented.

You should implement lexus\_dev\_ioctl() first, and then implement lexus\_register(), lexus\_unregister(), your final step should be implementing lexus\_schedule().
 - In your lexus\_dev\_ioctl(), you either call lexus\_register() or call lexus\_unregister(), depending on which command you receive from the application via the ioctl() system call. Eventually the command will be passed from the user-space ioctl() system call to the kernel space, and it will be the second parameter of your lexus\_dev\_ioctl():

```c
static long lexus_dev_ioctl(struct file *filp, unsigned int ioctl, unsigned long arg)
```

The second parameter of this function, which is *ioctl*, is the command number, which will be either 1 or 2, but instead of using numbers, you are recommended to use LEXUS\_REGISTER or LEXUS\_UNREGISTER, as we define in lexus.h that LEXUS\_REGISTER is command 1, and LEXUS\_UNREGISTER is command 2.

The following is how applications send the ioctl commands, note that the third parameter of the ioctl is the information the application attempts to send along with the command.
```c
int register_process(struct lottery_struct lottery_info) {
        int ret;
        #ifdef DEBUG
        printf("%lu: registering...\n", lottery_info.pid);
        #endif
        ret = ioctl(lexus_fd, LEXUS_REGISTER, &lottery_info);
        if (ret == -1)
                return -errno;
        return 0;
}
```

In the above example, lottery\_info is a *struct lottery\_struct* variable which carries two fields, one is the process id, the other is the process's number of tickets. Note that this struct, which is the third parameter of the ioctl() system call, will be passed to your lexus\_dev\_ioctl() also as the third parameter, which is *arg*. Because we are passing the address of lottery\_info, therefore you can expect *arg* to be the address of lottery\_info, and remember from the previous project, in kernel space, you can not access a user-space address directly.

Once you complete your implemention of lexus\_dev\_ioctl(), you should test to see, when applications try to register, does your lexus\_register() get called; when applications try to unregister, does your lexus\_unregister() get called? If yes, then you move on to implement lexus\_register() and lexus\_unregister(). And then implement lexus\_schedule().

## Predefined Data Structures, Global Variables, and Provided Helper Functions
  - *struct lottery\_struct*: The lottery\_info variable we just mentioned, is an instance of this struct, which is defined in lexus.h. It is used by the application to pass parameters to the kernel module; because both applications and the kernel module includes "lexus.h", thus the kernel module also knows this data structure, and thus you can use it in your kernel module. This allows you to pass information from the application to the kernel module.
  - *struct lexus\_task\_struct*: each instance of this data structure is representing a process; the Linux kernel defines *struct task\_struct*, each of such struct represents a process in the Linux kernel. *struct lexus\_task\_struct* is a wrapper of *struct task\_struct*, in other words, it includes *struct task\_struct*, but also includes other fields necessary for the lottery scheduling.
  - unsigned long nTickets: this integer nTickets represents how many tickets in total we have in the lottery scheduling system, at first, it is initialized to 0; you should increment this number when you have tasks get registered, and decrement this number when tasks unregister.
  - struct task\_struct\* find\_task\_by\_pid(unsigned int pid); given a pid, this function returns a pointer pointing to its associated *struct task\_struct*. You will want to use this function in your lexus\_register() function, so that you can associate your *struct lexus\_task\_struct* with the process's *struct task\_struct*.

## Related Kernel APIs

I used the following APIs. 

  - list manipulation: In this assignment, there is one global list, and only one, which is used to connect all *struct lexus\_task\_struct* instances. This is how it is implemented: the kernel module defines a global variable called *struct lexus\_task\_struct lexus\_task\_struct*, as below:

```c
/* use this global variable to track all registered tasks, by adding into its list */
static struct lexus_task_struct lexus_task_struct;
```

*struct lexus\_task\_struct* has a field called *struct list\_head list*, given that *struct lexus\_task\_struct lexus\_task\_struct* is a global variable, its list field is actually representing a global list. The Linux kernel provides a unique way for you to add a node into this list, assume you have a *struct lexus\_task\_struct* pointer called *node*, and you want to add *node* into this global list, then you can use:

```c
list_add(&(node->list), &(lexus_task_struct.list));
```

What if you want to delete a node, in this assignment, the only place you want to delete a node is in lexus\_unregister() - when the process has finished its job, it's time for it to leave the lottery scheduling system and remove itself from the global list. Your lexus\_unregister() should execute code like this:

```c
    struct list_head *p, *n;
    struct lexus_task_struct *node;
    list_for_each_safe(p, n, &lexus_task_struct.list) {
	/* node points to each lexus_task_struct in the list. */
        node = list_entry(p, struct lexus_task_struct, list);
        list_del(p);
    }
```

The above code snippet will iterate the global list, and node will be each node, however, when deleting the node, we do not pass node as a parameter to list\_del. Instead of node, we pass the list field of node, which in the above code snippet, is p, to list\_del(). This is consistent to list\_add(), whose first parameter is also the list of a node. list\_for\_each\_safe() is a macro provided by the Linux kernel, this macro will be expanded as a for loop. Thus you can use break as needed. The above example just shows you how to iterate the list, but does not show you what you need to do in each iteration, apparently, when a process wants to unregister, it should remove its own node from the list, but should not remove other nodes from the list. In other words, you should get out of this loop as soon as you remove the node which is associated with the process which is trying to unregister.

Note that in the above example, p and n and temporary pointers, which are required for the list\_for\_each\_safe() and list\_entry() macros. Therefore, in this assignment, you are recommended to use these few lines whenever you want to iterate through this global this (once again, there is only one global list in this assignment):

```c
    struct list_head *p, *n;
    struct lexus_task_struct *node;
    list_for_each_safe(p, n, &lexus_task_struct.list) {
        node = list_entry(p, struct lexus_task_struct, list);
	/* add your code here, now that you have the node, what do you want to do with this node? */
    }
```

  - spin locks to protect the list, and global variables. Any code which manipulates the list needs to be locked, so as to avoid thing like this to happen: while you are iterating over the list, someone deletes some node from the list, or adds a node to the list. This could cause chaos to the list. To have the protection, see the following example:

```c
unsigned long flags;
spin_lock_irqsave(&lexus_lock, flags);
list_add(&(node->list), &(lexus_task_struct.list));
spin_unlock_irqrestore(&lexus_lock, flags);
```

First define a local variable called flags, and then call spin\_lock\_irqsave(&lexus\_lock, flags) before you want to manipulate the list - in the above example, we call list\_add() to add a node to the list, and then call spin\_unlock\_irqrestore(&lexus\_lock, flags) once your manipulation is finished. We will explain these lock functions in a more detailed fashion later this semester when we move on to the concurrency topics. You should also use these two functions when you attempt to change global variables, such as nTickets, and lexus\_current.

  - adjusting scheduling priority: when the timer goes off, it's time to hold a new lottery and maybe schedule a different process (than the one that is currently running). At this point, the old task should be treated like this:

```c
    struct sched_param sparam;
    sparam.sched_priority=0; 
    sched_setscheduler(node, SCHED_NORMAL, &sparam);
```
The above code will tell the CFS scheduler that this process now has a low priority. In the meantime, the newly chosen process should be treated like this:

```c
    struct sched_param sparam;
    wake_up_process(node->task);
    sparam.sched_priority=99;
    sched_setscheduler(node, SCHED_FIFO, &sparam);
```
The above code will first wakp the chosen process, and tell the CFS scheduler that this process now has a high priority - any task running on SCHED\_FIFO will hold the CPU for as long as the application needs.
  - wake\_up\_process(). As the name suggests, this function wakes up a process. Note that this function take a *struct task\_struct* as a parameter, not a *struct lexus\_task\_struct*. The timer will call this function to wake up the dispatching thread; the dispatching thread will call this function to wake up the chosen process, the lexus\_exit() function will call this function to wake up the dispatch thread so it can get ready to exit.
  - let the dispatching thread sleep: once the dispatching thread has run the lottery algorithm and chosen a process, the dispatching thread itself should go to sleep, and get woken up when the timer goes off again. The following two lines let the dispatching thread go to sleep:

```c
    set_current_state(TASK_INTERRUPTIBLE);
    schedule();	/* this function does not take any parameter, call this function will trigger the CFS scheduler to make a new scheduling decision. */
```
  - the dispatching thread() should put itself in an infinite loop so that it keeps running forever until the we remove the kernel module. To achieve this, it should execute code like this:

```c
while(!kthread_should_stop()) {
}
```

And we can see in lexus\_exit(), it calls kthread\_stop(dispatch\_kthread); that's when the dispatching thread finally gets out of this while loop and return.

  - get\_random\_bytes(). prototype: void get\_random\_bytes(void \*buf, int nbytes); This function returns the requested number of random bytes and stores them in a buffer. Below is an example of how to use this function, assuming your lottery system has 888 tickets in total.

```c
    unsigned long winner = 0;
    int randval = 0;
    /* producing a random number as the lottery winning number */
    get_random_bytes(&randval, sizeof(int)-1);
    winner = (randval & 0x7FFFFFFF) % 888;
```

The winner here will be an integer in between 0 and 888-1. Note that your code should not come to here when you have zero ticket, your system would crash due to a "divide by zero" exception.

  - APIs mentioned in the previous project are not described here, including kmalloc(), kfree(), copy\_from\_user(), copy\_to\_user(), you may use some of them, or all of them.

## Expected Results

Three testing scripts are provided, when running these test scripts, this is the expected results:

```console
[cs452@localhost scheduler]$ ./lexus-test1.sh
[cs452@localhost scheduler]$ pid 6906, with 250 tickets: computing lucas(42) took 6.20 seconds.
pid 6904, with 100 tickets: computing lucas(42) took 10.16 seconds.
pid 6905, with 50 tickets: computing lucas(42) took 11.50 seconds.

[cs452@localhost scheduler]$ ./lexus-test2.sh
[cs452@localhost scheduler]$ pid 7064, with 100 tickets: computing lucas(42) took 4.28 seconds.
pid 7065, with 5 tickets: computing lucas(42) took 8.20 seconds.

[cs452@localhost scheduler]$ ./lexus-test3.sh
[cs452@localhost scheduler]$ pid 7031, with 600 tickets: computing lucas(42) took 7.14 seconds.
pid 7028, with 550 tickets: computing lucas(42) took 15.50 seconds.
pid 7026, with 250 tickets: computing lucas(42) took 17.26 seconds.
pid 7024, with 100 tickets: computing lucas(42) took 20.68 seconds.
pid 7027, with 50 tickets: computing lucas(42) took 24.40 seconds.
pid 7029, with 20 tickets: computing lucas(42) took 25.30 seconds.
pid 7030, with 10 tickets: computing lucas(42) took 30.52 seconds.
pid 7025, with 5 tickets: computing lucas(42) took 32.12 seconds.
```

Note that the test takes some time, thus do not panic when you do not see the results right after typing the commands. From the above results, it can be seen that when all processes are trying to compute lucas number 42, processes with more tickets finish faster. Also note, while loading and unloading the kernel module requires the root privilege, running these tests does not require special privileges.

Also, a process which holds 600 lottery tickets doesn't always finish the task faster than the process which holds 550 tickets, as shown below, some fluctuation is normal and acceptable.

```console
[cs452@localhost scheduler]$ ./lexus-test3.sh 
[cs452@localhost scheduler]$ pid 3468, with 550 tickets: computing lucas(44) took 27.53 seconds.
pid 3471, with 600 tickets: computing lucas(44) took 31.15 seconds.
pid 3466, with 250 tickets: computing lucas(44) took 42.63 seconds.
pid 3464, with 100 tickets: computing lucas(44) took 47.52 seconds.
pid 3467, with 50 tickets: computing lucas(44) took 56.11 seconds.
pid 3469, with 20 tickets: computing lucas(44) took 73.45 seconds.
pid 3470, with 10 tickets: computing lucas(44) took 76.61 seconds.
pid 3465, with 5 tickets: computing lucas(44) took 81.10 seconds.
```

In addition, when your results do not seem to be reasonable, run the test multiple times.

# Submission

Due: 23:59pm, Feburary 1st, 2022. Late submission will not be accepted/graded.

# Grading Rubric (Undergraduate and Graduate)
Grade: /100

- [ 90 pts] Functional Requirements:
  - Process registering and unregistering are successful. /20
   - If your lottery scheduling system works as expected, you do not need to prove this; otherwise, you need to provide evidence proving your registering and unregistering are successful.
  - Lottery scheduling produces reasonable scheduling results. Grader will test this using the testing scripts, but you should also include your testing results in the README file.
   - lexus-test1.sh produces reasonable results. /20
   - lexus-test2.sh produces reasonable results. /20
   - lexus-test3.sh produces reasonable results. /20
  - Module can be installed and removed without crashing the system: /10
   - You won't get these points if your module doesn't implement any of the above functional requirements.

- [10 pts] Documentation:
  - README.md file (replace this current README.md with a new one using the README template. You do not need to check in this current README file.)
  - You are required to fill in every section of the README template, missing 1 section will result in a 2-point deduction.
