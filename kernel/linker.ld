/* ==============================================================================
   MyOS - Linker Script (kernel/linker.ld)
   Places kernel at 1MB (0x100000) - standard for x86 OS development
   ============================================================================== */

ENTRY(_start)

SECTIONS {
    /* Load at 1MB physical address */
    . = 0x100000;

    /* Multiboot header must be in first 8KB */
    .multiboot : { *(.multiboot) }

    /* Code section */
    .text ALIGN(4096) : {
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata ALIGN(4096) : {
        *(.rodata)
        *(.rodata.*)
    }

    /* Initialized data */
    .data ALIGN(4096) : {
        *(.data)
        *(.data.*)
    }

    /* Uninitialized data (zero-initialized by boot) */
    .bss ALIGN(4096) : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /* Export kernel end address for memory manager */
    kernel_end = .;
}
