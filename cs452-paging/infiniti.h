/* 
 * Original author: Jack Lange
 */

#ifndef __KERNEL__
char * dev_file = "/dev/infiniti";
#endif

#define RESERVED 0
#define FREE 1

struct alloc_request {
    // input
    unsigned long long size;

    // output 
    unsigned long long addr;    
} __attribute__((packed));

struct page_fault {
    unsigned long long fault_addr;
    unsigned int error_code;
} __attribute__((packed));

// IOCTLs
#define LAZY_ALLOC     30
#define LAZY_FREE      31
#define DUMP_STATE 32

#define PAGE_FAULT     50
#define INVALIDATE_PAGE 51

#ifdef __KERNEL__
/* 128GB of virtual address space, that will hopefully be unused
   These are pulled from the user space memory region 
   Note that this could be a serious issue, because this range is not reserved 

   Based on my thorough investigation (running `cat /proc/self/maps` ~20 times), 
   this appears to sit between the heap and runtime libraries
*/
#define INFINITI_MEM_REGION_START 0x1000000000LL
#define INFINITI_MEM_REGION_END   0x3000000000LL

// this range, which is 0x2000000000, is 137438953472 bytes, which is 33554432*4096, thus, at the beginning, we see 33554432 pages when dump.

// Returns the current CR3 value
static inline uintptr_t get_cr3(void) {
    u64 cr3 = 0;

    __asm__ __volatile__ ("movq %%cr3, %0; "
			  : "=q"(cr3)
			  :
			  );

    return (uintptr_t)cr3;
}

static inline void invlpg(uintptr_t page_addr) {
    //printk(KERN_INFO "invalidating address %p\n", (void *)page_addr);
    __asm__ __volatile__ ("invlpg (%0); "
			  : 
			  :"r"(page_addr)
			  : "memory"
			  );
}

#include <linux/types.h>

#endif
