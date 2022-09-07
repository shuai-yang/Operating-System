#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/uaccess.h> /* for copy_to_user and copy_from_user */
#include <linux/random.h> /* for random number generator */
#include <linux/miscdevice.h> /* for misc devices */
#include <linux/pid.h> /* for pid_task */
#include "lexus.h"

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Shuai Yang"); /* Note: change this line to your name! */
MODULE_DESCRIPTION("CS452 Lexus");

#define DEBUG 1 /* Note: uncomment this line so you can see debug messages in /var/log/messages, or when you run dmesg */

/* this integers tracks how many number of tickets we have in total */
unsigned long nTickets = 0;

/* this variable is the timer, which helps us to call your lexus_schedule() every 200 millisecond. 
 * you don't need to use this variable in the four functions you implement. */
static struct timer_list dispatch_timer;

/* each lexus_task_struct represents one process in the lottery scheduling system */
struct lexus_task_struct {
        struct list_head list; /* Kernel's list structure */
        struct task_struct* task;
        unsigned long pid;
        unsigned long tickets;
        task_state state;
};

/* use this global variable to track all registered tasks, by adding into its list */
static struct lexus_task_struct lexus_task_struct;

/* the currently running lexus task */
static struct lexus_task_struct *lexus_current;

/* spinlock to protect the linked list, and the global variables defined in this kernel module */
static spinlock_t lexus_lock;

/* dispatch kernel thread */
static struct task_struct *dispatch_kthread;

/* given a pid, returns its task_struct */
struct task_struct* find_task_by_pid(unsigned int pid)
{
   struct task_struct* task;
   rcu_read_lock();
   task=pid_task(find_vpid(pid), PIDTYPE_PID);
   rcu_read_unlock();

   return task;
}

/* free all the lexus_task_struct instances: delete its list, 
 * and free its memory allocated via kmalloc().
 * this function is called in lexus_exit(), you won't use this function. */
void free_lexus_list(void) {
    struct list_head *p, *n;
    struct lexus_task_struct *tmp;
    unsigned long flags;
    spin_lock_irqsave(&lexus_lock, flags);
    /* You can just treat this list_for_each_safe() as a for loop:
     * for (p = lexus_task_struct.list->next; p != lexus_task_struct.list; p = p->next), 
     * you can ignore n, it's a temporary pointer used inside the loop. */
    list_for_each_safe(p, n, &lexus_task_struct.list) {
        tmp = list_entry(p, struct lexus_task_struct, list);
        list_del(p);
        kfree(tmp);
    }
    spin_unlock_irqrestore(&lexus_lock, flags);
}

/* register a process into the lottery scheduling system */
void lexus_register(struct lottery_struct lottery){
	unsigned long flags;
	struct lexus_task_struct *node;
	printk("lexus_register() get called");
	node = kmalloc(sizeof(struct lexus_task_struct*), GFP_KERNEL);
	node->list = lexus_task_struct.list;
	node->task = find_task_by_pid(lottery.pid);
	node->pid = lottery.pid;
	node->tickets = lottery.tickets;
	node->state = READY;
	spin_lock_irqsave(&lexus_lock, flags);
	list_add(&(node->list), &(lexus_task_struct.list));	
	nTickets += node->tickets; 
	spin_unlock_irqrestore(&lexus_lock, flags);
}

/* unregister a process from the lottery scheduling system */
void lexus_unregister(struct lottery_struct lottery){
	struct list_head *p, *n;
	struct lexus_task_struct *node;
	unsigned long flags;
	printk("lexus_unregister() get called");
	spin_lock_irqsave(&lexus_lock, flags);
	list_for_each_safe(p, n, &lexus_task_struct.list){
		/*node points to each lexus_task_struct in the list.*/
		node = list_entry(p, struct lexus_task_struct, list);
		if(node == (struct lexus_task_struct*)find_task_by_pid(lottery.pid)){
			lexus_current = NULL;
			nTickets -= node->tickets;
			list_del(p);
			break;
		}	
	}
	spin_unlock_irqrestore(&lexus_lock, flags);
	kfree(node);
}


/* executes a context switch: pick a task and dispatch it to the Linux CFS scheduler */
int lexus_schedule(void *data)
{
	while(!kthread_should_stop()){
		printk(KERN_ERR "hello scheduler\n");

		if(winner is not current){
			wake_up_process();
		}
	}	
	return 0;
}

