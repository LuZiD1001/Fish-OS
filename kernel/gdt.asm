; ==============================================================================
;  MyOS - GDT Load (kernel/gdt.asm)
;  Loads the Global Descriptor Table and reloads segment registers
; ==============================================================================

global gdt_flush
global tss_flush

; void gdt_flush(uint32_t gdt_ptr)
; Loads GDT from a GDT pointer struct
gdt_flush:
    mov eax, [esp+4]
    lgdt [eax]

    ; Reload segment registers with kernel data selector (0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Far jump to reload CS with kernel code selector (0x08)
    jmp 0x08:.flush
.flush:
    ret

; void tss_flush()
; Loads the Task State Segment (used for ring3 → ring0 stack switch)
tss_flush:
    mov ax, 0x2B   ; TSS selector (index 5, RPL=3)
    ltr ax
    ret
