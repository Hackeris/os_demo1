#include "klib.h"
#include "pic.h"

void disable_interrupt()
{
	_asm cli
}

void enable_interrupt()
{
	_asm sti
}

void out_byte(uint16_t port,uint8_t val)
{
	_asm{
		xor edx,edx
		xor eax,eax
		mov dx,[port]
		mov al,[val]
		out	dx,al
		nop
	}
}

uint8_t in_byte(uint16_t port)
{
	_asm{		
		xor edx,edx
		xor eax,eax
		mov dx,[port]
		xor	eax,eax
		in	al,dx
		nop
	}
}

void enable_irq(int irq)
{
	if(irq < 8){
		out_byte(INT_M_CTLMASK,in_byte(INT_M_CTLMASK)& ~(1 << irq));
	}
	else{
		out_byte(INT_S_CTLMASK,in_byte(INT_S_CTLMASK)& ~(1 << irq));
	}
}

void disable_irq(int irq)
{
	if(irq < 8){
		out_byte(INT_M_CTLMASK,in_byte(INT_M_CTLMASK)| (1 << irq));
	}
	else{
		out_byte(INT_S_CTLMASK,in_byte(INT_S_CTLMASK)| (1 << irq));
	}
}