#pragma once
#include "kernel/types.h"

typedef enum {
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_DEAD,
} process_state_t;

typedef struct {
    uint32_t        pid;
    char            name[32];
    process_state_t state;
    uint32_t        esp;        /* Saved stack pointer */
    uint32_t        eip;        /* Saved instruction pointer */
    uint32_t        cr3;        /* Page directory */
} process_t;

void       process_init(void);
process_t* process_create(const char* name);
process_t* process_current(void);
