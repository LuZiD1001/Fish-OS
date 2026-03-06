/* ==============================================================================
   MyOS - Global Descriptor Table (kernel/gdt.h)
   Defines memory segments for protected mode: kernel/user code+data, TSS
   ============================================================================== */

#pragma once
#include "kernel/types.h"

/* GDT Entry: describes a memory segment */
typedef struct {
    uint16_t limit_low;    /* Lower 16 bits of segment limit */
    uint16_t base_low;     /* Lower 16 bits of base address */
    uint8_t  base_mid;     /* Middle 8 bits of base address */
    uint8_t  access;       /* Access byte (present, ring, type) */
    uint8_t  granularity;  /* Flags + upper 4 bits of limit */
    uint8_t  base_high;    /* Upper 8 bits of base address */
} PACKED gdt_entry_t;

/* GDT Pointer passed to lgdt instruction */
typedef struct {
    uint16_t limit;        /* Size of GDT - 1 */
    uint32_t base;         /* Linear address of GDT */
} PACKED gdt_ptr_t;

/* Task State Segment - needed for ring switches */
typedef struct {
    uint32_t prev_tss;
    uint32_t esp0;         /* Stack pointer for ring 0 */
    uint32_t ss0;          /* Stack segment for ring 0 */
    uint32_t esp1, ss1;
    uint32_t esp2, ss2;
    uint32_t cr3, eip, eflags;
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap, iomap_base;
} PACKED tss_entry_t;

void gdt_init(void);
void gdt_set_kernel_stack(uint32_t stack);

/* ASM functions */
extern void gdt_flush(uint32_t gdt_ptr);
extern void tss_flush(void);
