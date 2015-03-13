#include "pmm.h"
#include "string.h"

#include "dbg.h"

static uint32_t		_pmmgr_memory_size = 0;

static uint32_t		_pmmgr_used_blocks = 0;

static uint32_t		_pmmgr_max_blocks = 0;

static uint32_t*	_pmmgr_memory_map = 0;

inline void mmap_set(int bit)
{
	_pmmgr_memory_map[bit / 32] |= (1 << (bit % 32));
}

inline void mmap_unset(int bit)
{
	_pmmgr_memory_map[bit / 32] &= ~(1 << (bit % 32));
}

inline bool mmap_test(int bit)
{
	return _pmmgr_memory_map[bit / 32] & (1 << (bit % 32));
}

int mmap_first_free()
{
	//DbgPrintf("block count: %d.\n",pmmgr_get_block_count());
	//	find the first free bit
	for(uint32_t i = 0; i < pmmgr_get_block_count(); i ++ ){
		if(_pmmgr_memory_map[i] != 0xffffffff){
			for(int j = 0; j<32 ; j++){
				int bit = i << j;
				if(!(_pmmgr_memory_map[i] & bit)){
					return i*4*8 + j;
				}
			}
		}
	}
	return -1;
}

int mmap_first_free_s(size_t size)
{
	if (size==0)
		return -1;

	if (size==1)
		return mmap_first_free ();

	for (uint32_t i=0; i<pmmgr_get_block_count(); i++)
		if (_pmmgr_memory_map[i] != 0xffffffff)
			for (int j=0; j<32; j++) {	// test each bit in the dword

				int bit = 1<<j;
				if (! (_pmmgr_memory_map[i] & bit) ) {

					int startingBit = i*32;
					startingBit+=bit;		//get the free bit in the dword at index i

					uint32_t free=0; //loop through each bit to see if its enough space
					for (uint32_t count=0; count<=size;count++) {

						if (! mmap_test (startingBit+count) )
							free++;	// this bit is clear (free frame)

						if (free==size)
							return i*4*8+j; //free count==size needed; return index
					}
				}
			}
	return -1;
}


void pmmgr_init(size_t mem_size,physical_addr bitmap)
{
	_pmmgr_memory_size	= mem_size;
	_pmmgr_memory_map	= (uint32_t*) bitmap;
	_pmmgr_max_blocks	= (pmmgr_get_memory_size() * 1024) / PMMGR_BLOCK_SIZE;
	//DbgPrintf("init mem size %d block count: %d.\n",_pmmgr_memory_size,_pmmgr_max_blocks);
	_pmmgr_used_blocks	= pmmgr_get_block_count();

	//	by default, all of memory is in use
	memset(_pmmgr_memory_map, 0xf,pmmgr_get_block_count() / PMMGR_BLOCKS_PER_BYTE);
}

void pmmgr_init_region(physical_addr base,size_t size)
{
	int align = base / PMMGR_BLOCK_SIZE;
	int blocks = size / PMMGR_BLOCK_SIZE;

	for(; blocks > 0; blocks --){
		mmap_unset(align ++);
		_pmmgr_used_blocks --;
	}

	mmap_set(0);
}

void pmmgr_deinit_region(physical_addr base,size_t size)
{
	int align = base / PMMGR_BLOCK_SIZE;
	int blocks = size / PMMGR_BLOCK_SIZE;

	for(; blocks > 0 ; blocks --){
		mmap_set(align ++);
		_pmmgr_used_blocks ++;
	}
	mmap_set (0);	//first block is always set. This insures allocs cant be 0
}

void* pmmgr_alloc_block()
{
	if(pmmgr_get_free_block_count() <= 0){
		return 0;			//	out of memory
	}
	int frame = mmap_first_free();

	if(frame == -1){
		return 0;			//	out of memory
	}

	mmap_set(frame);

	physical_addr addr = frame * PMMGR_BLOCK_SIZE;
	_pmmgr_used_blocks ++;

	return (void*)addr;
}

void pmmgr_free_block(void* p)
{
	physical_addr addr = (physical_addr)p;
	int frame = addr / PMMGR_BLOCK_SIZE;

	mmap_unset(frame);

	_pmmgr_used_blocks --;
}

void* pmmgr_alloc_blocks(size_t size)
{
	if (pmmgr_get_free_block_count() <= size)
		return 0;	//not enough space

	int frame = mmap_first_free_s (size);

	if (frame == -1)
		return 0;	//not enough space

	for (uint32_t i=0; i<size; i++)
		mmap_set(frame+i);

	physical_addr addr = frame * PMMGR_BLOCK_SIZE;
	_pmmgr_used_blocks+=size;

	return (void*)addr;
}

void pmmgr_free_blocks(void* p, size_t size)
{
	physical_addr addr = (physical_addr)p;
	int frame = addr / PMMGR_BLOCK_SIZE;

	for (uint32_t i=0; i<size; i++)
		mmap_unset (frame+i);

	_pmmgr_used_blocks-=size;
}

size_t	pmmgr_get_memory_size()
{
	return _pmmgr_memory_size;
}

uint32_t pmmgr_get_block_count()
{
	return _pmmgr_max_blocks;
}

uint32_t pmmgr_get_use_block_count()
{
	return _pmmgr_used_blocks;
}

uint32_t pmmgr_get_free_block_count()
{
	return _pmmgr_max_blocks - _pmmgr_used_blocks;
}

uint32_t pmmgr_get_block_size()
{
	return PMMGR_BLOCK_SIZE;
}

void pmmgr_paging_enable(bool b)
{
	_asm {
		mov	eax, cr0
		cmp [b], 1
		je	enable
		jmp disable
enable:
		or eax, 0x80000000		//set bit 31
		mov	cr0, eax
		jmp done
disable:
		and eax, 0x7FFFFFFF		//clear bit 31
		mov	cr0, eax
done:
	}
}

bool pmmgr_is_paging()
{
	uint32_t res=0;
	_asm{
		mov	eax, cr0
		mov	[res], eax
	}
	return (res & 0x80000000) ? false : true;
}

void pmmgr_load_PDBR(physical_addr addr)
{
	_asm {
		mov	eax, [addr]
		mov	cr3, eax		// PDBR is cr3 register in i86
	}
}

physical_addr pmmgr_get_PDBR()
{
	_asm{
		mov	eax, cr3
		ret
	}
}