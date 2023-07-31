#include "proc.h"
#include "mmu.h"
#include "pagesim.h"
#include "va_splitting.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * --------------------------------- PROBLEM 3 --------------------------------------
 * Checkout PDF section 4 for this problem
 * 
 * This function gets called every time a new process is created.
 * You will need to allocate a frame for the process's page table using the
 * free_frame function. Then, you will need update both the frame table and
 * the process's PCB. 
 * 
 * @param proc pointer to process that is being initialized 
 * 
 * HINTS:
 *      - pcb_t: struct defined in pagesim.h that is a process's PCB.
 *      - You are not guaranteed that the memory returned by the free frame allocator
 *      is empty - an existing frame could have been evicted for our new page table.
 * ----------------------------------------------------------------------------------
 */
void proc_init(pcb_t *proc) {
    // TODO: initialize proc's page table.
    pfn_t i = free_frame(); //allocating frame, returns a type physical frame number (PFN), serves as index
    memset(mem+(i*PAGE_SIZE), 0, PAGE_SIZE); //zero out ith frame in memory, uptil size of page_size since that is size of each frame

    frame_table[i].protected = 1; //mark first entry of frame table as protected to not evict it, don't want to evict frame 
    //containing page table of currently executing process

    proc->saved_ptbr = i; //update PBTR to new PFN
}

/**
 * --------------------------------- PROBLEM 4 --------------------------------------
 * Checkout PDF section 5 for this problem
 * 
 * Switches the currently running process to the process referenced by the proc 
 * argument.
 * 
 * Every process has its own page table, as you allocated in proc_init. You will
 * need to tell the processor to use the new process's page table.
 * 
 * @param proc pointer to process to become the currently running process.
 * 
 * HINTS:
 *      - Look at the global variables defined in pagesim.h. You may be interested in
 *      the definition of pcb_t as well.
 * ----------------------------------------------------------------------------------
 */
void context_switch(pcb_t *proc) {
    // TODO: update any global vars and proc's PCB to match the context_switch.
    PTBR = proc->saved_ptbr;
}

/**
 * --------------------------------- PROBLEM 8 --------------------------------------
 * Checkout PDF section 8 for this problem
 * 
 * When a process exits, you need to free any pages previously occupied by the
 * process.
 * 
 * HINTS:
 *      - If the process has swapped any pages to disk, you must call
 *      swap_free() using the page table entry pointer as a parameter.
 *      - If you free any protected pages, you must also clear their"protected" bits.
 * ----------------------------------------------------------------------------------
 */
void proc_cleanup(pcb_t *proc) {
    // TODO: Iterate the proc's page table and clean up each valid page
    pte_t *page_table = (pte_t *) ((proc->saved_ptbr * PAGE_SIZE) + mem); //process's page table

    for (size_t i = 0; i < NUM_PAGES; i++) {
        pte_t *curr = page_table + i; //current page table entry in this process's page table
        if (curr->valid == 1) {
            frame_table[curr->pfn].mapped = 0; //unmap frames
            frame_table[curr->pfn].protected = 0; //clear protected bits
        }
        if (swap_exists(curr)) {
            swap_free(curr); //free any swap entries that have been mapped to pages.
        }
    }
    frame_table[proc->saved_ptbr].mapped = 0; //free the whole page table, which is located in the frame table
    frame_table[proc->saved_ptbr].protected = 0;
}

#pragma GCC diagnostic pop
