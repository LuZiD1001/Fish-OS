/* ==============================================================================
   MyOS - Interrupt Descriptor Table (kernel/idt.h)
   ============================================================================== */

#pragma once
#include "kernel/types.h"

/* IDT Entry */
typedef struct {
    uint16_t base_low;   /* Lower 16 bits of handler address */
    uint16_t selector;   /* Kernel code segment selector */
    uint8_t  zero;       /* Always 0 */
    uint8_t  flags;      /* Type and attributes */
    uint16_t base_high;  /* Upper 16 bits of handler address */
} PACKED idt_entry_t;

/* IDT Pointer */
typedef struct {
    uint16_t limit;
    uint32_t base;
} PACKED idt_ptr_t;

/* Saved CPU registers on interrupt */
typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pusha */
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;              /* pushed by CPU */
} registers_t;

/* Interrupt handler function type */
typedef void (*isr_t)(registers_t*);

void idt_init(void);
void idt_register_handler(uint8_t n, isr_t handler);

/* Exception names for debugging */
extern const char* exception_names[];
