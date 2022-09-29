# Overview

In this assignment, we will write a Linux kernel module called infiniti. This module implements a page fault handler for a 64-bit x86 system, which runs multiple level page tables. You should still use the cs452 VM which you used for your tesla and lexus, as loading and unloading the kernel module requires the root privilege.

## Learning Objectives

 - Get further familiar with the frequently used system call function: the ioctl() system call.
 - Understand the x86 64-bit multiple level page table structures.
 - Understand a key concept in virtual memory: address translation.

## Important Notes

You MUST build against the kernel version (3.10.0-1160.el7.x86\_64), which is the default version of the kernel installed on the cs452 VM.

While working on this assignment, please keep in mind, in current x86 based Linux systems, a virtual address has just 48 bits, not 64 bits. And your physical address has at most 52 bits.

## Book References

Read these chapters carefully in order to prepare yourself for this project:

 - Operating Systems: Three Easy Pieces: [Chapter 18: Introduction to Paging](https://pages.cs.wisc.edu/~remzi/OSTEP/vm-paging.pdf)
 - Operating Systems: Three Easy Pieces: [Chapter 19: Translation Lookaside Buffers](https://pages.cs.wisc.edu/~remzi/OSTEP/vm-tlbs.pdf)
 - Operating Systems: Three Easy Pieces: [Chapter 20: Advanced Page Tables](https://pages.cs.wisc.edu/~remzi/OSTEP/vm-smalltables.pdf)
 - Operating Systems: Three Easy Pieces: [Chapter 21: Swapping: Mechanisms](https://pages.cs.wisc.edu/~remzi/OSTEP/vm-beyondphys.pdf)

Other reference:
 - Intel Software Developer's Manual, we will call it the Intel SDM in this README.

# Specification

According to the textbook chapter 21: "**The act of accessing a page that is not in physical memory is commonly referred to as a page fault**". When a page fault happens, a kernel level function will be called to handle it, and this function is known as the page fault handler. In this assignment, you will develop a page fault handler in a Linux system. The Linux kernel has its own page fault handler. In this assignment, we do not intend to take over the default page fault handler, rather we try to maintain a separate handler, this handler will only handle memory pages mapped into a specific reserved memory region which the kernel will ignore. This memory region is in between virtual address 0x1000000000LL and virtual address 0x3000000000LL, and it is in the user space.

## The Starter Code

The starter code already provides you with the code for a kernel module called infiniti, and the code for a user-level library which interacts with the kernel module via ioctls. The kernel module implements a memory manager system, which manages the aforementioned reserved memory region. This is a region in the user space and the kernel will not use it. To install the module, run *make* and then *sudo insmod infiniti.ko*; to remove it, run *sudo rmmod infiniti*. Yes, in rmmod, whether or not you specify *ko* does not matter; but in insmod, you must have that *ko*.

What this module currently does is: create a file called /dev/infiniti, which provides an interface for applications to communicate with the kernel module. One way to communicate between applications and the kernel module, is applications issue ioctl() system calls to this device file (i.e., /dev/infiniti), and the kernel module will handle these ioctl commands. A list of commands are currently supported:

 - LAZY\_ALLOC: Applications call library function *infiniti_malloc*(), which sends this LAZY\_ALLOC command to the kernel module, so as to allocate memory.
 - LAZY\_FREE: Applications call library function *infiniti_free*(), which sends this LAZY\_FREE command to the kernel module, so as to free memory.
 <!-- - DUMP\_STATE: Applications call library function *infiniti_dump*(), which sends this DUMP\_STATE command to the kernel module, so as to dump the state of our reserved memory region to the kernel log file /var/log/messages.-->
 - PAGE\_FAULT: Applications call library functions *init_infiniti*(), which registers the application into our memory manager system. Applications managed by our system need to use *infiniti_malloc*() to allocate dynamic memory, and use *infiniti_free*() to free dynamic memory. Such applications need to have their own page fault handler, because we only allocate memory from the aforementioned reserved memory region, which is a memory region the kernel will not handle.

The starter code will manage the reserved memory region, but it will not map any virtual address into a physical address. Thus, when the application tries to call *infiniti_malloc*(), if memory in the reserved memory region is available, the malloc function will succeed, and a pointer will be returned, just like the regular malloc() function. However, because the virtual address pointed to by this pointer is not mapped into anywhere in the physical memory, any access to such an address will just fail. When that access fails, the kernel will deliver a signal to the process (i.e., the application), normally this signal will kill the process, but the process is configured (in *init_infiniti*())to intercept such signals and when such signals are received, the process will deliver a PAGE\_FAULT command to the kernel module, and now in this kernel module, your page fault handler *infiniti_do_page_fault*() will be called. In this handler function, you need to create the mapping, between this user space address, and a physical address. To achieve this, you call *get_zeroed_page*() to allocate physical memory, and then you update the page table so that that user space address is mapped to this physical address. After that, your *infiniti_do_page_fault*() will return, and the application will try to access that user space address again, and this time it will succeed - if your *infiniti_do_page_fault*() function has updated the page table correctly.

The starter code also includes a user-level library, which implements functions such as *init_infiniti*(), *infiniti_malloc*(), *infiniti_free*()<!--, *infiniti_dump*()-->. Several testing programs (infiniti-test[1-6].c) are also provided. The user-level library, as well as the test programs, are located in the **user** folder. Once you navigate into the **user** folder, you need to run *make* to compile these test programs, and at the same time the user-level library will be automatically compiled and linked into the resulted binary of the test programs.

## Functions You Need to Implement

The only file you should modify in this assignment is fault.c. You need to implement the following 2 functions in this file:

 - *infiniti_do_page_fault*(): This function will be called when the application triggers a page fault. In this function you should ask physical memory from the kernel and then update the page tables. The prototype of this function is:

```c
int infiniti_do_page_fault(struct infiniti_vm_area_struct *infiniti_vma, uintptr_t fault_addr, u32 error_code);
```
this function should return 0 if a page fault is handled successfully, and return -1 if not. The second parameter **fault_addr** is the user space address the application is trying to access. To handle the page fault, you need to update the page tables so that a mapping between the **fault_addr** and the physical address you allocated via *get_zeroed_page*() is created.

The first parameter **infiniti_vma** will be used once, and only once, at the very beginning of your page fault handler function, to check if this *fault_addr* is within the aforementioned reserved memory region or not. If not, then it is not your page fault handler's responsibility to handle this situation, therefore your handler should just return -1. A helper function called *is_valid_address*() is provided to determine if the *fault_addr* is within the reserved memory region or not, and you can use the function like this:

```c
if (is_valid_address(infiniti_vma, fault_addr)==0)
	return -1;
```

The third parameter **error_code** will also be used once, and only once, at the very beginning of your page fault handler function, to check if the application has permission to access this fault address or not, if not, then once again, your handler should just return -1. You can check that like this:

```c
if (error_code == SEGV_ACCERR)
	return -1;
```

 - *infiniti_free_pa*(): this function will be called when the application calls *infiniti_free*(). In this function you should give the physical memory back to the kernel and then update the page tables. The prototype of this function is:

```c
void infiniti_free_pa(uintptr_t user_addr);
```
 Before the application calls your *infiniti_free_pa*() (via *infiniti_free*()), this *user_addr* is mapped to some physical address - thanks to your *infiniti_do_page_fault*(). Now when the application calls your *infiniti_free_pa*() (via *infiniti_free*()), you should update the page tables so as to destroy the mapping. In other words, when the application calls *infiniti_free*(), the mapping will no longer exist, and the application should no longer be able to access that same physical address. 

## Provided Helper Functions

- *invlpg*(): The TLB cache stores recently used mappings between virtual memory pages and physical page frames, or as the textbook chapter 19 describes it: **A TLB is part of the chip’s memory-management unit (MMU), and is simply a hardware cache of popular virtual-to-physical address translations**. In other words, the TLB is a cache of the page table, representing only a subset of the page-table contents. The reason that TLB is just a subset of the page-table contents, is because cache size is very limited, we can not store all the mappings, but rather, we only store popular mappings. This requires us to constantly evict some not-so-popular mappings out of the TLB, and store some popular mappings into the TLB. This evicting act is done by the *invlpg* x86 instruction, which is short for "invalidate page". The function *invlpg*() is just a wrapper of this instruction. We call this function to invalidate the tlb entry for one specific page, and such behavior is what the textbook chapter refers to as **flush the TLB**. This function takes a virtual address, the processor determines the page that contains that virtual address and flushes all TLB entries for that page. In theory, you should call this function whenever you modify a page table entry, so that TLB does not store out of date mappings. But my experience shows, even if I do not call this function at all, my programs still exhibit correct behaviors, so it is my recommendation that you do not call this function for now. However, if your program does not behave as expected, especially if infiniti-test2 does not behave as expected, then that is the moment you need to consider to use this function in your code, especially in your *infiniti_free_pa*() function. This *invlpg*() function takes a virtual address as its parameter, and its prototype is like this:

```c
static inline void invlpg(uintptr_t page_addr);
```

- *get_cr3*(): we call this function to get the content of the cr3 register. This is how you call it:

```c
unsigned long cr3;
cr3 = get_cr3();
```

- *is_valid_address*(): already mentioned before, see above.

- *is_entire_table_free*(): This function's prototype is:

```c
int is_entire_table_free(unsigned long table);
```

You will use this function in your *infiniti_free_pa*() function. Keep reading this document, and you will see it says in step 6 of your *infiniti_free_pa*(), you need to "check if the entire page table is free". This function does exactly that for you. Just pass your table's virtual address to this function. For example,

```c
unsigned long page_table;
if(is_entire_table_free(page_table)){
	do something;
}else{
	do something else;
}
```

## Related Kernel APIs

I used the following APIs. 
 - *get_zeroed_page*()and *free_page*(). You call *get_zeroed_page*() to get a free memory page (filled with zeros) from the kernel, later on you call *free_page*() to give the memory back to the kernel. This is how you use *get_zeroed_pages*():

```c
uintptr_t kernel_addr = 0;
kernel_addr = (uintptr_t)get_zeroed_page(GFP_KERNEL);
if (!kernel_addr) {
	printk(KERN_INFO "failed to allocate one page\n");
	return -ENOMEM;
}
```

and then later this is how you use *free_page*().

```c
free_page(kernel_addr);
```

the address returned by *get_zeroed_page*() is a page aligned virtual address (in kernel space), which means its lowest 12 bits are 0, and its corresponding physical address also has its lowest 12 bits be 0. For example, if you add a printk statement to print the address represented by the above *kernel_addr* variable:

```c
printk(KERN_INFO "kernel address is %lx, and its physical address is %lx\n", kernel_addr, __pa(kernel_addr));
```

in your log, you will see some messages like this:

```console
kernel address is 0xffff880077313000, and its physical address is 0x77313000
kernel address is 0xffff880059eca000, and its physical address is 0x59eca000
kernel address is 0xffff880075faf000, and its physical address is 0x75faf000
```

 - *__va*() and *__pa*(). Given a physical address, *__va*() gives you its virtual address; given a virtual address, *__pa*() gives you its physical address. Of course, the virtual address gets involved here must be a kernel virtual address. For user virtual address, we still need to walk the page table to do the translation. Note, there are two underscore signs in *__va*(), and two underscore signs in *__pa*(). *__va*() returns a void \* type pointer, and therefore you may want to cast it, for example, to unsigned long. *__pa*() returns an unsigned long address, so you very likely do not need to cast.

## Expected Results

If you compile the starter code, install the default infiniti kernel module, and run the tests, you will get the following results:

```console
[cs452@localhost user]$ ./infiniti-test1
Segmentation fault (core dumped)
[cs452@localhost user]$ ./infiniti-test2
Segmentation fault (core dumped)
[cs452@localhost user]$ ./infiniti-test3
Segmentation fault (core dumped)
[cs452@localhost user]$ ./infiniti-test4
Segmentation fault (core dumped)
[cs452@localhost user]$ ./infiniti-test5
Segmentation fault (core dumped)
[cs452@localhost user]$ ./infiniti-test6
Segmentation fault (core dumped)
```

Once your implementation is complete, you install the infiniti kernel module, and run the tests, you should get the following results:

```console
[cs452@localhost user]$ ./infiniti-test1
Hello Boise!
[cs452@localhost user]$ ./infiniti-test2
Hello Boise!
Segmentation fault (core dumped)
[cs452@localhost user]$ ./infiniti-test3
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
buf is a
test
success
[cs452@localhost user]$ ./infiniti-test4
infiniti_malloced 2692 bytes and stored address 0x1000000000 at x[14]
infiniti_freed address 0x100002a000 of size 1136 in x[83]
infiniti_freed address 0x1000036000 of size 3845 in x[56]
infiniti_malloced 734 bytes and stored address 0x100003d000 at x[66]
infiniti_malloced 1393 bytes and stored address 0x100003e000 at x[57]
... (omitted)
infiniti_freed address 0x1000004000 of size 3560 in x[53]
infiniti_malloced 2417 bytes and stored address 0x1000042000 at x[45]
infiniti_freed address 0x1000019000 of size 1131 in x[60]
infiniti_malloced 300 bytes and stored address 0x1000043000 at x[26]
infiniti_freed address 0x1000041000 of size 3289 in x[42]
[cs452@localhost user]$ ./infiniti-test5
result ret[0] is 0
result ret[1] is 1
[cs452@localhost user]$ ./infiniti-test6
s1 is PAHNAPLSIIGYIR
```
Note that seg fault message showed when running infiniti-test2, is expected, and is intentionally showed. It happens because the test program tries to access a freed buffer. If yours does not show this seg fault message, it means your *infiniti_free_pa*() function is not implemented correctly. For example, if your *infiniti_free_pa*() function is completely empty, but your *infiniti_do_page_fault*() works correctly, then this is what you will see when running infiniti-test2.

```console
[cs452@localhost user]$ ./infiniti-test2
Hello Boise!
Hello Boise!
```

The logic here is, the test program allocates a buffer to store this message, and then the program prints this message; and then the program tries to free its memory, however, because your *infiniti_free_pa*() does nothing, the memory will not be freed, and the page table mappings are therefore still there, thus the program can still print this same message. A fully functioning system would tell the program this is not allowed, and would kill the program and show a seg fault message.

## Walk Through the 4-Level Page Tables

In this assignment, we only consider 4KB pages, i.e., each page is 4KB. By default, a 4-level page table structure is used in the Linux kernel running on 64-bit x86 platforms. In Intel's terminology, these 4-level tables are called:

 - PML4, or page map level 4; each entry in this table is called  a page map level 4 entry, or PML4E.
 - PDP Table, or page directory pointer table; each entry in this table is called a page directory pointer table entry, or PDPTE.
 - PD Table, or page directory table; each entry in this table is called a page directory table entry, or PDE.
 - PT, or page table; each entry in this table is called a page table entry, or PTE.

<!--The 4-level page tables are known as:

 - Page Global Directory (PGD)
 - Page Upper Directory (PUD).
 - Page Middle Directory (PMD).
 - Page Table Entry directory (PTE).-->

Each of these tables has 512 entries, and each entry is 8 bytes, and thus in total it's 512x8=4KB per table, which is one page.

Whenever we need to translate a virtual address to a physical address, we need to walk through all 4 tables. This whole process starts from the register *CR3* - this register always points to the starting physical address of the PML4 table. Remember, your physical address has at most 52 bits, thus *CR3*, which is a 64-bit register, has its bit 52-63 all 0s. In addition, all 4-level page tables are stored at page-aligned addresses, which means their starting physical address has it lowest 12 bits all 0s: because one page is 4KB, which is 2^12. Therefore, for a virtual machine whose total memory size is 2GB, the following is a typical value stored in *CR3*:

```console
0x77272000
```

Note that this address has its lowest 12 bits all 0s, and if we convert this address to binary, it will be: 0b 111 0111 0010 0111 0010 0000 0000 0000. From this binary number, we can see this address has 31 bits, which is reasonable when the total memory size is 2GB, because 2GB=2^31.

As we mentioned before, in current Linux systems running on x86 computers, each virtual address has 48 bits. And translation uses 9 bits at a time from the virtual address. To translate a virtual address to a physical address,

1. we get the physical address of the PML4 table from CR3 - the Intel SDM says, a 4-KByte naturally aligned PML4 table is located at the physical address specified in bits 51:12 of CR3; and use bit 47:39 of the virtual address to get the correct PML4E. (use bits 47:39 of the virtual address as bit 11:3 of the entry's offset, lowest 3 bits are all 0.)
2. we use the PML4E to get the physical address of the PDP table - the Intel SDM says, a 4-KByte naturally aligned page-directory-pointer table is located at the physical address specified in bits 51:12 of the PML4E; and use bit 38:30 of the virtual address to get the correct PDPTE. (use bits 38:30 of the virtual address as bit 11:3 of the entry's offset, lowest 3 bits are all 0.)
3. we use the PDPTE to get the physical address of the PD table - the Intel SDM says, a 4-KByte naturally aligned page directory is located at the physical address specified in bits 51:12 of the PDPTE; and use bit 29:21 of the virtual address to get the correct PDE. (use bits 29:21 of the virtual address as bit 11:3 of the entry's offset, lowest 3 bits are all 0.)
4. we use the PDE to get the physical address of the page table - the Intel SDM says, a 4-KByte naturally aligned page table is located at the physical address specified in bits 51:12 of the PDE; and use bit 20:12 of the virtual address to get the correct PTE. (use bits 20:12 of the virtual address as bit 11:3 of the entry's offset, lowest 3 bits are all 0.)
5. we use the PTE to get the page frame number (also known as physical frame number, or PFN) - the Intel SDM says, PTE's bit 51:12 gives us the page frame number, and we use bits 11:0 of the virtual address to get the offset.
6. we concatenate the page frame number with the offset to get the physical address.

Note: bit 47 to bit 39 are 9 bits; bit 38 to bit 30 are 9 bits, bit 29 to bit 21 are 9 bits, bit 20 to bit 12 are 9 bits. And 2^9 is 512, this matches with the fact that each of these tables has 512 entries: you only need 9 bits to index a table when the table has 512 entries in total.

Keep in mind that in this assignment, your job is NOT to translate a virtual address to a physical address, but rather is to update these tables so that CPU will be able to walk through the tables and translate a virtual address to a physical address. CPU will follow the above 6 steps to walk a 4-level page table structure, your job is just to make sure these page tables contain the correct information. More specifically, when your *infiniti_do_page_fault*() is called, you should update these page tables so that the CPU, via walking the page tables, can translate the fault address to the newly allocated physical address. And then when your *infiniti_free_pa*() is called, you should update these page tables again, so that the mapping between that user space address and its corresponding physical address will be destroyed.

To achieve the goals we just described, your *infiniti_do_page_fault*() should do the following:

1. find the PML4E, check its present bit, which is bit 0 of the entry, if it is 1, then move on to step 2; if it is 0, then we need to call *get_zeroed_page*() to allocate a page for the PDP table, and update the PML4E entry to reflect that this PDP table is now present, is writable, is a user page. This requires you to change the PML4E's bit 0, bit 1, bit 2 to 1. Also, store the bits 51:12 of the allocated page's physical address into the PML4E entry's bits 51:12. 
2. find the PDPTE, check its present bit, which is bit 0 of the entry, if it is 1, then move on to step 3; if it is 0, then we need to call *get_zeroed_page*() to allocate a page for the PD table, and update the PDPTE entry to reflect that this PD table is now present, is writable, is a user page. This requires you to change the PDPTE's bit 0, bit 1, bit 2 to 1. Also, store the bits 51:12 of the allocated page's physical address into the PDPTE entry's bit 51:12.
3. find the PDE, check its present bit, which is bit 0 of the entry, if it is 1, then move on to step 4; if it is 0, then we need to call *get_zeroed_page*() to allocate a page for the page table, and update the PDE entry to reflect that this page table is now present, is writable, is a user page. This requires you to change the PDE's bit 0, bit 1, bit 2 to 1. Also, store the bits 51:12 of the allocated page's physical address into the PDPTE entry's bit 51:12.
4. find the PTE, check its present bit, which is bit 0 of the entry, if it is 1, then move on to step 5; if it is 0, then we need to call *get_zeroed_page*() to allocate a page for the physical page, and update the PTE entry to reflect that this physical page is now present, is writable, is a user page. This requires you to change the PTE's bit 0, bit 1, bit 2 to 1. Also, store the bits 51:12 of the allocated page's physical address into the PTE entry's bits 51:12. 
5. return 0.

And your *infiniti_free_pa*(), which takes *uintptr_t user_addr* as its parameter, should do the following:

1. find the PML4E, check its present bit, which is bit 0 of the entry, if it is 0, then there is nothing you need to free - there is no valid mapping, so just return; if it is 1, then move on to step 2.
2. find the PDPTE, check its present bit, which is bit 0 of the entry, if it is 0, then there is nothing you need to free - there is no valid mapping, so just return; if it is 1, then move on to step 3.
3. find the PDE, check its present bit, which is bit 0 of the entry, if it is 0, then there is nothing you need to free - there is no valid mapping, so just return; if it is 1, then move on to step 4.
4. find the PTE, check its present bit, which is bit 0 of the entry, if it is 0, then there is nothing you need to free - there is no valid mapping, so just return; if it is 1, then move on to step 5.
5. now that you are here, you actually have just "accidentally" walked the whole page tables, and now the PTE contains the physical frame number of the page the application wants to free, so get the offset from *user_addr*, and concatenate the physical frame number with the offset, will give you the physical address you should free, convert this physical address to its kernel space address (via *__va*()), and call *free_page*() to free it.
6. now that the physical memory page is freed, you need to update the page tables to destroy the mapping. following steps are needed:
   - set the entire PTE entry to 0. and then, check if the entire page table is free:
     - if in this page table, every entry's present bit is 0, then we can say this page table is not used at all, and therefore its memory should be freed. call *free_page*() to free this page table. 
     - otherwise - at least one entry's present bit is 1, then we should not free this table, therefore we just return.
   - set the entire PDE entry to 0. and then, check if the entire PD table is free:
     - if in this PD table, every entry's present bit is 0, then we can say this PD table is not used at all, and therefore its memory should be freed. call *free_page*() to free this PD table.
     - otherwise - at least one entry's present bit is 1, then we should not free this table, therefore we just return.
   - set the entire PDPTE entry to 0. and then, check if the entire PDP table is free:
     - if in this PDP table, every entry's present bit is 0, then we can say this PDP table is not used at all, and therefore its memory should be freed. call *free_page*() to free this PDP table.
     - otherwise - at least one entry's present bit is 1, then we should not free this table, therefore we just return.
   - set the entire PML4E entry to 0. and then, check if the entire PML4 table is free:
     - if in this PML4 table, every entry's present bit is 0, then we can say this PML4 table is not used at all, and therefore its memory should be freed. call *free_page*() to free this PML4 table.
     - otherwise - at least one entry's present bit is 1, then we should not free this table, therefore we just return.

# Submission

Due: 23:59pm, October 6th, 2022. Late submission will not be accepted/graded.

# Grading Rubric (Undergraduate and Graduate)
Grade: /100

- [ 70 pts] Functional Requirements: page faults handled correctly, memory correctly freed.
    - infiniti-test1 runs and ends smoothly: message printed correctly, no program crash, no kernel crash. /10
    - infiniti-test2 runs and ends with a seg fault: message printed, then seg fault, but no kernel crash. /20
    - infiniti-test3 runs and ends smoothly: messages printed correctly, no program crash, no kernel crash. /10
    - infiniti-test4 runs to end without crash. /10
    - infiniti-test5 runs and ends smoothly, messages printed correctly, no program crash, no kernel crash. /10
    - infiniti-test6 runs and ends smoothly, messages printed correctly, no program crash, no kernel crash. /10

- [10 pts] Module can be installed and removed without crashing the system: 
   - You won't get these points if your module doesn't implement any of the above functional requirements.

- [10 pts] Compiling:
   - Each compiler warning will result in a 3-point deduction.
   - You are not allowed to suppress warnings. (you won't get these points if your module doesn't implement any of the above functional requirements.)

- [10 pts] Documentation:
   - README.md file (rename this current README file to README.orig and rename the README.template to README.md.)
   - You are required to fill in every section of the README template, missing 1 section will result in a 2-point deduction.
