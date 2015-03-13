#ifndef _H_PMM
#define _H_PMM
#include "ctype.h"

#define PMMGR_BLOCKS_PER_BYTE		8

#define PMMGR_BLOCK_SIZE			4096

#define PMMGR_BLOCK_ALIGN			PMMGR_BLOCK_SIZE

typedef unsigned int	physical_addr;

int mmap_first_free();

int mmap_first_free_s(size_t size);

void pmmgr_init(size_t mem_size,physical_addr bitmap);

void pmmgr_init_region(physical_addr base,size_t size);

void pmmgr_deinit_region(physical_addr base,size_t size);

void* pmmgr_alloc_block();

void pmmgr_free_block(void* p);

void* pmmgr_alloc_blocks(size_t size);

void pmmgr_free_blocks(void* p, size_t size);

size_t pmmgr_get_memory_size();

uint32_t pmmgr_get_use_block_count();

uint32_t pmmgr_get_free_block_count();

uint32_t pmmgr_get_block_count();

uint32_t pmmgr_get_block_size();

void pmmgr_paging_enable(bool b);

bool pmmgr_is_paging();

void pmmgr_load_PDBR(physical_addr addr);

physical_addr pmmgr_get_PDBR();

#endif