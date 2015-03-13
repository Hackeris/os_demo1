#ifndef _H_GDT
#define _H_GDT
#include "ctype.h"

#define MAX_DESCRIPTORS		256

#define		DA_32			0x4000
#define		DA_LIMIT_4K		0x8000

#define		DA_DPL0			0x00
#define		DA_DPL1			0x20
#define		DA_DPL2			0x40
#define		DA_DPL3			0x60

#define		DA_DR			0x90
#define		DA_DRW			0x92
#define		DA_DRWA			0x93
#define		DA_C			0x98
#define		DA_CR			0x9a
#define		DA_CCO			0x9c
#define		DA_CCOR			0x9e

#define		DA_LDT				0x82
#define		DA_TASK_GATE		0x85
#define		DA_386TSS			0x89
#define		DA_386CGATE			0x8c
#define		DA_386IGATE			0x8e
#define		DA_386TGATE			0x8f

#define		SA_RPL0				0
#define		SA_RPL1				1
#define		SA_RPL2				2
#define		SA_RPL3				3

#define		SA_TIG				0
#define		SA_TIL				4

//! set access bit
#define I86_GDT_DESC_ACCESS			0x0001			//00000001
//! descriptor is readable and writable. default: read only
#define I86_GDT_DESC_READWRITE			0x0002			//00000010
//! set expansion direction bit
#define I86_GDT_DESC_EXPANSION			0x0004			//00000100
//! executable code segment. Default: data segment
#define I86_GDT_DESC_EXEC_CODE			0x0008			//00001000
//! set code or data descriptor. defult: system defined descriptor
#define I86_GDT_DESC_CODEDATA			0x0010			//00010000
//! set dpl bits
#define I86_GDT_DESC_DPL			0x0060			//01100000
//! set "in memory" bit
#define I86_GDT_DESC_MEMORY			0x0080			//10000000

/**	gdt descriptor grandularity bit flags	***/
//! masks out limitHi (High 4 bits of limit)
#define I86_GDT_GRAND_LIMITHI_MASK		0x0f			//00001111
//! set os defined bit
#define I86_GDT_GRAND_OS			0x10			//00010000
//! set if 32bit. default: 16 bit
#define I86_GDT_GRAND_32BIT			0x40			//01000000
//! 4k grandularity. default: none
#define I86_GDT_GRAND_4K			0x80			//10000000

typedef struct gdt_descriptor {
	//! bits 0-15 of segment limit
	uint16_t		limit;
	//! bits 0-23 of base address
	uint16_t		baseLo;
	uint8_t			baseMid;
	//! descriptor access flags
	uint8_t			flags;
	uint8_t			grand;
	//! bits 24-32 of base address
	uint8_t			baseHi;
}GDT_DESCRIPTOR;

typedef struct gdtr{
	//! size of gdt
	uint16_t		m_limit;
	//! base address of gdt
	uint32_t		m_base;
}GDTR;

gdt_descriptor* i86_gdt_get_descriptor(int i);

void gdt_set_descriptor(uint32_t i, uint32_t base, uint32_t limit, uint8_t access, uint8_t grand);

void gdt_set_descriptor(uint32_t i, uint32_t base, uint32_t limit,uint16_t attr);

static void gdt_install();

int i86_gdt_initialize();

#endif