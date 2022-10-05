#include <linux/slab.h>

#include "infiniti.h"
#include "fault.h"

/* segv signal error code, defined in /usr/include/asm-generic/siginfo.h */
#define SEGV_MAPERR 1 // segv signal, error code 1: address not mapped to object
#define SEGV_ACCERR 2 // segv signal, error code 2: invalid permissions for mapped object

int is_valid_address(struct infiniti_vm_area_struct *infiniti_vma, uintptr_t user_addr){
	struct list_head *pos, *next;
 	struct infiniti_vm_area_struct *node;
	/* traverse the list of nodes, and see if there is one which is reserved, and its range includes this address. */
	list_for_each_safe(pos, next, &(infiniti_vma->list)) {
		node = list_entry(pos, struct infiniti_vm_area_struct, list);
        if(node->status == RESERVED && user_addr >= node->start && user_addr < (node->start + 4096 * node->num_pages)){
            return 1;
        }
	}
    return 0;
}

int is_entire_table_free(unsigned long table){
	int i = 0;
	/* all four tables have 512 entries, each has 8 bytes, and thus 4KB per entry.. */
	for(i = 0; i < 512; i++){
		int offset = i * 8;
		unsigned long * entry = (unsigned long *)(table + offset);
		/* as long as one of them is one, then we can't free them or invalidate the tlb entries. */
		if(*entry & 0x1){
			/* not entirely free */
			return 0;
		}
	}
	/* table entirely free */
	return 1;
}

/*
 * error_code:
 * 1 == not present
 * 2 == permissions error
 * return 0 if handled successful; otherwise return -1.
 * */
uintptr_t get_free_page();

int infiniti_do_page_fault(struct infiniti_vm_area_struct *infiniti_vma, uintptr_t fault_addr, u32 error_code) {
	unsigned long cr3;	
	unsigned kernel_addr; //uintptr_t kernel_addr
	unsigned long pml4, pdp, pd, pt;
    unsigned long *pml4_entry, *pdp_entry, *pd_entry, *pt_entry; 

	//fault_addr should within the reserved memory region and application should have permission to access fault address
	if(is_valid_address(infiniti_vma, fault_addr)==0 || error_code == SEGV_ACCERR)
		return -1;

	//get the content of the cr3 register
	cr3 = get_cr3();
	//
	pml4 = __va(cr3 & 0xffffffffff000); 
	//base address + offset
	pml4_entry = (unsigned long*)pml4 + (((fault_addr >> 39) & 0xfff) << 3);
    //
    if(!(pml4_entry & 0x1)){
		get_free_page();
		pml4_entry = pml4_entry | 0x1;
		pml4_entry = pml4_entry | 0x2;
		pml4_entry = pml4_entry | 0x4
	}
 
    //if

	printk("Page fault!\n");
    return -1; //return 0 if a page fault is handled successfully
}

uintptr_t get_free_page(){
	uintptr_t kernel_addr = 0;
	kernel_addr = (uintptr_t)get_zeroed_page(GFP_KERNEL);
	if (!kernel_addr) {
		printk(KERN_INFO "failed to allocate one page\n");
		return -ENOMEM;
	}	
	return kernel_addr;
}

/* this function takes a user VA and free its PA as well as its kernel va. */
void infiniti_free_pa(uintptr_t user_addr){
	// is_entire_table_free(unsigned long table);
	unsigned long page_table;
	if(is_entire_table_free(page_table)){
		do something;
	}else{
		do something;
	}

	free_page(kernel_addr);
	
	return;
}

/* vim: set ts=4: */
