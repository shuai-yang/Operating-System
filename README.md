# Overview

In this assignment, we will write a Linux kernel module called lexus. Note that, you will only be able to test this assignment on a Linux machine where you have root privilege. A VMware-based CentOS 7 (64 bit) VM image is provided. Later on we will refer to this VM as the cs452 VM (username/password: cs452/cs452, run commands with sudo to gain root privilege). You can also download a CentOS 7 (64 bit) and install it by yourself, and you can also use VirtualBox.

## Important Notes

You MUST build against the kernel version (3.10.0-957.el7.x86_64) installed on the cs452 VM. You will need to use root in this project is to load and unload the kernel module. For this assignment, your should only allocate one single core to your VM.

# Specification

Your will develop a lottery scheduler for a single core processor in a Linux system. Please refer to the book chapter to have a basic understanding of how lottery scheduling works.

Your scheduler will work as a kernel module. Processes in a Linux system are by default scheduled by the default Linux CFS scheduler. In this assignment, we do not intend to take over the default CFS scheduler, rather we try to maintain a seperate scheduler and based on our lottery scheduling policy, we choose tasks we want to run and increase their priority, and then dispatch the chosen tasks to the CFS scheduler.

## The Starter Code

The starter code already provides you with the code for a kernel module. 

What this module currently does is, create a file called /dev/lexus, which provides an inteface for applications to communicate with the kernel module. In this assignment, the only way to communicate between applications and the kernel module, is applications issue ioctl() system calls to this device file (i.e., /dev/lexus), and the kernel module will handle these ioctl commands. The two commands we need to support are: register and unregister. Applications who want to be managed by our lottery scheduling should issue a register command to /dev/lexus; registered applications who want to get out should issue an unregister command.

You need to implement the following 5 functions:
  - lexus_register(): this function will be called when applications want to register themselves into the lottery scheduling system.
  - lexus_unregister(): this function will be called when applications want to unregister themselves from the lottery scheduling system.
  - lexus_dev_ioctl(): this function will be called when applications issue commands via the ioctl system call. Applications do not call lexus_register()/lexus_unregister() directly, they send ioctl commands to the kernel module, which handles these commands via this lexus_dev_ioctl() function and call the reigster/unregister functions on behalf of applications.
  - lexus_schedule(): this is the main scheduling function of the lottery scheduling system. refer to the book chapter for how this part should be implemented.
  - dispatch_timer_callback(): when the timer expires, this function will be called. In this function you update the timer and wake up the lottery scheduling thread. The idea is, we want to hold a lottery every 200 milliseconds - thus we want to timer which expires every 200 milliseconds.

A testing program (test-lexus.c) and corresponding testing scripts (lexus-test*.sh) are also provided. The test scripts start a number of the test program simultaneously, and pass different parameters to the testing program. Each testing program will run as a seperate process, which holds a number of tickets. When every process tries to complete the same task, processes which hold more tickets of course will be more likely to be scheduled, and thus are expected to finish faster, and this complies with the basic idea of lottery scheduling.

## Predefined Data Structures
  - struct lexus_task_struct: each instance of this data structure is representing a process; the Linux kernel defines struct task_struct, each of such struct represents a process in the Linux kernel. lexus_task_struct is a wrapper of task_struct, in other words, it include task_struct, but also includes other fields necessary for the lottery scheduling.
  - struct lottery_struct: this data structure, defined in lexus.h, is used by the application to pass parameters to the kernel module; because both applications and the kernel module includes "lexus.h", thus the kernel module also knows this data structure, and thus you can use it in your kernel module. This allows you to pass information from the application to the kernel module.

## Related Kernel APIs

  - list manipulation: to be added soon.
  - spin locks to protect the list: to be added soon.
  - slab memory allocation and reclaim: to be added soon.
  - adjusting scheduling priority: to be added soon.

# References

Operating systems: three easy pieces: chapter 9: Lottery Scheduling (the inside of the chapter is also titled as "Scheduling: Proportional Share").

# Submission

Due: 23:59pm, Feburary 1st, 2022. Late submission will not be accepted/graded.

# Grading Rubric (Undergraduate and Graduate)
Grade: /100

- [ 90 pts] Functional Requirements:
  - [20 pts] lexus_schedule() is called periodically. - if your lottery scheduling system works as expected, you do not need to prove this; otherwise, you need to provide evidence proving your lexus_schedule() is called periodically.
  - [20 pts] process registering and unregistering are successful. - if your lottery scheduling system works as expected, you do not need to prove this; otherwise, you need to provide evidence proving your registering and unregistering are successful.
  - [50 pts] lottery scheduling works - producing reasonable scheduling results. Grader will test this using the testing scripts, but you should also include your testing results in the README file.

- [10 pts] Documentation:
  - README.md file (replace this current README.md with a new one using the README template)
