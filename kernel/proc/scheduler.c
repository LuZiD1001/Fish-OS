/* Scheduler stub - round-robin scheduler to be implemented */
#include "scheduler.h"
#include "kernel/drivers/vga.h"

void scheduler_init(void) {
    vga_printf("[SCHED] Scheduler ready (cooperative mode)\n");
}

void scheduler_tick(void) {
    /* Future: preemptive context switch here */
}
