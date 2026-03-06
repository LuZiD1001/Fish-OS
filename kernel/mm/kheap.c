/* ==============================================================================
   MyOS - Kernel Heap Allocator (kernel/mm/kheap.c)
   Simple free-list allocator for kernel dynamic memory (kmalloc/kfree).
   Uses a linked list of blocks with a header.
   ============================================================================== */

#include "kheap.h"
#include "kernel/drivers/vga.h"

/* Each allocation has this header prepended */
typedef struct heap_block {
    uint32_t          size;    /* Size of data (not including header) */
    bool              free;    /* Is this block free? */
    struct heap_block* next;   /* Next block in list */
    uint32_t          magic;   /* Corruption guard */
} heap_block_t;

#define HEAP_MAGIC  0xDEADBEEF
#define HEAP_START  0x01000000   /* 16 MB - safe area after kernel */
#define HEAP_SIZE   (4 * 1024 * 1024)  /* 4 MB initial heap */

static heap_block_t* heap_head = NULL;

void kheap_init(void) {
    heap_head = (heap_block_t*)HEAP_START;
    heap_head->size  = HEAP_SIZE - sizeof(heap_block_t);
    heap_head->free  = true;
    heap_head->next  = NULL;
    heap_head->magic = HEAP_MAGIC;
    vga_printf("[HEAP] Initialized at %x, size: %d KB\n",
               HEAP_START, HEAP_SIZE / 1024);
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;

    /* Align to 8 bytes */
    size = (size + 7) & ~7;

    heap_block_t* block = heap_head;
    while (block) {
        if (block->magic != HEAP_MAGIC) {
            vga_printf("[HEAP] CORRUPTION DETECTED at %x!\n", (uint32_t)block);
            return NULL;
        }
        if (block->free && block->size >= size) {
            /* Split block if there's enough space for another header + data */
            if (block->size >= size + sizeof(heap_block_t) + 8) {
                heap_block_t* new_block = (heap_block_t*)
                    ((uint8_t*)block + sizeof(heap_block_t) + size);
                new_block->size  = block->size - size - sizeof(heap_block_t);
                new_block->free  = true;
                new_block->next  = block->next;
                new_block->magic = HEAP_MAGIC;
                block->next = new_block;
                block->size = size;
            }
            block->free = false;
            return (void*)((uint8_t*)block + sizeof(heap_block_t));
        }
        block = block->next;
    }
    return NULL;  /* Out of heap */
}

void kfree(void* ptr) {
    if (!ptr) return;

    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    if (block->magic != HEAP_MAGIC) {
        vga_printf("[HEAP] kfree: bad pointer %x\n", (uint32_t)ptr);
        return;
    }
    block->free = true;

    /* Coalesce adjacent free blocks to prevent fragmentation */
    heap_block_t* cur = heap_head;
    while (cur && cur->next) {
        if (cur->free && cur->next->free) {
            cur->size += sizeof(heap_block_t) + cur->next->size;
            cur->next  = cur->next->next;
        } else {
            cur = cur->next;
        }
    }
}

/* Allocate zero-initialized memory */
void* kcalloc(size_t count, size_t size) {
    void* ptr = kmalloc(count * size);
    if (ptr) {
        uint8_t* p = (uint8_t*)ptr;
        for (size_t i = 0; i < count * size; i++)
            p[i] = 0;
    }
    return ptr;
}
