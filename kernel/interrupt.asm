; ==============================================================================
;  MyOS - Interrupt Stubs (kernel/interrupt.asm)
;  CPU exceptions and IRQ handlers that call into C
; ==============================================================================

BITS 32

extern isr_handler   ; C handler for CPU exceptions
extern irq_handler   ; C handler for hardware IRQs

; Common ISR stub - saves CPU state, calls C handler, restores state
isr_common_stub:
    pusha               ; Save EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    mov ax, ds
    push eax            ; Save data segment
    mov ax, 0x10        ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp            ; Pass pointer to registers struct
    call isr_handler
    pop eax
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8          ; Clean up error code + interrupt number
    iret

; Common IRQ stub
irq_common_stub:
    pusha
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp
    call irq_handler
    pop eax
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8
    iret

; Macro: ISR without error code (CPU doesn't push one - we push dummy 0)
%macro ISR_NOERR 1
global isr%1
isr%1:
    cli
    push byte 0     ; dummy error code
    push byte %1    ; interrupt number
    jmp isr_common_stub
%endmacro

; Macro: ISR with error code (CPU already pushed one)
%macro ISR_ERR 1
global isr%1
isr%1:
    cli
    push byte %1
    jmp isr_common_stub
%endmacro

; Macro: IRQ handler
%macro IRQ 2
global irq%1
irq%1:
    cli
    push byte 0
    push byte %2
    jmp irq_common_stub
%endmacro

; CPU Exception handlers (0-31)
ISR_NOERR 0   ; Division by zero
ISR_NOERR 1   ; Debug
ISR_NOERR 2   ; Non-maskable interrupt
ISR_NOERR 3   ; Breakpoint
ISR_NOERR 4   ; Overflow
ISR_NOERR 5   ; Bound range exceeded
ISR_NOERR 6   ; Invalid opcode
ISR_NOERR 7   ; Device not available
ISR_ERR   8   ; Double fault (has error code)
ISR_NOERR 9   ; Coprocessor segment overrun
ISR_ERR   10  ; Invalid TSS
ISR_ERR   11  ; Segment not present
ISR_ERR   12  ; Stack-segment fault
ISR_ERR   13  ; General protection fault
ISR_ERR   14  ; Page fault
ISR_NOERR 15  ; Reserved
ISR_NOERR 16  ; x87 FPU error
ISR_NOERR 17  ; Alignment check
ISR_NOERR 18  ; Machine check
ISR_NOERR 19  ; SIMD FP exception

; Hardware IRQ handlers (mapped to interrupts 32-47)
IRQ  0, 32   ; Timer (PIT)
IRQ  1, 33   ; Keyboard
IRQ  2, 34   ; Cascade
IRQ  3, 35   ; COM2
IRQ  4, 36   ; COM1
IRQ  5, 37   ; LPT2
IRQ  6, 38   ; Floppy disk
IRQ  7, 39   ; LPT1
IRQ  8, 40   ; CMOS RTC
IRQ  9, 41   ; Free
IRQ 10, 42   ; Free
IRQ 11, 43   ; Free
IRQ 12, 44   ; PS/2 mouse
IRQ 13, 45   ; FPU
IRQ 14, 46   ; Primary ATA
IRQ 15, 47   ; Secondary ATA
