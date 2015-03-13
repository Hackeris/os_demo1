#ifndef _H_GATE
#define _H_GATE
#include "ctype.h"
#include "gdt.h"

typedef struct gate{
	uint16_t	offsetLo;
	uint16_t	sel;
	uint16_t	attr;
	uint16_t	offsetHi;
}GATE,*PGATE;

void i86_set_gate(PGATE gt,uint16_t sel,uint32_t offset,uint8_t dcount,uint8_t attr);

#endif