/* ==============================================================================
   MyOS - IDT Implementation (kernel/idt.c)
   Sets up 256 interrupt descriptors, remaps PIC, installs ISR/IRQ handlers
   ============================================================================== */

#include "idt.h"
#include "kernel/drivers/vga.h"

/* I/O port helpers */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" :: "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

/* PIC ports */
#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

static idt_entry_t idt[256];
static idt_ptr_t   idt_ptr;
static isr_t       interrupt_handlers[256];

/* Exception names for panic messages */
const char* exception_names[] = {
    "Division By Zero", "Debug", "Non-maskable Interrupt", "Breakpoint",
    "Overflow", "Bound Range Exceeded", "Invalid Opcode", "Device Not Available",
    "Double Fault", "Coprocessor Segment Overrun", "Invalid TSS",
    "Segment Not Present", "Stack-Segment Fault", "General Protection Fault",
    "Page Fault", "Reserved", "x87 FPU Error", "Alignment Check",
    "Machine Check", "SIMD FP Exception"
};

/* External ASM stubs */
extern void isr0(void);  extern void isr1(void);  extern void isr2(void);
extern void isr3(void);  extern void isr4(void);  extern void isr5(void);
extern void isr6(void);  extern void isr7(void);  extern void isr8(void);
extern void isr9(void);  extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void);
extern void isr15(void); extern void isr16(void); extern void isr17(void);
extern void isr18(void); extern void isr19(void);
extern void irq0(void);  extern void irq1(void);  extern void irq2(void);
extern void irq3(void);  extern void irq4(void);  extern void irq5(void);
extern void irq6(void);  extern void irq7(void);  extern void irq8(void);
extern void irq9(void);  extern void irq10(void); extern void irq11(void);
extern void irq12(void); extern void irq13(void); extern void irq14(void);
extern void irq15(void);

static void idt_set_gate(uint8_t n, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[n].base_low  = base & 0xFFFF;
    idt[n].base_high = (base >> 16) & 0xFFFF;
    idt[n].selector  = sel;
    idt[n].zero      = 0;
    idt[n].flags     = flags;
}

/* Remap PIC so IRQs don't conflict with CPU exceptions */
static void pic_remap(void) {
    /* Save masks */
    uint8_t m1 = inb(PIC1_DATA);
    uint8_t m2 = inb(PIC2_DATA);

    /* Start init sequence */
    outb(PIC1_CMD, 0x11);
    outb(PIC2_CMD, 0x11);

    /* Remap: IRQ0-7 → INT 32-39, IRQ8-15 → INT 40-47 */
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    /* Tell PICs about each other */
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    /* 8086 mode */
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    /* Restore masks */
    outb(PIC1_DATA, m1);
    outb(PIC2_DATA, m2);
}

void idt_init(void) {
    idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idt_ptr.base  = (uint32_t)&idt;

    /* Install exception handlers */
    idt_set_gate(0,  (uint32_t)isr0,  0x08, 0x8E);
    idt_set_gate(1,  (uint32_t)isr1,  0x08, 0x8E);
    idt_set_gate(2,  (uint32_t)isr2,  0x08, 0x8E);
    idt_set_gate(3,  (uint32_t)isr3,  0x08, 0x8E);
    idt_set_gate(4,  (uint32_t)isr4,  0x08, 0x8E);
    idt_set_gate(5,  (uint32_t)isr5,  0x08, 0x8E);
    idt_set_gate(6,  (uint32_t)isr6,  0x08, 0x8E);
    idt_set_gate(7,  (uint32_t)isr7,  0x08, 0x8E);
    idt_set_gate(8,  (uint32_t)isr8,  0x08, 0x8E);
    idt_set_gate(9,  (uint32_t)isr9,  0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);

    pic_remap();

    /* Install IRQ handlers */
    idt_set_gate(32, (uint32_t)irq0,  0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1,  0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2,  0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3,  0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4,  0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5,  0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6,  0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7,  0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8,  0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9,  0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);

    /* Load IDT */
    __asm__ volatile("lidt %0" :: "m"(idt_ptr));
    __asm__ volatile("sti");  /* Enable interrupts */
}

void idt_register_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

/* Called from isr_common_stub in interrupt.asm */
void isr_handler(registers_t* regs) {
    if (interrupt_handlers[regs->int_no]) {
        interrupt_handlers[regs->int_no](regs);
    } else {
        /* Unhandled exception - kernel panic */
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
        vga_printf("\n\n  *** KERNEL PANIC ***\n");
        if (regs->int_no < 20)
            vga_printf("  Exception: %s\n", exception_names[regs->int_no]);
        vga_printf("  INT=%d  ERR=%x  EIP=%x\n",
                   regs->int_no, regs->err_code, regs->eip);
        __asm__ volatile("cli; hlt");
    }
}

/* Called from irq_common_stub in interrupt.asm */
void irq_handler(registers_t* regs) {
    /* Send EOI to PIC(s) */
    if (regs->int_no >= 40)
        outb(PIC2_CMD, 0x20);   /* Secondary PIC */
    outb(PIC1_CMD, 0x20);       /* Primary PIC */

    if (interrupt_handlers[regs->int_no])
        interrupt_handlers[regs->int_no](regs);
}
