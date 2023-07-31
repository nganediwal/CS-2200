#include "mmu.h"
#include "pagesim.h"
#include "va_splitting.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* The frame table pointer. You will set this up in system_init. */
fte_t *frame_table;

/**
 * --------------------------------- PROBLEM 2 --------------------------------------
 * Checkout PDF sections 4 for this problem
 * 
 * In this problem, you will initialize the frame_table pointer. The frame table will
 * be located at physical address 0 in our simulated memory. You should zero out the 
 * entries in the frame table, in case for any reason physical memory is not clean.
 * 
 * HINTS:
 *      - mem: Simulated physical memory already allocated for you.
 *      - PAGE_SIZE: The size of one page
 * ----------------------------------------------------------------------------------
 */
void system_init(void) {
    // TODO: initialize the frame_table pointer.
    frame_table = (fte_t *)(mem); //initialize frame table pointer to point to the first frame in physical memory
    memset(frame_table, 0, PAGE_SIZE); //Zero out the memory used by frame table
    
    frame_table->protected = 1; //mark first entry as 'protected', never want to evict frame table
}

/**
 * --------------------------------- PROBLEM 5 --------------------------------------
 * Checkout PDF section 6 for this problem
 * 
 * Takes an input virtual address and performs a memory operation.
 * 
 * @param addr virtual address to be translated
 * @param rw   'r' if the access is a read, 'w' if a write
 * @param data If the access is a write, one byte of data to write to our memory.
 *             Otherwise NULL for read accesses.
 * 
 * HINTS:
 *      - Remember that not all the entry in the process's page table are mapped in. 
 *      Check what in the pte_t struct signals that the entry is mapped in memory.
 * ----------------------------------------------------------------------------------
 */
uint8_t mem_access(vaddr_t addr, char rw, uint8_t data) {
    // TODO: translate virtual address to physical, then perfrom the specified operation
    
    stats.accesses += 1;
    vpn_t vpn = vaddr_vpn(addr); //vpn for virtual address
    uint16_t offset = vaddr_offset(addr); //offset for virtual address, stays same for physical address
    pte_t *table = ( (pte_t*) (mem + (PTBR * PAGE_SIZE))) + vpn; //get page table entry for PFN, by 
    //jumping to page table for current process, index using vpn

    if (table->valid == 0)
      page_fault(addr);

    pfn_t pfn = table->pfn; //get physical frame number from page table entry in page table
    //paddr_t phys_addr = *(mem + PAGE_SIZE * vpn); // ?vpn or pfn

    //update frame table
    frame_table[pfn].vpn = vpn; 
    frame_table[pfn].referenced = 1;
    frame_table[pfn].process = current_process; //how to define?

    paddr_t phys_addr = (paddr_t) (((table->pfn) * PAGE_SIZE) + offset); //physical address = PFN + offset

    //paddr_t* p_addr = (paddr_t*)(mem + pfn * PAGE_SIZE + offset); //vpn or pfn, index into physical memory
    // to get to physical address

    /* Either read or write the data to the physical address
       depending on 'rw' */
    if (rw == 'r') {
      return mem[phys_addr];
    } else {  
      table->dirty = 1;
      uint8_t ans = mem[phys_addr];
      mem[phys_addr] = data;
      return ans;
    }

    return 0;
}
