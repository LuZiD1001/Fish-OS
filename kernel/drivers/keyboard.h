#pragma once
#include "kernel/types.h"

void keyboard_init(void);
char keyboard_getchar(void);
int  keyboard_readline(char* buf, int max);
