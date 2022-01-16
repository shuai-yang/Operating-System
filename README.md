# Overview

In this assignment, we will write a Linux kernel module called lexus. You should still the cs452 VM which you used for your p1, as loading and unloading the kernel module requires the root privilege.

## Important Notes

You MUST build against the kernel version (3.10.0-1160.el7.x86\_64), which is the default version of the kernel installed on the cs452 VM. For this assignment, your should only allocate one single core to your VM.

# Specification

Your will develop a lottery scheduler for a single core processor in a Linux system. Please refer to the book chapter to have a basic understanding of how lottery scheduling works.

Your scheduler will work as a kernel module. Processes in a Linux system are by default scheduled by the default Linux CFS scheduler. In this assignment, we do not intend to take over the default CFS scheduler, rather we try to maintain a seperate scheduler and based on our lottery scheduling policy, we choose tasks we want to run and increase their priority, and then dispatch the chosen tasks to the CFS scheduler.

## The Starter Code

The starter code already provides you with the code for a kernel module. 

What this module currently does is: create a file called /dev/lexus, which provides an inteface for applications to communicate with the kernel module. In this assignment, the only way to communicate between applications and the kernel module, is applications issue ioctl() system calls to this device file (i.e., /dev/lexus), and the kernel module will handle these ioctl commands. The two commands we need to support are: register and unregister. Applications who want to be managed by our lottery scheduling should issue a register command to /dev/lexus; registered applications who want to get out should issue an unregister command.

You need to implement the following 5 functions in the kernel module:
  - lexus\_register(): this function will be called when applications want to register themselves into the lottery scheduling system.
  - lexus\_unregister(): this function will be called when applications want to unregister themselves from the lottery scheduling system.
  - lexus\_dev\_ioctl(): this function will be called when applications issue commands via the ioctl system call. Applications do not call lexus\_register()/lexus\_unregister() directly, they send ioctl commands to the kernel module, which handles these commands via this lexus\_dev\_ioctl() function and call the reigster/unregister functions on behalf of applications.
  - lexus\_schedule(): this is the main scheduling function of the lottery scheduling system. Refer to the book chapter for how this part should be implemented.
  - dispatch\_timer\_callback(): when the timer expires, this function will be called. In this function you update the timer and wake up the lottery scheduling thread. The idea is, we want to hold a lottery every 200 milliseconds - thus we want a timer which expires every 200 milliseconds.

A testing program (test-lexus.c) and corresponding testing scripts (lexus-test\*.sh) are also provided. The test scripts start a number of the test program simultaneously, and pass different parameters to the testing program. Each testing program will run as a seperate process, which holds a number of tickets. When every process tries to complete the same task, processes which hold more tickets of course will be more likely to be scheduled, and thus are expected to finish faster, and this complies with the basic idea of lottery scheduling.

## Predefined Data Structures
  - struct lexus\_task\_struct: each instance of this data structure is representing a process; the Linux kernel defines struct task\_struct, each of such struct represents a process in the Linux kernel. lexus\_task\_struct is a wrapper of task\_struct, in other words, it include task\_struct, but also includes other fields necessary for the lottery scheduling.
  - struct lottery\_struct: this data structure, defined in lexus.h, is used by the application to pass parameters to the kernel module; because both applications and the kernel module includes "lexus.h", thus the kernel module also knows this data structure, and thus you can use it in your kernel module. This allows you to pass information from the application to the kernel module.

## Related Kernel APIs

  - list manipulation: to be added soon.
  - spin locks to protect the list: to be added soon.
  - slab memory allocation and reclaim: to be added soon.
  - adjusting scheduling priority: to be added soon.

## Expected Results

Three testing scripts are provided, when running these test scripts, this is the expected results:

```console
[cs452@localhost scheduler]$ ./lexus-test.sh
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

# Book References

Operating Systems: Three Easy Pieces: [Chapter 9: Lottery Scheduling](https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched-lottery.pdf)(as know as "Scheduling: Proportional Share").

# Submission

Due: 23:59pm, Feburary 1st, 2022. Late submission will not be accepted/graded.

# Grading Rubric (Undergraduate and Graduate)
Grade: /100

- [ 90 pts] Functional Requirements:
  - [20 pts] lexus\_schedule() is called periodically. - if your lottery scheduling system works as expected, you do not need to prove this; otherwise, you need to provide evidence proving your lexus\_schedule() is called periodically.
  - [20 pts] process registering and unregistering are successful. - if your lottery scheduling system works as expected, you do not need to prove this; otherwise, you need to provide evidence proving your registering and unregistering are successful.
  - [50 pts] lottery scheduling works - producing reasonable scheduling results. Grader will test this using the testing scripts, but you should also include your testing results in the README file.

- [10 pts] Documentation:
  - README.md file (replace this current README.md with a new one using the README template. You do not need to check in this current README file.)
