/* original author: Jack Lange
 * */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include "infiniti.h"
#include "fault.h"

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Jidong Xiao"); /* Note: change this line to your name! */
MODULE_DESCRIPTION("CS452 Infiniti");

#define PAGE_POWER 12
#define PAGE_ADDR(x) (((x) >> PAGE_POWER) << PAGE_POWER)
#define DEBUG 1

/* when user testing program opens /dev/infiniti, this function gets called by infiniti_open(),
 * which initializes a list head represented by infiniti_vma->list,
 * but both infiniti_vma and node are local pointers. 
 * in order to access these two later, here we return infiniti_vma and in infiniti_open(),
 * this return value is assigned to flip->private_data, so
 * later on when we need to access infiniti_vma and node, we
 * just need to access this flip->private_data. We can see this in infiniti_ioctl().
 * this is how we pass information from infiniti_open() to infiniti_ioctl(). */
struct infiniti_vm_area_struct * infiniti_init_process(void) {
	/* allocate virtual memory for two struct struct infiniti_vm_area_struct instances.
	 * infiniti_vma to represent the list, where as node just represents the initial big node we have.
	 * it should be noted that infiniti_vma will not be used when traversing the list, which means,
	 * if we only have infiniti_vma, but don't have any nodes on this list, our list will be treated as an empty list, i.e.,
	 * function list_for_each_safe() will just skip the loop and not do anything. */
	struct infiniti_vm_area_struct *infiniti_vma = (struct infiniti_vm_area_struct *)kmalloc(sizeof(struct infiniti_vm_area_struct), GFP_KERNEL);
	struct infiniti_vm_area_struct *node = (struct infiniti_vm_area_struct *) kmalloc(sizeof(struct infiniti_vm_area_struct), GFP_KERNEL);
    printk(KERN_INFO "process initialization...\n");
	INIT_LIST_HEAD(&(infiniti_vma->list));
	node->status = FREE;
	/* in this project, we assume this region of the virtual address space will not be used, and thus we use it. 
	 * remember, this memory region is a user-space region; thus all the memory regions on our list is user-space memory regions. */
	node->num_pages = ((INFINITI_MEM_REGION_END - INFINITI_MEM_REGION_START) >> PAGE_POWER); // PAGE_POWER is 12, 2^12=4KB.
	node->start = INFINITI_MEM_REGION_START;

	list_add(&(node->list), &(infiniti_vma->list));
    return infiniti_vma;

}

void infiniti_deinit_process(struct infiniti_vm_area_struct *infiniti_vma) {
	struct list_head *pos, *next;
	struct infiniti_vm_area_struct *node;
    int i;
    printk(KERN_INFO "process de-initialization...\n");
	list_for_each_safe(pos, next, &(infiniti_vma->list)){
		node = list_entry(pos, struct infiniti_vm_area_struct, list);
		/* we free the physical memory, only if this node is reserved. */
		if(node->status == RESERVED) {
			// each node may contain multiple pages, thus we release one page after another. 
        	for(i = 0; i < node->num_pages; i++){
            	infiniti_free_pa(node->start + (4096*i));	// start is a virtual address, but this is a user-space va.
        	}
		}
		list_del(pos);
		kfree(node);
	}
	/* infiniti_vma is per process, we allocate memory when the process calls init_infiniti(),
	 * which opens /dev/infiniti, which leads to a call to infiniti_open(), which calls infiniti_init_process(), which
	 * asks memory from the kernel for an infiniti_vma pointer; now in infiniti_deinit_process(), we free that memory. */
	kfree(infiniti_vma);
}

