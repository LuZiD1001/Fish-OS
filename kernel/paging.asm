; Paging stubs - to be expanded with full page table management
BITS 32
global paging_enable
global paging_load_dir

paging_load_dir:
    mov eax, [esp+4]
    mov cr3, eax
    ret

paging_enable:
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax
    ret
