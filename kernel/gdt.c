/* ==============================================================================
   MyOS - GDT Implementation (kernel/gdt.c)
   Sets up 5 segments: null, kernel code, kernel data, user code, user data + TSS
   ============================================================================== */

#include "gdt.h"
#include "libc/string.h"

#define GDT_ENTRIES 6

static gdt_entry_t gdt[GDT_ENTRIES];
static gdt_ptr_t   gdt_ptr;
static tss_entry_t tss;

static void gdt_set_entry(int i, uint32_t base, uint32_t limit,
                           uint8_t access, uint8_t gran) {
    gdt[i].base_low    = (base & 0xFFFF);
    gdt[i].base_mid    = (base >> 16) & 0xFF;
    gdt[i].base_high   = (base >> 24) & 0xFF;
    gdt[i].limit_low   = (limit & 0xFFFF);
    gdt[i].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[i].access      = access;
}

static void tss_init(uint32_t kernel_stack) {
    uint32_t base  = (uint32_t)&tss;
    uint32_t limit = base + sizeof(tss);

    /* TSS descriptor in GDT slot 5 */
    gdt_set_entry(5, base, limit, 0xE9, 0x00);

    memset(&tss, 0, sizeof(tss));
    tss.ss0  = 0x10;            /* Kernel data segment */
    tss.esp0 = kernel_stack;    /* Kernel stack */
    tss.cs   = 0x0B;            /* User code segment | 3 */
    tss.ss   = tss.ds = tss.es = tss.fs = tss.gs = 0x13;
}

void gdt_init(void) {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * GDT_ENTRIES) - 1;
    gdt_ptr.base  = (uint32_t)&gdt;

    /* 0: Null descriptor (required by CPU) */
    gdt_set_entry(0, 0, 0, 0, 0);

    /* 1: Kernel Code  - ring 0, execute/read */
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* 2: Kernel Data  - ring 0, read/write */
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* 3: User Code    - ring 3, execute/read */
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    /* 4: User Data    - ring 3, read/write */
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    /* 5: TSS */
    tss_init(0);

    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();
}

void gdt_set_kernel_stack(uint32_t stack) {
    tss.esp0 = stack;
}
