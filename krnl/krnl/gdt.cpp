#include "gdt.h"
#include "string.h"

#include "dbg.h"

GDT_DESCRIPTOR _gdt[MAX_DESCRIPTORS];

GDTR	_gdtr;

void gdt_set_descriptor(uint32_t i, uint32_t base, uint32_t limit, uint8_t access, uint8_t grand)
{
	if(i > MAX_DESCRIPTORS){
		return ;
	}

	memset((void*)&_gdt[i],0,sizeof(gdt_descriptor));

	_gdt[i].baseLo	= (uint16_t)(base & 0xffff);
	_gdt[i].baseMid	= (uint8_t)((base >> 16) & 0xff);
	_gdt[i].baseHi	= (uint8_t)((base >> 24) & 0xff);
	_gdt[i].limit	= (uint16_t)(limit & 0xffff);

	_gdt[i].flags = access;
	_gdt[i].grand = (uint8_t)((limit >> 16) & 0x0f);
	_gdt[i].grand |= grand & 0xf0;
}

void gdt_set_descriptor(uint32_t i, uint32_t base, uint32_t limit,uint16_t attr)
{
	gdt_set_descriptor(i,base,limit,attr & 0xff,(attr >> 8) & 0xf0);
	////DbgPrintf("idx: %d   base: 0x%x \n",i,base);
	//_gdt[i].baseLo	= (uint16_t)(base & 0xffff);
	//_gdt[i].baseMid	= (uint8_t)((base >>16) & 0xff);
	//_gdt[i].baseHi	= (uint8_t)((base >> 24) & 0xff);
	//_gdt[i].limit		= (uint16_t)(limit & 0xffff);
	//_gdt[i].flags		= (attr & 0xff);
	//_gdt[i].grand		= ((attr >> 8) & 0xf0) | (uint8_t)((limit >> 16) & 0x0f);
}

gdt_descriptor* i86_gdt_get_descriptor(int i)
{
	if (i > MAX_DESCRIPTORS)
		return 0;

	return &_gdt[i];
}

static void gdt_install()
{
	_asm lgdt [_gdtr]
}

int i86_gdt_initialize()
{
	_gdtr.m_limit = (sizeof(GDT_DESCRIPTOR) * MAX_DESCRIPTORS) - 1;
	_gdtr.m_base = (uint32_t) &_gdt[0];

	gdt_set_descriptor(0,0,0,0,0);

	/*gdt_set_descriptor(1,0,0xfffff,
		I86_GDT_DESC_READWRITE | I86_GDT_DESC_EXEC_CODE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY ,
		I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);

	gdt_set_descriptor(2,0,0xfffff,
		I86_GDT_DESC_READWRITE | I86_GDT_DESC_CODEDATA |I86_GDT_DESC_MEMORY ,
		I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);*/

	gdt_set_descriptor(1,0,0xfffff,
		DA_32 | DA_CR | DA_LIMIT_4K );
	gdt_set_descriptor(2,0,0xfffff,
		DA_32 | DA_DRW | DA_LIMIT_4K );

	gdt_install();

	return 0;
}