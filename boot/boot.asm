; ==============================================================================
;  MyOS - Bootloader (boot/boot.asm)
;  Multiboot-compliant entry point. GRUB loads this first.
;  Sets up the stack and jumps into the C kernel.
; ==============================================================================

BITS 32

; Multiboot header constants
MULTIBOOT_MAGIC     equ 0x1BADB002
MULTIBOOT_FLAGS     equ 0x00000003   ; align modules + memory map
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

; Stack size: 16 KB
STACK_SIZE equ 0x4000

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

section .bss
align 16
stack_bottom:
    resb STACK_SIZE
stack_top:

section .text
global _start
extern kernel_main

_start:
    ; Set up our own stack (GRUB doesn't guarantee one)
    mov esp, stack_top

    ; Push multiboot info pointer and magic for kernel_main(uint32_t magic, uint32_t* mbi)
    push ebx    ; multiboot info struct pointer
    push eax    ; multiboot magic number

    ; Call the C kernel
    call kernel_main

    ; If kernel_main returns, hang the CPU
.hang:
    cli         ; Disable interrupts
    hlt         ; Halt
    jmp .hang   ; Safety loop
