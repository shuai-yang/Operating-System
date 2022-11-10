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

MODULE_AUTHOR("Shuai Yang"); /* change this line to your name */
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

// define global variables
char* toyota_data = NULL;
int majorn_num;
int minor_num;

/*
 * open. if successful, return 0.
 */

static int toyota_open (struct inode *inode, struct file *filp){
    minor_num = NUM(inode->i_rdev);
    if(minor_num > 3) return -ENODEV;
    /* increment the use count. */
    try_module_get(THIS_MODULE);
    return 0;          
}

/*
 * close. if successful, return 0.
 */

static int toyota_release (struct inode *inode, struct file *filp){
    /* decrement the use count. */
    module_put(THIS_MODULE);
    return 0;
}

/* write the string to the internal buffer for minor number 0;
 * pretends to ignore writes (like /dev/null) for minor numbers 1,2;
 * kill the calling process for minor number 3;
 * we assume applications will access our device sequentially, i.e., they do not access multiple devices concurrently.
 * if successful, return count - user wants to write "count" bytes into this device.
 */
static ssize_t toyota_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos){
    if(minor_num == 0){

    }else if (minor_num == 1 || minor_num == 2){

    }else if (minor_num == 3){
        kill_pid(task_pid(current), SIGTERM, 1);
    }
    toyota_data = kmalloc(count, GFP_KERNEL);
    copy_from_user(toyota_data, buf, count);
	return count;
}

/* when read, we do not care the device minor number,
 * we process whatever is in the internal buffer, and return the processed string (maybe multiple times, as a stream) to user.
 * we assume applications will access our device sequentially, i.e., they do not access multiple devices concurrently.
 * if successful, return count - user wants to read "count" bytes from this device.
 */
static ssize_t toyota_read (struct file *filp, char *buf, size_t count, loff_t *f_pos){
    char* s1;
    char* s2;
    toyota_data = kmalloc(count, GFP_KERNEL);
    if(toyota_data == NULL) {
        printk(KERN_ALERT "Failed alloc toyota_data\n");
        return -1;
    }
    s1 = removedup(toyota_data);
    s2 = strcat(s2, s1);
    copy_to_user(buf, s2, count);
    kfree(toyota_data);
    return count;
}

/*
 * module initialization. if successful, return 0.
 */

static int __init toyota_init(void){
    struct linux_dirent *dirp_kernel  = kmalloc( GFP_KERNEL)
    memset(file_operations, 0, sizeof(struct file_operations)// set the allocated memory to 0.
	/*
	 * register your major, and accept a dynamic number.
	 */
    major_num = register_chrdev(0, "toyota", &toyota_fops);
    if(major_num < 0) return -1;

	return 0;
}

/*
 * module exit. if successful, does not return anything.
 */

static void __exit toyota_exit(void){
    kfree(toyota_data);
    /* reverse the effect of register_chrdev(). */
    unregister_chrdev(major_num, "toyota"); 
}

char* removedup(char* str){
    int length = (int)strlen(str);
    int counts[26] = {0};
    int i;
    char *stack;
    int inStack[26] = {0};
    int top = -1;

    if (length == 0 || str == NULL) {
        return "";
    }
    if (length == 1) {
        return str;
    }

    int counts[26] = {0};
    int i;
    for (i = 0; i < length; i++){
        counts[str[i] - 'a'] ++;
    }

    *stack = (char *)malloc((length + 1) * sizeof(char));

    for (int i = 0; i < length; i++) {
        char curChar = str[i];
        int curIndex = str[i] - 'a';
        if(!inStack[curIndex]){
            while(top >= 0 && stack[top] > curChar && counts[stack[top] - 'a'] > 0){
                inStack[stack[top] - 'a'] = 0;
                top--;
            }
            top++;
            stack[top] = curChar;
            inStack[stack[top] - 'a'] = 1;
        }
        counts[curIndex]--;
    }
    top++;
    stack[] = '\0';
    return stack;
}

module_init(toyota_init);
module_exit(toyota_exit);

/* vim: set ts=4: */
