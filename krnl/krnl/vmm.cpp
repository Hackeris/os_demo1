#include "vmm.h"
#include "string.h"
#include "vmm_pde.h"
#include "vmm_pte.h"

#include "dbg.h"

extern PROCESS proc_table[];

// current directory table
pdirectory*		_cur_directory=0;

// current page directory base register
physical_addr	_cur_pdbr=0;

inline uint32_t vmmngr_ptable_virt_to_index(virtual_addr addr)
{
	// return index only if address doesnt exceed page table address space size
	return (addr >= PTABLE_ADDR_SPACE_SIZE) ? 0 : addr/PAGE_SIZE;
}

inline pt_entry* vmmngr_ptable_lookup_entry(ptable* p,virtual_addr addr)
{
	if (p)
		return &p->m_entries[vmmngr_ptable_virt_to_index (addr) ];
	return 0;
}

inline void vmmngr_ptable_clear(ptable* p)
{
	if (p)
		memset ( p,0,sizeof (ptable) );
}

inline void vmmngr_pdirectory_clear(pdirectory* dir)
{
	if (dir)
		memset ( dir,0,sizeof (pdirectory) );
}

inline uint32_t vmmngr_pdirectory_virt_to_index(virtual_addr addr)
{
	// return index only if address doesnt exceed 4gb (page directory address space size)
	return (addr >= DTABLE_ADDR_SPACE_SIZE) ? 0 : addr/PAGE_SIZE;
}

inline pd_entry* vmmngr_pdirectory_lookup_entry(pdirectory* p, virtual_addr addr)
{
	if (p)
		return &p->m_entries[ vmmngr_ptable_virt_to_index (addr) ];
	return 0;
}

inline bool vmmngr_switch_pdirectory(pdirectory* dir)
{
	if (!dir)
		return false;
	_cur_directory = dir;
	pmmgr_load_PDBR (_cur_pdbr);
	return true;
}

void vmmngr_flush_tlb_entry (virtual_addr addr)
{
	_asm {
		cli
		invlpg	addr
		sti
	}
}

pdirectory* vmmngr_get_directory()
{
	return _cur_directory;
}

bool vmmngr_alloc_page(pt_entry* e)
{
	// allocate a free physical frame
	void* p = pmmgr_alloc_block ();
	if (!p)
		return false;

	// map it to the page
	pt_entry_set_frame (e, (physical_addr)p);
	pt_entry_add_attrib (e, I86_PTE_PRESENT);

	return true;
}

void vmmngr_free_page (pt_entry* e)
{
	void* p = (void*)pt_entry_pfn (*e);
	if (p)
		pmmgr_free_block (p);

	pt_entry_del_attrib (e, I86_PTE_PRESENT);
}


#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xfff)

void vmmngr_initialize()
{
	uint32_t* page_dir_base = (uint32_t*)pmmgr_alloc_block();
	uint32_t* page_tbl_base;

	uint32_t	mem_size = pmmgr_get_memory_size();
	//mem_size = 5 * 1024;

	uint32_t tbl_count = mem_size / 4096;	//	4MB every page table can refer
	uint32_t i = 0,count;
	if(mem_size % 4096 != 0){
		tbl_count ++;
	}
	page_tbl_base = (uint32_t*)pmmgr_alloc_blocks(tbl_count);
	//	initialize page directories
	count = tbl_count;
	while(count --){
		((uint32_t*)page_dir_base)[i] = (uint32_t)(page_tbl_base + i * 4096);
		((uint32_t*)page_dir_base)[i] |= I86_PDE_PRESENT | I86_PDE_USER |I86_PDE_WRITABLE | I86_PDE_USER;
		i++;
	}
	//	initialize page tables
	count = tbl_count * 1024;
	i = 0;
	while(count --){
		((uint32_t*)page_tbl_base)[i] = (i * 4096) | 
			I86_PTE_PRESENT | I86_PTE_USER |I86_PTE_WRITABLE | I86_PDE_USER;
		i++;
	}
	// store current PDBR
	_cur_pdbr = (physical_addr)page_dir_base;

	// switch to our page directory
	vmmngr_switch_pdirectory ((pdirectory*)_cur_pdbr);

	// enable paging
	pmmgr_paging_enable (true);
}

uint32_t ldt_seg_linear(PROCESS* p,int idx)
{
	LDT_DESCRIPTOR* d = &p->ldts[idx];
	return (d->baseHi << 24) | (d->baseMid << 16) | (d->baseLo);
}

void* va2la(int pid,void* va)
{
	PROCESS* p = &proc_table[pid];

	uint32_t seg_base = ldt_seg_linear(p,INDEX_LDT_RW);
	uint32_t la = seg_base + (uint32_t)va;

	if(pid < NR_TASKS + NR_NATIVE_PROCS){
		//	assert(la == (uint32_t)val);
	}

	return (void*)la;
}