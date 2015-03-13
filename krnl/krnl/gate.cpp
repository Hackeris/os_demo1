#include "gate.h"

void i86_set_gate(PGATE gt,uint16_t sel,uint32_t offset,uint8_t dcount,uint8_t attr)
{
	gt->offsetLo = offset & 0xffff;
	gt->sel = sel;
	gt->attr = (dcount & 0x1f) | ((attr << 8) & 0xff00);
	gt->offsetHi = (offset >> 16) & 0xffff;
}