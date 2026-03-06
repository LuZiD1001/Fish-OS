#pragma once
#include "kernel/types.h"

void     timer_init(uint32_t hz);
void     timer_sleep(uint32_t ms);
uint64_t timer_get_ticks(void);
