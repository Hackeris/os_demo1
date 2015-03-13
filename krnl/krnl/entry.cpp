#include "bootinfo.h"
#include "proc.h"

#include "dbg.h"

uint16_t	kernelSize;

extern int _cdecl kernel_main(boot_info* bootinfo);

void kernel_entry(boot_info* bootinfo)
{
	_asm mov word ptr[kernelSize],dx

	_asm{
		cli
		mov ax,10h
		mov ds,ax
		mov es,ax
		mov fs,ax
		mov gs,ax
	}

	DbgCls(0x7);

	DbgPrintf("Kernel size: %d(sectors).\n",kernelSize);

	kernel_main(bootinfo);

hlt:
	goto hlt;
}