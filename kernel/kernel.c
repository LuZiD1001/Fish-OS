/* ==============================================================================
   MyOS - Kernel Entry Point (kernel/kernel.c)
   This is where the kernel starts after the bootloader hands control.
   Initializes all subsystems in order, then launches the shell.
   ============================================================================== */

#include "kernel/types.h"
#include "kernel/gdt.h"
#include "kernel/idt.h"
#include "kernel/drivers/vga.h"
#include "kernel/drivers/timer.h"
#include "kernel/drivers/keyboard.h"
#include "kernel/mm/pmm.h"
#include "kernel/mm/vmm.h"
#include "kernel/mm/kheap.h"
#include "kernel/fs/vfs.h"
#include "kernel/fs/ramfs.h"
#include "kernel/proc/process.h"
#include "kernel/proc/scheduler.h"
#include "kernel/shell.h"

/* Multiboot info structure (partial - we only use what we need) */
typedef struct {
    uint32_t flags;
    uint32_t mem_lower;   /* KB below 1MB */
    uint32_t mem_upper;   /* KB above 1MB */
    /* ... (more fields we don't use yet) */
} PACKED multiboot_info_t;

#define MULTIBOOT_MAGIC_EXPECTED 0x2BADB002

/* Kernel end symbol from linker script */
extern uint32_t kernel_end;

/* ── Boot Banner ── */
static void print_banner(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts(
        "\n"
        "  ███╗   ███╗██╗   ██╗ ██████╗ ███████╗\n"
        "  ████╗ ████║╚██╗ ██╔╝██╔═══██╗██╔════╝\n"
        "  ██╔████╔██║ ╚████╔╝ ██║   ██║███████╗\n"
        "  ██║╚██╔╝██║  ╚██╔╝  ██║   ██║╚════██║\n"
        "  ██║ ╚═╝ ██║   ██║   ╚██████╔╝███████║\n"
        "  ╚═╝     ╚═╝   ╚═╝    ╚═════╝ ╚══════╝\n"
    );
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts("                 v0.1  |  i686  |  Hobby OS\n\n");
    vga_set_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    vga_puts("  ─────────────────────────────────────────────\n\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

/* ── Kernel Main ── */
void kernel_main(uint32_t magic, multiboot_info_t* mbi) {
    /* 1. VGA first - need output before anything else */
    vga_init();
    print_banner();

    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("  [BOOT] Kernel loaded. Initializing subsystems...\n\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    /* 2. Verify bootloader */
    if (magic != MULTIBOOT_MAGIC_EXPECTED) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_printf("  FATAL: Invalid multiboot magic: %x\n", magic);
        __asm__ volatile("cli; hlt");
    }

    /* 3. GDT - segment descriptors */
    gdt_init();
    vga_puts("  [OK] GDT\n");

    /* 4. IDT - interrupt handlers + PIC */
    idt_init();
    vga_puts("  [OK] IDT + PIC\n");

    /* 5. Physical memory manager */
    uint32_t mem_kb = mbi->mem_upper;
    pmm_init(mem_kb, (uint32_t)&kernel_end);
    vga_puts("  [OK] Physical Memory Manager\n");

    /* 6. Virtual memory */
    vmm_init();
    vga_puts("  [OK] Virtual Memory Manager\n");

    /* 7. Kernel heap */
    kheap_init();
    vga_puts("  [OK] Kernel Heap\n");

    /* 8. Timer (100 Hz = 10ms resolution) */
    timer_init(100);
    vga_puts("  [OK] Timer (PIT @ 100Hz)\n");

    /* 9. Keyboard */
    keyboard_init();
    vga_puts("  [OK] PS/2 Keyboard\n");

    /* 10. Virtual filesystem */
    vfs_init();
    vfs_node_t* root = ramfs_create_root();
    vfs_mount("/", root);
    vga_puts("  [OK] VFS + RAMFS\n");

    /* 11. Process manager */
    process_init();
    vga_puts("  [OK] Process Manager\n");

    /* 12. Scheduler */
    scheduler_init();
    vga_puts("  [OK] Scheduler\n");

    /* Done! */
    vga_set_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    vga_puts("\n  ─────────────────────────────────────────────\n");
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("  All systems ready.\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    /* Hand off to interactive shell */
    shell_run();

    /* Should never reach here */
    __asm__ volatile("cli; hlt");
}