/* called by infiniti_ioctl() in case of LAZY_ALLOC. allocate num_pages pages to the user. */
uintptr_t infiniti_alloc_vspace(struct infiniti_vm_area_struct *infiniti_vma, u64 num_pages) {
	struct list_head *pos, *next;
	struct infiniti_vm_area_struct *node, *new_node;
    printk(KERN_INFO "memory allocation\n");
	node = NULL;
	/* traverse infiniti_vma->list, find one whose status is FREE. */
	list_for_each_safe(pos, next, &(infiniti_vma->list)) {
		node = list_entry(pos, struct infiniti_vm_area_struct, list);
		if(node->status == FREE && node->num_pages >= num_pages){
			break;
		}
	}
	/* if no nodes is free and has a size larger than the requested size, then return -1. */
	if(node == NULL){
		return -1;
	}

	printk(KERN_INFO "node to break apart: 0x%p\n", (void *)node->start);
	node->status = RESERVED;
	/* if it's an exact match */
	if(node->num_pages == num_pages){
		return node->start;
	}

	/* otherwise, we need to split. */
	new_node = (struct infiniti_vm_area_struct *)kmalloc(sizeof(struct infiniti_vm_area_struct), GFP_KERNEL);
	/* add new_node to infiniti_vma list: when allocating, we don't remove any nodes,
	 * we add one more node, and update the old node's size. thus for the two nodes: one is allocated, one is free. 
	 * the one marked as allocated will be returned to the user. 
	 * note: when allocating one new node, we initialize its size, start, and status. */
	list_add(&(new_node->list), &(infiniti_vma->list));
	new_node->num_pages = node->num_pages - num_pages;
	new_node->start = node->start + (num_pages << PAGE_POWER); // the first num_pages are allocated, we keep the remaining.
	new_node->status = FREE;

	node->num_pages = num_pages;
	/* so basically we add new_node to the list and keep both new_node and node,
	 * but we return node->start to the caller. note that the size of node is deducted,
	 * and we keep the remainder in new_node. */
	return node->start;
}

void infiniti_dump_vspace(struct infiniti_vm_area_struct *infiniti_vma) {
	struct list_head *pos, *next;
	struct infiniti_vm_area_struct *node;
	int i=0;
	printk("-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n");
	printk("-- -- -- -- --virtual memory dump-- -- -- -- --\n");
	printk("-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n");

	list_for_each_safe(pos, next, &(infiniti_vma->list)) {
		node = list_entry(pos, struct infiniti_vm_area_struct, list);
		printk("node %d: status = %ld start = %lx num_pages = %lu \n", i, node->status, node->start, node->num_pages);
		i++;
	}
	return;
}

/* take a user_addr, free its node - the user_addr must point to the starting address of a node.  */
void infiniti_free_vspace(struct infiniti_vm_area_struct *infiniti_vma, uintptr_t user_addr) {
	/* be careful, both *n and *next_node represents the word "next", and can easily get confused. */
	struct list_head *pos, *n;
	struct infiniti_vm_area_struct *node, *tmp, *next_node, *prev_node;
    int i;
    printk(KERN_INFO "free virtual memory\n");
	node = NULL;
	list_for_each_safe(pos, n, &(infiniti_vma->list)) {
		tmp = list_entry(pos, struct infiniti_vm_area_struct, list);
		/* the address they return must be the start address of some node..., which makes sense,
		 * given that our infiniti_alloc_vspace() only return the start address of some node. */
		if(tmp->start == user_addr){
		    node = tmp;
        }
	}
	/* no match */
	if(node == NULL){
		return;
	}
    for( i = 0; i < node->num_pages; i++){
        infiniti_free_pa(node->start + (i * 4096)); // start is a virtual address, but this is a user-space va.

    }
	node->status = FREE;
    for(i = 0; i < node->num_pages; i++){
        invlpg(node->start + (i * 4096));
    }

	/* coalescing. list_entry — get the struct for this entry. 
	 * the following two lines thus get the struct for the next entry, and the struct for the prev entry. */
	next_node = list_entry(node->list.next, struct infiniti_vm_area_struct, list);
	prev_node = list_entry(node->list.prev, struct infiniti_vm_area_struct, list);

	/* we don't delete this one, rather we delete the next one, and add its size to our size. */
	if(next_node->start != user_addr && next_node->status == FREE){
		node->num_pages = node->num_pages + next_node->num_pages;
		list_del(node->list.next);
		kfree(next_node);
	}
	/* we delete this one, but first we add its size to the size of the previous one. */
	if(prev_node->start != user_addr && prev_node->status == FREE){
		prev_node->num_pages = prev_node->num_pages + node->num_pages;
		/* list_del takes a struct list_head pointer as its parameter, i.e., void list_del (struct list_head * entry); */
		list_del(&(node->list));
		kfree(node);
	}
    return;
}

