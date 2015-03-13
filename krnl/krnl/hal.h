#ifndef _H_HAL
#define _H_HAL
#include "ctype.h"
#include "cpu.h"

int _cdecl i86_hal_initialize();

void _cdecl setvect(int intno, I86_IRQ_HANDLER vect);

#endif