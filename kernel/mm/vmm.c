/* Virtual Memory Manager stub */
#include "vmm.h"
#include "kernel/drivers/vga.h"

void vmm_init(void) {
    vga_printf("[VMM] Virtual memory manager initialized (identity mapped)\n");
}
