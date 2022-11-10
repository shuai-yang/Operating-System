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
int major_num;
int minor_num;

char* removedup(char* str);

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
    int err;
	if(count == 0) return count;
	if(minor_num == 0){
        toyota_data = kmalloc(count + 1, GFP_KERNEL);
        toyota_data[count] = '\0';
		err = copy_from_user(toyota_data, buf, count);
    }else if (minor_num == 3){
        kill_pid(task_pid(current), SIGTERM, 1);
    }
	return count;
}

/* when read, we do not care the device minor number,
 * we process whatever is in the internal buffer, and return the processed string (maybe multiple times, as a stream) to user.
 * we assume applications will access our device sequentially, i.e., they do not access multiple devices concurrently.
 * if successful, return count - user wants to read "count" bytes from this device.
 */
static ssize_t toyota_read (struct file *filp, char *buf, size_t count, loff_t *f_pos){
    char* s1;
    char* s2 = "";
	int i, err;
	if(toyota_data == NULL) {
        printk(KERN_ALERT "Failed alloc toyota_data\n");
        return -1;
    }

    s1 = (char*)removedup(toyota_data);
	//printk("Result string S1 is %s\n", s1);
	s2 = (char*)kmalloc(count, GFP_KERNEL);
    memset(s2, '\0', count);
	for(i = 0; i < count/strlen(s1); i++){
		s2 = strcat(s2, s1);
	}
    //printk("S2 after for loop is %s\n", s2);
	s2 = strncat(s2, s1, count%strlen(s1));	
	//printk("Final S2 is %s\n", s2);    

	err = copy_to_user(buf, s2, count);
    kfree(toyota_data);
	kfree(s2);
    toyota_data = NULL;
    s2 = NULL;
    return count;
}

/*
 * module initialization. if successful, return 0.
 */

static int __init toyota_init(void){
	/*
	 * register your major, and accept a dynamic number.
	 */
    major_num = register_chrdev(0, "toyota", &toyota_fops);
    if(major_num < 0){
        printk("toyota can't register major number\n");
        return -1;
    }
	return 0;
}

/*
 * module exit. if successful, does not return anything.
 */

static void __exit toyota_exit(void){
    /* reverse the effect of register_chrdev(). */
    unregister_chrdev(major_num, "toyota"); 
}

/*
 * removedup()
 */

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

    for (i = 0; i < length; i++){
        counts[str[i] - 'a'] ++;
    }

    stack = (char *)kmalloc((length + 1) * sizeof(char), GFP_KERNEL);

    for (i = 0; i < length; i++) {
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
    stack[top] = '\0';
    return stack;
}

module_init(toyota_init);
module_exit(toyota_exit);

/* vim: set ts=4: */
