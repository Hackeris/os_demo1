#include "idt.h"
#include "dbg.h"

GATE	idt[IDT_SIZE];

IDTR	_idtr;

//	times re-enter interrupt (interrupt occured when in inperrupt)
int32_t	k_reenter;

void i86_default_handler () {
	disable_interrupt();
	DbgPrintf("interrupt\n");
	for(;;);					//	halt
}

void init_idt_desc(uint32_t i,I86_IRQ_HANDLER handler,uint8_t type,uint8_t privilege,uint16_t codeSel)
{
	if(i >= IDT_SIZE){
		return ;
	}
	i86_set_gate(&idt[i],codeSel,(uint32_t)handler,0,type | (privilege << 5));
}

int i86_idt_initialize(uint16_t codeSel)
{
	_idtr.limit = sizeof(GATE) * IDT_SIZE - 1;
	_idtr.base	= (uint32_t) &idt[0];

	memset((void*)&idt[0],0,sizeof(GATE) * IDT_SIZE);

	for(int i = 0;i < IDT_SIZE;i++){
		init_idt_desc(i,i86_default_handler,DA_386IGATE,PRIVILEGE_KRNL,codeSel);
	}

	idt_install();
}

void idt_install()
{
	_asm lidt [_idtr]
}