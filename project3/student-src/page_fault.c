#include "mmu.h"
#include "pagesim.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * --------------------------------- PROBLEM 6 --------------------------------------
 * Checkout PDF section 7 for this problem
 * 
 * Page fault handler.
 * 
 * When the CPU encounters an invalid address mapping in a page table, it invokes the 
 * OS via this handler. Your job is to put a mapping in place so that the translation 
 * can succeed.
 * 
 * @param addr virtual address in the page that needs to be mapped into main memory.
 * 
 * HINTS:
 *      - You will need to use the global variable current_process when
 *      altering the frame table entry.
 *      - Use swap_exists() and swap_read() to update the data in the 
 *      frame as it is mapped in.
 * ----------------------------------------------------------------------------------
 */
void page_fault(vaddr_t addr) {
   // TODO: Get a new frame, then correctly update the page table and frame table
   vpn_t vpn = vaddr_vpn(addr);
   //uint16_t off = vaddr_offset(addr);
   stats.page_faults += 1;
   
   pfn_t temp = free_frame(); //since page fault grab a new frame

   pte_t* t = ((pte_t*) (mem + (PTBR * PAGE_SIZE))) + vpn;


   paddr_t* p_address = (paddr_t*) ((PAGE_SIZE * temp) + mem);

   if(swap_exists(t)){ //only if data is not null
      swap_read(t, p_address);
   } else {
      memset(p_address, 0, PAGE_SIZE);
   }

   t->valid = 1;
   t->dirty = 0;
   t->pfn = temp;

   frame_table[temp].mapped = 1;
   frame_table[temp].process = current_process;
   frame_table[temp].vpn = vpn;
}

#pragma GCC diagnostic pop
