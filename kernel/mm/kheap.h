#pragma once
#include "kernel/types.h"

void  kheap_init(void);
void* kmalloc(size_t size);
void* kcalloc(size_t count, size_t size);
void  kfree(void* ptr);
