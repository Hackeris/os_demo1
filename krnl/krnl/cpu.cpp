#include "cpu.h"

extern int32_t	k_reenter;

int i86_cpu_initialize()
{
	k_reenter = 0;
	i86_gdt_initialize();
	i86_idt_initialize( 0x8 );
	return 0;
}