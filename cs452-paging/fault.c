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

uintptr_t get_free_page(void){
	uintptr_t kernel_addr = 0;
	kernel_addr = (uintptr_t)get_zeroed_page(GFP_KERNEL);
	if (!kernel_addr) {
		printk(KERN_INFO "failed to allocate one page\n");
		return -ENOMEM;
	}	
	return kernel_addr;
}

int infiniti_do_page_fault(struct infiniti_vm_area_struct *infiniti_vma, uintptr_t fault_addr, u32 error_code) {
	unsigned long cr3;	
	unsigned long pml4, pdp, pd, pt;
    unsigned long *pml4_entry, *pdp_entry, *pd_entry, *pt_entry; 
    unsigned long kernel_addr;

	//fault_addr should within the reserved memory region and application should have permission to access fault address
	if(is_valid_address(infiniti_vma, fault_addr)==0 || error_code == SEGV_ACCERR)
		return -1;

	//get the content of the cr3 register
	cr3 = get_cr3();
	pml4 = (unsigned long)__va(cr3 & 0xffffffffff000); 
	pml4_entry = (unsigned long*)(pml4 + (((fault_addr >> 39) & 0x1ff) << 3));
    //if bit 0 is not 1
    if(!(*pml4_entry & 0x1)){
		kernel_addr = get_free_page();
		printk(KERN_INFO "kernel address is %lx, and its physical address is %lx\n", kernel_addr, __pa(kernel_addr));
		//change the PML4E's bit 0, bit 1, bit 2 to 1
		*pml4_entry = *pml4_entry | 0x7;
		//store the bits 51:12 of the allocated page's physical address into the PML4E entry's bits 51:12
		*pml4_entry = *pml4_entry | __pa(kernel_addr & 0xffffffffff000);
	}

	pdp = (unsigned long)__va(*pml4_entry & 0xffffffffff000); 
	pdp_entry = (unsigned long*)(pdp + (((fault_addr >> 30) & 0x1ff) << 3));
	if(!(*pdp_entry & 0x1)){
		kernel_addr = get_free_page();
		printk(KERN_INFO "kernel address is %lx, and its physical address is %lx\n", kernel_addr, __pa(kernel_addr));
		*pdp_entry = *pdp_entry | 0x7;
		*pdp_entry = *pdp_entry | __pa(kernel_addr & 0xffffffffff000);			
	}
	
	pd = (unsigned long)__va(*pdp_entry & 0xffffffffff000);
	pd_entry = (unsigned long*)(pd + (((fault_addr >> 21) & 0x1ff) << 3));
	if(!(*pd_entry & 0x1)){
		kernel_addr = get_free_page();
		printk(KERN_INFO "kernel address is %lx, and its physical address is %lx\n", kernel_addr, __pa(kernel_addr));
		*pd_entry = *pd_entry | 0x7;
		*pd_entry = *pd_entry | __pa(kernel_addr & 0xffffffffff000);
	}

	pt = (unsigned long)__va(*pd_entry & 0xffffffffff000);
	pt_entry = (unsigned long*)(pt + (((fault_addr >> 12) & 0x1ff) << 3));
	if(!(*pt_entry & 0x1)){
		kernel_addr = get_free_page();
		*pt_entry = *pt_entry | 0x7;
		*pt_entry = *pt_entry | __pa(kernel_addr & 0xffffffffff000);
	}			
    return 0; 
}

/* this function takes a user VA and free its PA as well as its kernel va. */
void infiniti_free_pa(uintptr_t user_addr){
	unsigned long cr3;	
	unsigned long kernel_addr;
	unsigned long pml4, pdp, pd, pt;
    unsigned long *pml4_entry, *pdp_entry, *pd_entry, *pt_entry; 

	cr3 = get_cr3();
	pml4 = (unsigned long)__va(cr3 & 0xffffffffff000); 
	pml4_entry = (unsigned long*)(pml4 + (((user_addr >> 39) & 0x1ff) << 3));
	if(!(*pml4_entry & 0x1)) {
		return;
	}

	pdp = (unsigned long)__va(*pml4_entry & 0xffffffffff000); 
	pdp_entry = (unsigned long*)(pdp + (((user_addr >> 30) & 0x1ff) << 3));
	if(!(*pdp_entry & 0x1)) {
		return;
	}
	
	pd = (unsigned long)__va(*pdp_entry & 0xffffffffff000);
	pd_entry = (unsigned long*)(pd + (((user_addr >> 21) & 0x1ff) << 3));
	if(!(*pd_entry & 0x1)) {
		return;
	}
	
	pt = (unsigned long)__va(*pd_entry & 0xffffffffff000);
	pt_entry = (unsigned long*)(pt + (((user_addr >> 12) & 0x1ff) << 3));
	if(!(*pt_entry & 0x1)) {
		return;
	}

	kernel_addr = (unsigned long)__va((*pt_entry & 0xffffffffff000) + (user_addr & 0xfff));
	//kernel_addr = (unsigned long)__va(*pt_entry + (user_addr & 0xfff));
	free_page(kernel_addr);
	invlpg(kernel_addr);

	*pt_entry = 0;
	if(is_entire_table_free(pt)){
		free_page(pt);
		invlpg(pt);
	}else{
		return;
	}

	*pd_entry = 0;
	if(is_entire_table_free(pd)){
		free_page(pd);
		invlpg(pd);
	}else{
		return;
	}
	
	*pdp_entry = 0;
	if(is_entire_table_free(pdp)){
		free_page(pdp);
		invlpg(pdp);
	}else{
		return;
	}
	
	*pml4_entry = 0;
	if(is_entire_table_free(pml4)){
		free_page(pml4);
		invlpg(pml4);
	}else{
		return;
	}

	return;
}

/* vics452m: set ts=4: */
