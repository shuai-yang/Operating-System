#ifndef __FAULT_H__
#define __FAULT_H__

#include <linux/module.h>
#include <linux/list.h>

struct infiniti_vm_area_struct {
	unsigned long status;
	unsigned long start;
	unsigned long num_pages; /* num_pages represents how many pages we have in this memory region */
	struct list_head list;
};

int infiniti_do_page_fault(struct infiniti_vm_area_struct *vma, uintptr_t fault_addr, u32 error_code);
void infiniti_free_pa(uintptr_t user_vaddr);
#endif

/* vim: set ts=4: */