/* handle ioctl system calls */
static long lexus_dev_ioctl(struct file *filp, unsigned int ioctl, unsigned long arg)
{
	struct lottery_struct lottery_info;
	if(ioctl == LEXUS_REGISTER){
		if(copy_from_user(&lottery_info, (char*)arg, sizeof(struct lottery_struct)) != 0){
			return -EFAULT;
		}		
		lexus_register(lottery_info);
	}
	if(ioctl == LEXUS_UNREGISTER){
		if(copy_from_user(&lottery_info, (char*)arg, sizeof(struct lottery_struct)) != 0){
			return -EFAULT;
		}
		lexus_unregister(lottery_info);
	}
	return 0;
}

/* gets called when the timer goes off, we then reset the timer so as to make sure
 * this function gets called periodically - every 200 milliseconds. */
void dispatch_timer_callback(unsigned long data)
{
    #ifdef DEBUG
    printk("Timer\n" );
    #endif

    /* setup timer interval to 200 msecs */
    mod_timer(&dispatch_timer, jiffies + msecs_to_jiffies(200));
    if(nTickets == 0) // nTickets being zero suggests there is no registered processes.
        return;

    #ifdef DEBUG
    printk("wake up dispatch kthread...\n" );
    #endif

    /* wake up the lottery scheduling kthread */
    wake_up_process(dispatch_kthread);
}

static const struct file_operations lexus_chardev_ops = {
	.owner          = THIS_MODULE,
	.unlocked_ioctl = lexus_dev_ioctl,
	.compat_ioctl   = lexus_dev_ioctl,
	.llseek         = noop_llseek,
};

/* once we call misc_reigster(lexus_dev), a file called /dev/lexus will be created.
 * any ioctl commands sent to this device will be handled by lexus_dev.fops.unlocked_ioctl, 
 * which is lexus_dev_ioctl(). */
static struct miscdevice lexus_dev = {
	.minor = LEXUS_MINOR,
	.name = "lexus",
	.fops = &lexus_chardev_ops,
	.mode = 0666,
};

/* called when module is loaded */
int __init lexus_init(void)
{
	int r;

	#ifdef DEBUG
	printk("<1> lexus: loading.\n");
	#endif

	/* creating the device file /dev/lexus */
	r = misc_register(&lexus_dev);
	if (r) {
		printk(KERN_ERR "lexus: misc device register failed\n");
		return r;
	}

	/* the lexus version of "current" - in Linux kernel, the global variable "current" points to the currently running process - its task_struct */
	lexus_current = NULL;

	/* initialize the list_head to be empty */
	INIT_LIST_HEAD(&lexus_task_struct.list);

	/* a kernel thread named lexus_dispatch will be running at the background, which calls lexus_schedule().
 	 * We don't need to pass any parameter lexus_schedule(), thus here the 2nd parameter is NULL. */
	dispatch_kthread = kthread_create(lexus_schedule, NULL, "lexus_dispatch");

	/* initialize the spin lock */
	spin_lock_init(&lexus_lock);

	/* setup your timer to call dispatch_timer_callback */
	setup_timer(&dispatch_timer, dispatch_timer_callback, 0);
	/* setup timer interval to 200 msecs */
	mod_timer(&dispatch_timer, jiffies + msecs_to_jiffies(200));

	#ifdef DEBUG
	printk("<1> lexus: loaded.\n");
	#endif

	return 0;
}

/* called when module is unloaded */
void __exit lexus_exit(void)
{
	#ifdef DEBUG
	printk("<1> lexus: unloading.\n");
	#endif

	/* now it's time to exit the dispatch kthread, we wake it up so that it runs and checks the stop flag at the beginning of the while loop */
	wake_up_process(dispatch_kthread);
	kthread_stop(dispatch_kthread);

	/* remove kernel timer when unloading module */
	del_timer(&dispatch_timer);

	/* free the memory allocated for each lexus_task_struct */
	free_lexus_list();

	/* removing the device file /dev/lexus */
	misc_deregister(&lexus_dev);

	#ifdef DEBUG
	printk("<1> lexus: unloaded.\n");
	#endif
}

// register init and exit funtions
module_init(lexus_init);
module_exit(lexus_exit);

/* vim: set ts=4: */
