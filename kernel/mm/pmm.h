#pragma once
#include "kernel/types.h"

void  pmm_init(uint32_t mem_upper_kb, uint32_t kernel_end);
void* pmm_alloc(void);
void  pmm_free(void* addr);
uint32_t pmm_get_free_pages(void);
uint32_t pmm_get_total_pages(void);
