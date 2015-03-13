#ifndef _H_IDT
#define _H_IDT
#include "ctype.h"
#include "gate.h"
#include "string.h"
#include "klib.h"

#define		IDT_SIZE		256

/* privilege */
#define	PRIVILEGE_KRNL	0
#define	PRIVILEGE_TASK	1
#define	PRIVILEGE_USER	3

typedef void (_cdecl *I86_IRQ_HANDLER)(void);

//! describes the structure for the processors idtr register
typedef struct idtr{
	uint16_t	limit;
	uint32_t	base;
}IDTR;

void i86_default_handler();

void init_idt_desc(uint32_t i,I86_IRQ_HANDLER handler,uint8_t type,uint8_t privilege,uint16_t codeSel);

int i86_idt_initialize(uint16_t codeSel);

void idt_install();

#endif