struct class * infiniti_class = NULL;
static struct cdev ctrl_dev;
static int major_num = 0;

static long infiniti_ioctl(struct file * filp,
			 unsigned int ioctl, unsigned long arg) {
    void __user * argp = (void __user *)arg;

    switch (ioctl) {
	/* allocate virtual memory, because ADD Memory is allocating physical memory. user/infiniti.c doesn't issue this command, user/test.c does. */
	case LAZY_ALLOC: {
	    struct alloc_request req; // struct alloc_request has two fields, size and address, the user gives us the size and we give the user the address.
		/* this is set when the user opens the device, infiniti_open sets this one. 
 		 * FIXME: shall this really be set when the user is just openning the device file? especially if they just want to send the ADD_MEMORY command.
 		 * */
	    struct infiniti_vm_area_struct *infiniti_vma = filp->private_data;
	    u64 page_size = 0;
	    u64 num_pages = 0;

    	printk("infiniti ioctl: LAZY_ALLOC\n");
	    memset(&req, 0, sizeof(struct alloc_request)); // it looks like this is set to 0 here as well as in user/harness.c, before the user issues the LAZY_ALLOC command.

	    if (copy_from_user(&req, argp, sizeof(struct alloc_request))) {
		printk(KERN_ERR "error copying allocation request from user space\n");
		return -EFAULT;
	    }

		/* when the user calls infiniti_malloc(10), we will see this message in the kernel log: "requested allocation of 10 bytes". */
	    printk(KERN_INFO "requested allocation of %llu bytes\n", req.size);

		/* the user asks for bytes, we give the user pages; even if req.size is 1 byte, i.e., they ask for 1 byte, we give them one page. */
		/* PAGE_SIZE is 4096, PAGE_SHIFT is 12, when req.size is 1, the following 2 lines gives us num_pages 1. */
	    page_size = (req.size + (PAGE_SIZE - 1)) & (~(PAGE_SIZE - 1));
	    num_pages = page_size >> PAGE_SHIFT;

		/* this is the actual function which does virtual memory allocation. */
	    req.addr = infiniti_alloc_vspace(infiniti_vma, num_pages);

	    if (req.addr == 0) {
		printk(KERN_ERR "error: could not allocate virtual address region\n");
		return 0;
	    }

	    if (copy_to_user(argp, &req, sizeof(struct alloc_request))) {
		printk(KERN_ERR "error copying allocation request to user space\n");
		return -EFAULT;
	    }

	    break;
	}

	/* when user calls infiniti_free(), it sends LAZY_FREE command to the kernel, we free the virtual memory - the entire virtual memory managed by us. */
	case LAZY_FREE: {
	    uintptr_t addr = (uintptr_t)arg;
	    struct infiniti_vm_area_struct *infiniti_vma = filp->private_data;

    	printk("infiniti ioctl: LAZY_FREE\n");
	    infiniti_free_vspace(infiniti_vma, addr);

	    break;

	}

	case DUMP_STATE: {
	    struct infiniti_vm_area_struct *infiniti_vma = filp->private_data;

    	printk("infiniti ioctl: LAZY_DUMP_STATE\n");
	    infiniti_dump_vspace(infiniti_vma);
	    break;
	}

	case PAGE_FAULT: {
	    struct page_fault fault;
	    struct infiniti_vm_area_struct *infiniti_vma = filp->private_data;

    	printk("infiniti ioctl: PAGE_FAULT\n");
	    memset(&fault, 0, sizeof(struct page_fault));

	    if (copy_from_user(&fault, argp, sizeof(struct page_fault))) {
			printk(KERN_ERR "error copying page fault info from user space\n");
			return -EFAULT;
	    }

		/* the process will truly be killed if this ioctl command returns 0, infiniti_page_fault()
		 * itself should return 0 if the fault is handled smoothly. otherwise it returns -1.  */
	    if (infiniti_do_page_fault(infiniti_vma, (uintptr_t)fault.fault_addr, (u32)fault.error_code) != 0) {
			printk(KERN_ERR "error handling page fault for addr:%p (error=%d)\n", (void *)fault.fault_addr, fault.error_code);
			return 1;
	    }

	    // 0 == success
	    return 0;
	}

	case INVALIDATE_PAGE: {
	    uintptr_t addr = (uintptr_t)arg;
    	printk("infiniti ioctl: INVALIDATE_PAGE\n");
	    invlpg(PAGE_ADDR(addr));
	    break;
	}

	default:
	    printk("Unhandled ioctl (%d)\n", ioctl);
	    break;
    }

    return 0;

}

