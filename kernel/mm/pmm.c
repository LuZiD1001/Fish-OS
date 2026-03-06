/* ==============================================================================
   MyOS - Physical Memory Manager (kernel/mm/pmm.c)
   Bitmap allocator for 4KB physical page frames.
   Each bit = one 4KB page. 0 = free, 1 = used.
   ============================================================================== */

#include "pmm.h"
#include "kernel/drivers/vga.h"
#include "libc/string.h"

#define PAGE_SIZE     4096
#define BITMAP_PAGES  (1024 * 1024)        /* Track up to 4GB */
#define BITMAP_SIZE   (BITMAP_PAGES / 32)  /* 32 pages per uint32_t */

static uint32_t bitmap[BITMAP_SIZE];
static uint32_t total_pages;
static uint32_t free_pages;

/* Mark a page as used */
static void pmm_set(uint32_t page) {
    bitmap[page / 32] |= (1 << (page % 32));
}

/* Mark a page as free */
static void pmm_clear(uint32_t page) {
    bitmap[page / 32] &= ~(1 << (page % 32));
}

/* Test if page is used */
static bool pmm_test(uint32_t page) {
    return !!(bitmap[page / 32] & (1 << (page % 32)));
}

void pmm_init(uint32_t mem_upper_kb, uint32_t kernel_end) {
    /* Calculate total available pages */
    total_pages = (mem_upper_kb * 1024) / PAGE_SIZE;
    free_pages  = 0;

    /* Start: mark everything as used */
    memset(bitmap, 0xFF, sizeof(bitmap));

    /* Free pages above kernel and below 4GB */
    uint32_t kernel_page_end = (kernel_end + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint32_t i = kernel_page_end; i < total_pages; i++) {
        pmm_clear(i);
        free_pages++;
    }

    /* Mark first 1MB as used (BIOS, VGA, etc.) */
    for (uint32_t i = 0; i < 256; i++)
        pmm_set(i);

    vga_printf("[PMM] Total: %d MB | Free: %d MB | Kernel ends at: %x\n",
               (total_pages * PAGE_SIZE) / (1024 * 1024),
               (free_pages * PAGE_SIZE) / (1024 * 1024),
               kernel_end);
}

/* Allocate one physical page - returns physical address */
void* pmm_alloc(void) {
    for (uint32_t i = 0; i < BITMAP_SIZE; i++) {
        if (bitmap[i] == 0xFFFFFFFF) continue;  /* All used, skip */
        for (int b = 0; b < 32; b++) {
            uint32_t page = i * 32 + b;
            if (!pmm_test(page)) {
                pmm_set(page);
                free_pages--;
                return (void*)(page * PAGE_SIZE);
            }
        }
    }
    return NULL;  /* Out of memory */
}

/* Free a physical page */
void pmm_free(void* addr) {
    uint32_t page = (uint32_t)addr / PAGE_SIZE;
    pmm_clear(page);
    free_pages++;
}

uint32_t pmm_get_free_pages(void) { return free_pages; }
uint32_t pmm_get_total_pages(void) { return total_pages; }
