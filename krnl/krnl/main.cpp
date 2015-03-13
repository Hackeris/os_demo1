#include "ctype.h"
#include "bootinfo.h"
#include "hal.h"
#include "vmm.h"
#include "kbd.h"
#include "exception.h"
#include "proc.h"
#include "syscall.h"
#include "pmm.h"
#include "vmm.h"

#include "dbg.h"

struct memory_region {

	uint32_t	startLo;	//base address
	uint32_t	startHi;
	uint32_t	sizeLo;		//length (in bytes)
	uint32_t	sizeHi;
	uint32_t	type;
	uint32_t	acpi_3_0;
};

//! different memory regions (in memory_region.type)
char* strMemoryTypes[] = {

	{"Available"},			//memory_region.type==0
	{"Reserved"},			//memory_region.type==1
	{"ACPI Reclaim"},		//memory_region.type==2
	{"ACPI NVS Memory"}		//memory_region.type==3
};

extern uint32_t	ticks;

extern uint16_t	kernelSize;

int kernel_main(boot_info* bootinfo)
{
	uint32_t memSize = 1024 + bootinfo->m_memoryLo + bootinfo->m_memoryHi*64;

	i86_hal_initialize();

	install_def_irq();

	//i86_vmm_initialize(bootinfo);
	DbgPrintf("Memory size: %d KBs(%d MB).\n",memSize,memSize / 1024);
	pmmgr_init(memSize,0x200000);
	memory_region* region = (memory_region*)0x1000;
	DbgPrintf("Memory map:\n");
	for(int i = 0; i<10 ; i ++){
		if(region[i].type > 4)
			break;
		if(i>0 && region[i].sizeLo == 0)
			break;
				//! display entry
		DbgPrintf ("  region %d: start: 0x%x%x length (bytes): 0x%x%x type: %d (%s)\n", i, 
			region[i].startHi, region[i].startLo,
			region[i].sizeHi,region[i].sizeLo,
			region[i].type, strMemoryTypes[region[i].type-1]);

		if(region[i].type == 1)		//	if region is available, init it for use(free)
			pmmgr_init_region(region[i].startLo,region[i].sizeLo);
	}
	//	for system use
	pmmgr_deinit_region (0,0x200000);

	//vmmngr_initialize();

	ticks = 0;
	init_syscall();

	init_proc();
	i86_initialize_clock_irq();
	task_start();

	return 0;
}