/* whenever /dev/infiniti is opened, we call infiniti_init_process().
 * and put the return value in private_data; and in this project,
 * the one who opens /dev/infiniti is the user testing program, who calls
 * init_infiniti(), who calls open() to open /dev/infiniti. */
static int infiniti_open(struct inode * inode, struct file * filp) {


    printk(KERN_INFO "openning /dev/infiniti...\n");
    filp->private_data = infiniti_init_process();

    return 0;
}

/* FIXME: in the user program, they eventually should call close(fd) to close the file 
 * /dev/infiniti, that will lead to a call to this release function which releases all the resource.  */
static int infiniti_release(struct inode * inode, struct file * filp) {
    struct infiniti_vm_area_struct *infiniti_vma = filp->private_data;

    // garbage collect
    infiniti_deinit_process(infiniti_vma);
    printk(KERN_INFO "closing /dev/infiniti...\n");

    return 0;
}


static struct file_operations ctrl_fops = {
    .owner = THIS_MODULE,
    .open = infiniti_open,
    .release = infiniti_release,
    .unlocked_ioctl = infiniti_ioctl,
    .compat_ioctl = infiniti_ioctl
};

#define DEV_CLASS_MODE ((umode_t)(S_IRUGO|S_IWUGO))

static char *infiniti_class_devnode(struct device *dev, umode_t *mode)
{
    if (mode != NULL)
        *mode = DEV_CLASS_MODE;
    return NULL;
}

static int __init infiniti_init(void) {
    dev_t dev = MKDEV(0, 0);
    int ret = 0;

    printk("-------------------------\n");
    printk("-------------------------\n");
    printk("initializing infiniti memory manager\n");
    printk("-------------------------\n");
    printk("-------------------------\n");

    infiniti_class = class_create(THIS_MODULE, "infiniti");
	infiniti_class->devnode = infiniti_class_devnode;

    if (IS_ERR(infiniti_class)) {
	printk(KERN_ERR "failed to register infiniti memory class\n");
	return PTR_ERR(infiniti_class);
    }

    ret = alloc_chrdev_region(&dev, 0, 1, "infiniti");

    if (ret < 0) {
	printk(KERN_ERR "error registering memory controller device\n");
	class_destroy(infiniti_class);
	return ret;
    }


    major_num = MAJOR(dev);
    dev = MKDEV(major_num, 0);

    cdev_init(&ctrl_dev, &ctrl_fops);
    ctrl_dev.owner = THIS_MODULE;
    ctrl_dev.ops = &ctrl_fops;
    cdev_add(&ctrl_dev, dev, 1);

    device_create(infiniti_class, NULL, dev, NULL, "infiniti");

    return 0;
}


static void __exit infiniti_exit(void) {
    dev_t dev = 0;

    printk("unloading infiniti memory manager\n");
    dev = MKDEV(major_num, 0);

    unregister_chrdev_region(MKDEV(major_num, 0), 1);
    cdev_del(&ctrl_dev);
    device_destroy(infiniti_class, dev);

    class_destroy(infiniti_class);

}

module_init(infiniti_init);
module_exit(infiniti_exit);

/* vim: set ts=4: */
