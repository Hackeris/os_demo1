#include "hal.h"
#include "pic.h"

void _cdecl setvect(int intno, I86_IRQ_HANDLER vect)
{
	init_idt_desc(intno,vect,DA_386IGATE,PRIVILEGE_KRNL,0x8);
}

int _cdecl i86_hal_initialize()
{
	disable_interrupt();

	i86_cpu_initialize();
	i86_pic_initialize();

	enable_interrupt();
	return 0;
}