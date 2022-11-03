/*
 * the toyota char device driver.
 */

#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* for kmalloc() */
#include <linux/version.h> /* for kmalloc() */
#include <linux/fs.h>     /* everything... */
#include <linux/file.h>     /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/fcntl.h>        /* O_ACCMODE */
#include <linux/kmod.h>        /* for request_module */
#include <linux/init.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,12)
#include <linux/sched/signal.h>
#else
#include <linux/signal.h>
#endif
#include <linux/sched.h>
#include <linux/uaccess.h>

#include "toyota.h"        /* local definitions */

MODULE_AUTHOR("Jidong Xiao"); /* change this line to your name */
MODULE_LICENSE("GPL");

static int toyota_open (struct inode *inode, struct file *filp);
static int toyota_release (struct inode *inode, struct file *filp);
static ssize_t toyota_read (struct file *filp, char *buf, size_t count, loff_t *f_pos);
static ssize_t toyota_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos);

/* The different file operations.
 * Any member of this structure which we don't explicitly assign will be initialized to NULL by gcc. */
static struct file_operations toyota_fops = {
    .owner =      THIS_MODULE,
    .read =       toyota_read,
    .write =      toyota_write,
    .open =       toyota_open,
    .release =    toyota_release,
};

/*
 * open. if successful, return 0.
 */

static int toyota_open (struct inode *inode, struct file *filp){
    return 0;          /* success */
}

/*
 * close. if successful, return 0.
 */

static int toyota_release (struct inode *inode, struct file *filp){
    return 0;
}

/* write the string to the internal buffer for minor number 0;
 * pretends to ignore writes (like /dev/null) for minor numbers 1,2;
 * kill the calling process for minor number 3;
 * we assume applications will access our device sequentially, i.e., they do not access multiple devices concurrently.
 * if successful, return count - user wants to write "count" bytes into this device.
 */
static ssize_t toyota_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos){
	return count;
}

/* when read, we do not care the device minor number,
 * we process whatever is in the internal buffer, and return the processed string (maybe multiple times, as a stream) to user.
 * we assume applications will access our device sequentially, i.e., they do not access multiple devices concurrently.
 * if successful, return count - user wants to read "count" bytes from this device.
 */
static ssize_t toyota_read (struct file *filp, char *buf, size_t count, loff_t *f_pos){
    return count;
}

/*
 * module initialization. if successful, return 0.
 */

static int __init toyota_init(void){
	/*
	 * register your major, and accept a dynamic number.
	 */
    register_chrdev(0, "toyota", &toyota_fops);
	return 0;
}

/*
 * module exit. if successful, does not return anything.
 */

static void __exit toyota_exit(void){
}

module_init(toyota_init);
module_exit(toyota_exit);

/* vim: set ts=4: */
