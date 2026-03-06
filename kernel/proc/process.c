/* ==============================================================================
   MyOS - Process Manager Stub (kernel/proc/process.c)
   Foundation for future multitasking. Currently single-process.
   ============================================================================== */

#include "process.h"
#include "kernel/drivers/vga.h"
#include "kernel/mm/kheap.h"
#include "libc/string.h"

#define MAX_PROCESSES 64

static process_t* process_table[MAX_PROCESSES];
static uint32_t   next_pid = 1;
static process_t* current_process = NULL;

void process_init(void) {
    memset(process_table, 0, sizeof(process_table));

    /* Create the kernel process (PID 0) */
    process_t* kernel_proc = (process_t*)kcalloc(1, sizeof(process_t));
    kernel_proc->pid   = 0;
    kernel_proc->state = PROCESS_RUNNING;
    strncpy(kernel_proc->name, "kernel", 31);

    process_table[0]  = kernel_proc;
    current_process   = kernel_proc;

    vga_printf("[PROC] Process manager initialized\n");
}

process_t* process_create(const char* name) {
    if (next_pid >= MAX_PROCESSES) return NULL;

    process_t* proc = (process_t*)kcalloc(1, sizeof(process_t));
    proc->pid   = next_pid++;
    proc->state = PROCESS_READY;
    strncpy(proc->name, name, 31);

    process_table[proc->pid] = proc;
    return proc;
}

process_t* process_current(void) {
    return current_process;
}
