# Overview

In this assignment, we will write a Linux kernel module called toyota. This module will serve as a simple character device driver. You should still use the cs452 VM (username:cs452, password: cs452) which you used for your tesla, lexus, and infiniti, as loading and unloading the kernel module requires the root privilege.

## Learning Objectives

- Learning how to write a simple device driver in a Linux system.
- Practicing managing strings in C programming.
- Practicing using a stack to solve programming problems.

## Important Notes

You MUST build against the kernel version (3.10.0-1160.el7.x86_64), which is the default version of the kernel installed on the cs452 VM.

In this assignment, we assume applications only access our device sequentially, in other words, you do not need to consider the case where multiple application processes/threads access the device concurrently. For example, if one program is reading or writing the device, then no other programs will be reading or writing the device at this same time.

## Book References

Operating Systems: Three Easy Pieces: [I/O Devices](https://pages.cs.wisc.edu/~remzi/OSTEP/file-devices.pdf).

This chapter explains what roles I/O devices play in a computer system, and how device drivers work in general, but in reality, every device is different, and its behavior is defined by the device vendor - the company who makes the device. Given that everyone's computer is different, it is not realistic for us to write a device driver for a specific device - your computer may not have this device. Thus in this assignment, we will just pretend that there is a device, and we allow applications to access this device via our device driver. And in this device driver, we will simulate the behavior of a device. In particular, we allow applications to open, read, write, and close the device.

## Background

### Character Devices vs Block Devices

Linux systems define three types of devices.

- character devices (also known as char device): character devices support access by characters, transferring data by characters or by bytes. Example: keyboard.

- block devices: block devices support random access, transferring data by blocks (e.g. 4KB per block). Example: disk.

- network devices: Linux considers most network devices as a special type, they belong to neither character devices nor block devices. Example: network interface card.

In this assignment, we are writing a character device driver.

### Major Device Number vs Minor Device Number

Linux systems use a pair of numbers to differentiate devices: major device number and minor device number. Take the following as an example,

```console
(base) [jidongxiao@onyx ~]$ ls -l /dev/sda*
brw-rw----. 1 root disk 8, 0 Mar 21 08:26 /dev/sda
brw-rw----. 1 root disk 8, 1 Mar 21 08:26 /dev/sda1
brw-rw----. 1 root disk 8, 2 Mar 21 08:26 /dev/sda2
brw-rw----. 1 root disk 8, 3 Mar 21 08:26 /dev/sda3
```
Each device in Unix/Linux systems has a corresponding file under the */dev* directory. Here, */dev/sda* represents the hard disk. This disk currently has 3 partitions: */dev/sda1*, */dev/sda2*, */dev/sda3*. Linux kernel reserves major number 8 for the driver of this disk, and then uses minor number 0 to represent the whole disk, uses minor number 1 to represent the first partition, uses minor number 2 to represent the second partition, and uses minor number 3 to represent the third partition. In other words, if a device contains multiple members, then we use a different minor number to indicate each member.

# Specification

## The Starter Code

The starter code looks like this:

```console
[cs452@localhost cs452-device-driver]$ ls
Makefile  README.md  toyota.c  toyota.h  toyota_load  toyota-test1.c  toyota-test2.c  toyota-test3.c  toyota-test4.c  toyota_unload
```

You will be completing the toyota.c file. You should not modify the toyota.h file.

**Warning**: In previous assignments, we learned that we can use *sudo insmod* to install a kernel module, and use *sudo rmmod* to remove the kernel module. Please do not use these two commands in this assignment. We have two scripts for you: to load the module, run *sudo ./toyota_load*, and to remove the module, run *sudo ./toyota_unload*. *toyota_load* does two things: install the module, and then created device files (/dev/toyota0, /dev/toyota1, /dev/toyota2, /dev/toyota3); *toyota_unload* does two things: remove the module, and then delete these 4 device files.

Four testing programs (toyota-test[1-4].c) are provided. Refer to the [Expected Results](#expected-results) section to see what are expected when running these testing programs.

## Driver Requirements

The toyota driver is a simple character driver that supports the open, read, write and close operations. The driver supports four minor numbers: 0, 1, 2, and 3. The device files are: /dev/toyota0, /dev/toyota1, /dev/toyota2, /dev/toyota3. We will also create a link from /dev/toyota to /dev/toyota0, so that /dev/toyota0 acts as the default device when someone accesses /dev/toyota. The following describes how this driver should behave:

On writing to toyota devices:

- if a process tries to write /dev/toyota1 or /dev/toyota2, the toyota device driver works like /dev/null - it pretends to write a buffer but doesn't actually write to any device. 
- if a process tries to write to /dev/toyota3, it suffers from sudden death! Keep reading this README, and you will find out which function you can use to achieve this.
- if a process tries to write to /dev/toyota0, the toyota device driver must store the written data into an internal buffer - we assume applications (upon each write operation) only write one string to this device and we assume this string only contains lower case English letters.

On reading from /dev/toyota0, /dev/toyota1, /dev/toyota2 and /dev/toyota3, the driver will process the data (which is a string which is stored in the aforementioned internal buffer) in such a way: it removes duplicate letters from the string, so that every letter appears once and only once. You must make sure your result is the smallest in lexicographical order among all possible results. In this next paragraph, we will refer to this result as the **result string**.

Note that the driver does not just return the above **result string** to the user application. Rather, it returns a stream of the **result string**. 

For example, if the **result string** is *abc*, and if the user wants to read 9 bytes, then the final result that the read() function of your driver should return will be *abcabcabc*. 

In case if the number of bytes requested by the user is not a multiple of the length of the **result string**, then the final result that your read() function returns may contain a part of the **result string**. Here are some examples:

- if the **result string** is *abc*, and if the user wants to read 1 bytes, then the read() function of your driver should return *a*;
- if the **result string** is *abc*, and if the user wants to read 4 bytes, then the read() function of your driver should return *abca*;
- if the **result string** is *abc*, and if the user wants to read 5 bytes, then the read() function of your driver should return *abcab*;
- if the **result string** is *abc*, and if the user wants to read 10 bytes, then the read() function of your driver should return *abcabcabca*.

## Functions You Need to Implement

Here are the prototypes of the functions that you need to implement in toyota.c.

```c
static int toyota_open (struct inode *inode, struct file *filp);
static int toyota_release (struct inode *inode, struct file *filp);
static ssize_t toyota_read (struct file *filp, char *buf, size_t count, loff_t *f_pos);
static ssize_t toyota_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static int __init toyota_init(void);
static void __exit toyota_exit(void);
```

In the remainder of this README file, the above six functions will be referred to as your *open*(), *release*(), *read*(), *write*(), *init*(), *exit*(), respectively.

## Predefined Data Structures and Global Variables

The starter code does not define any data structures. It defines one global variable called *struct file_operations toyota_fops*, which will be described in the next section.

You are recommended to define 3 global variables.

- you may want to have a global integer variable to track which device is being accessed: 0,1,2, or 3?
- you may want to have a global buffer (or a global pointer) so that both *read*() and *write*() can access.
- you may want to have a global integer variable to track the major device number.

## Related Kernel APIs

I used the following APIs:

- kmalloc();
- kfree();
- copy_from_user();
- copy_to_user();

Read the README file of assignment 1 (i.e., [tesla](https://github.com/jidongbsu/cs452-system-call)) to see how to use them. After calling *kmalloc*(), you may want to use *memset*() to set the allocated memory to 0.

- register_chrdev();
- unregister_chrdev();
All drivers are eventually managed by the kernel, and we call register_chrdev() to register a char device driver with the kernel, and call *unregister_chrdev*() to unregister a char device driver from the kernel. To register, you can call *register_chrdev*() in your *init*() function like this:

```c
static int __init toyota_init(void){


    /*
     * register your major, and accept a dynamic number.
     */
    register_chrdev(0, "toyota", &toyota_fops);
    ...
}
```

The above code registers this driver into the kernel. The kernel will assign an available major number (a number between 0 and 255) to this device/driver. If the registration succeeds, *register_chrdev*() returns the assigned major number. Otherwise, *register_chrdev*() returns a negative value.

The first argument of *register_chrdev*(): if this argument is non-zero, it means we want to specify one specific major number; if this argument is zero, it means we do not care what the number is, just assign us any number that is still available. Here we use *0*, which means we want the kernel to dynamically allocate one number to us. Remember, the assigned number will be the return value of *register_chrdev*(), and this same number needs to be passed to *unregister_chrdev*() as its first argument. This is why you are recommended to use a global variable to store this return value so that later on it can easily be passed to *unregister_chrdev*().

The second argument of *register_chrdev*(), which is *toyota*, tells the kernel this driver is named as *toyota*. This name will then appear in */proc/devices* when the module is installed.

The third argument of *register_chrdev*(), which is *&toyota_fops*, tells the kernel, *toyota_fops*, which is a *struct file_operations* variable, will be responsible for file operations on /dev/toyota (including /dev/toyota0, /dev/toyota1, /dev/toyota2, ...). *toyota_fops* is defined as following:

```c
/*  The different file operations.
 *  Any member of this structure which we don't explicitly assign will be initialized to NULL by gcc. */
static struct file_operations toyota_fops = {
    .owner =      THIS_MODULE,
    .read =       toyota_read,
    .write =      toyota_write,
    .open =       toyota_open,
    .release =    toyota_release,
};
```

This struct variable (together with the *register_chrdev*() function), tells the kernel: when users try to open /dev/toyota*, the kernel should call *toyota_open*(); when users try to close /dev/toyota*, the kernel should call *toyota_release*(); when users try to read from /dev/toyota*, the kernel should call *toyota_read*(); when users try to write to /dev/toyota*, the kernel should call *toyota_write*().

You can then unregister the driver like this in your *exit*() function.
```c
static void __exit toyota_exit(void){
	/* reverse the effect of register_chrdev(). */
    unregister_chrdev(**whatever returned by register_chrdev()**, "toyota");
    ...
}    
```

- try_module_get();
- module_put(); 
Device drivers need to maintain a usage count, so that it can not be removed when it's in use. To this end, you can call *try_module_get*() in your *open*() function, and call *module_put*() in your *release*() function. You can call *try_module_get*() like this in your *open*() function,

```c
static int toyota_open (struct inode *inode, struct file *filp){
    ...
    /* increment the use count. */
    try_module_get(THIS_MODULE);
    return 0;          /* success */
}
```

You can call *module_put*() like this in your release() function:
```c
static int toyota_release (struct inode *inode, struct file *filp){
    ...
    /* decrement the use count. */
    module_put(THIS_MODULE);
    return 0;
}
```

- kill_pid();
This function allows you to kill a process from the kernel level, you can call it like this:

```c
kill_pid(task_pid(current), SIGTERM, 1);
```

Remember *current* in Linux kernel has a special meaning, it represents the current running process, and therefore you do not need to define/declare it.

### string operation APIs

You may need to use:

- strlen();
- strcat();
- strncat();

They are all available in kernel code - the Linux kernel re-implements them in the kernel space. You do not need to include any extra header files to use these functions. Use them in the kernel space the same way as you normally would in applications.

## Provided Helper Functions

- NUM();
Your write() function will behave differently based on the minor number of the device being accessed. To know the minor number of the accessed device, you can call *NUM*() like this:

```c
static int toyota_open (struct inode *inode, struct file *filp)
{
    int num = NUM(inode->i_rdev);
}
```

You may also want to check to make sure this *num* is smaller than 4 - because we only support minor number 0-3; if this *num* is equal to or greater than 4, your open() function should return *-ENODEV*, meaning no such a device.

## Debugging

Note that the kernel print messages will not show on the screen. The messages are, however, logged in the file /var/log/messages. You can open another terminal and watch the output to the system messages file with the command:

```console
# sudo tail -f /var/log/messages
```

Alternatively, you can use the command:

```console
# sudo dmesg --follow
```

## Expected Results

In order to run any tests, you need to first run *make* to compile everything, and then run *sudo ./toyota_load* - this command will install the module and create corresponding device files.

```console
[cs452@localhost device-driver]$ make
make -C /lib/modules/`uname -r`/build M=`pwd` modules
make[1]: Entering directory `/usr/src/kernels/3.10.0-1160.el7.x86_64'
  CC [M]  /home/cs452/device-driver/toyota.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/cs452/device-driver/toyota.mod.o
  LD [M]  /home/cs452/device-driver/toyota.ko
make[1]: Leaving directory `/usr/src/kernels/3.10.0-1160.el7.x86_64'
gcc    -c -o toyota-test1.o toyota-test1.c
gcc  -o toyota-test1 toyota-test1.o
gcc    -c -o toyota-test2.o toyota-test2.c
gcc  -o toyota-test2 toyota-test2.o
gcc    -c -o toyota-test3.o toyota-test3.c
gcc  -o toyota-test3 toyota-test3.o
gcc    -c -o toyota-test4.o toyota-test4.c
gcc  -o toyota-test4 toyota-test4.o
[cs452@localhost device-driver]$ sudo ./toyota_load 
[sudo] password for cs452: 
```

After the above, you can then run tests.

- When running *toyota-test1*, you are expected to get (exactly):

```console
[cs452@localhost device-driver]$ ./toyota-test1
Attempting to write to toyota device
Wrote 1000 bytes.
Read failed:

Attempting to write to toyota device
Wrote 1000 bytes.
Read failed:

Attempting to write to toyota device
Terminated
```

- When running *toyota-test2*, you are expected to get (exactly):

```console
[cs452@localhost device-driver]$ ./toyota-test2
wrote 5 bytes: bcabc
read 3 bytes: abc
wrote 13 bytes: toyotacorolla
read 7 bytes: oytacrl
wrote 11 bytes: toyotacamryla
read 7 bytes: otacmry
wrote 12 bytes: toyotatacomaa
read 6 bytes: oyatcm
```

- When running *toyota-test3*, you are expected to get (exactly):

```console
[cs452@localhost device-driver]$ ./toyota-test3
wrote 5 bytes: bcabc
read 6 bytes: abcabc
wrote 8 bytes: cbacdcbc
read 6 bytes: acdbac
wrote 11 bytes: bcbcbcababa
read 6 bytes: bcabca
wrote 86 bytes: tknvntfipavdqjiyslpdlokuymbutpynnxqekoktlqzrhoyvbewklzuamhwtqygsiakymyqwqiqtouynaiowwf
read 25 bytes: adjbpnxeklqrhovwzgsimtuyf
```

- When running *toyota-test4*, you are expected to get a huge number of "abc"s like this:

```console
[cs452@localhost device-driver]$ ./toyota-test4
wrote 5 bytes: bcabc
read 200000 bytes: abcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcab
... (omitted)
cabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcab
```

After all these tests, you should run *sudo ./toyota_unload*, which will remove the module and delete device files. 

```console
[cs452@localhost device-driver]$ sudo ./toyota_unload 
[sudo] password for cs452: 
```

## Submission

Due: 23:59pm, November 10th, 2022. Late submission will not be accepted/graded.

## Project Layout

All files necessary for compilation and testing need to be submitted, this includes source code files, header files, and Makefile. The structure of the submission folder should be the same as what was given to you.

## Grading Rubric (Undergraduate and Graduate)

- [10 pts] Compiling
  - Each compiler warning will result in a 3 point deduction.
  - You are not allowed to suppress warnings

- [70 pts] Main driver: supports read properly, writing (to device 1 and 2) acts like /dev/null, kill process writing to /dev/toyota3.
  - toyota-test1 produces expected results /10
  - toyota-test2 produces expected results /20
  - toyota-test3 produces expected results /20
  - toyota-test4 produces expected results /20

- [10 pts] Module can be installed and removed without crashing the system:
  - You won't get these points if your module doesn't implement any of the above functional requirements.

- [10 pts] Documentation:
  - README.md file (rename this current README file to README.orig and rename the README.template to README.md.)
  - You are required to fill in every section of the README template, missing 1 section will result in a 2-point deduction.

## Related Exercises

A major part of your read() function is removing duplicate letters from a string, and returning the smallest in lexicographical order result. This part is from the leetcode problem 316 - [Remove Duplicate Letters](https://leetcode.com/problems/remove-duplicate-letters/). You are highly recommended to solve this problem on leetcode first, and then port the code into the read function of your kernel driver.
