#ifndef _H_VMM
#define _H_VMM
#include "ctype.h"
#include "bootinfo.h"
#include "proc.h"
#include "pmm.h"
#include "vmm_pde.h"
#include "vmm_pte.h"

// virtual address
typedef uint32_t virtual_addr;

// i86 architecture defines 1024 entries per table--do not change
#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR	1024

// page table represents 4mb address space
#define PTABLE_ADDR_SPACE_SIZE 0x400000

// directory table represents 4gb address space
#define DTABLE_ADDR_SPACE_SIZE 0x100000000

// page sizes are 4k
#define PAGE_SIZE 4096

struct ptable{
	pt_entry m_entries[PAGES_PER_TABLE];
};

// page directory
struct pdirectory{
	pd_entry m_entries[PAGES_PER_DIR];
};

void vmmngr_initialize();

uint32_t ldt_seg_linear(PROCESS* p,int idx);

void* va2la(int pid,void* va);

#endif