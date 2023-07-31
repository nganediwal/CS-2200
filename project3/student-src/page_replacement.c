#include "types.h"
#include "pagesim.h"
#include "mmu.h"
#include "swapops.h"
#include "stats.h"
#include "util.h"

pfn_t select_victim_frame(void);

pfn_t last_evicted = 0;

/**
 * --------------------------------- PROBLEM 7 --------------------------------------
 * Checkout PDF section 7 for this problem
 * 
 * Make a free frame for the system to use. You call the select_victim_frame() method
 * to identify an "available" frame in the system (already given). You will need to 
 * check to see if this frame is already mapped in, and if it is, you need to evict it.
 * 
 * @return victim_pfn: a phycial frame number to a free frame be used by other functions.
 * 
 * HINTS:
 *      - When evicting pages, remember what you checked for to trigger page faults 
 *      in mem_access
 *      - If the page table entry has been written to before, you will need to use
 *      swap_write() to save the contents to the swap queue.
 * ----------------------------------------------------------------------------------
 */
pfn_t free_frame(void) {
    pfn_t victim_pfn;
    victim_pfn = select_victim_frame();

    fte_t *victim = (fte_t*) (frame_table + victim_pfn); //index into frame table which is victim, using pfn

    // TODO: evict any mapped pages.
    if (frame_table[victim_pfn].mapped) {
        pcb_t* vict_pcb = victim->process; //get process id from frame table
        pte_t* vict_pte = (pte_t*) (mem+(vict_pcb->saved_ptbr*PAGE_SIZE)); //get start address of page table for this process using PTBR
        vict_pte += victim->vpn; //add vpn offset to index into page table entry to remove 
        if (vict_pte->dirty == 1) {
            stats.writebacks += 1;
            swap_write(vict_pte, mem + (victim_pfn * PAGE_SIZE)); //put these contents from victim PTE into disk
        }
        vict_pte->valid = 0;
        vict_pte->dirty = 0; //clear dirty bit
    }   
    victim->mapped = 0;
    /* Return the pfn */
    return victim_pfn;
}


/**
 * --------------------------------- PROBLEM 9 --------------------------------------
 * Checkout PDF section 7, 9, and 11 for this problem
 * 
 * Finds a free physical frame. If none are available, uses either a
 * randomized, FCFS, or clocksweep algorithm to find a used frame for
 * eviction.
 * 
 * @return The physical frame number of a victim frame.
 * 
 * HINTS: 
 *      - Use the global variables MEM_SIZE and PAGE_SIZE to calculate
 *      the number of entries in the frame table.
 *      - Use the global last_evicted to keep track of the pointer into the frame table
 * ----------------------------------------------------------------------------------
 */
pfn_t select_victim_frame() {
    /* See if there are any free frames first */
    size_t num_entries = MEM_SIZE / PAGE_SIZE;
    for (size_t i = 0; i < num_entries; i++) {
        if (!frame_table[i].protected && !frame_table[i].mapped) {
            return i;
        }
    }

    // RANDOM implemented for you.
    if (replacement == RANDOM) {
        /* Play Russian Roulette to decide which frame to evict */
        pfn_t last_unprotected = NUM_FRAMES;
        for (pfn_t i = 0; i < num_entries; i++) {
            if (!frame_table[i].protected) {
                last_unprotected = i;
                if (prng_rand() % 2) {
                    return i;
                }
            }
        }
        /* If no victim found yet take the last unprotected frame
           seen */
        if (last_unprotected < NUM_FRAMES) {
            return last_unprotected;
        }
    } else if (replacement == FIFO) {
        // TODO: Implement a FIFO algorithm here
        for (pfn_t i = last_evicted; i < NUM_FRAMES; i++) {
            if (!frame_table[i].protected) {
                last_evicted = (i + 1) % NUM_FRAMES;
                return i;
            }
            if (i == NUM_FRAMES - 1) {
                i = 0;
            }
        }
    } else if (replacement == CLOCKSWEEP) {
        // TODO: Implement a clocksweep page replacement algorithm here 
        for (pfn_t i = last_evicted; i < NUM_FRAMES; i++) {
            if (!frame_table[i].protected) {
                if (frame_table[i].referenced == 1) {
                    frame_table[i].referenced = 0;
                }        
                else {
                    last_evicted = (i + 1) % NUM_FRAMES;
                    return i; 
                }
            }
            if (i == NUM_FRAMES - 1) {
                i = 0; //return to simple FIFO
            }
        }
        for (pfn_t i = last_evicted; i < NUM_FRAMES; i++) {
            if (!frame_table[i].protected) {
                last_evicted = (i + 1) % NUM_FRAMES;
                return i;
            }
            if (i == NUM_FRAMES - 1) {
                i = 0;
            }
        }
    }

    /* If every frame is protected, give up. This should never happen
       on the traces we provide you. */
    panic("System ran out of memory\n");
    exit(1);
}
