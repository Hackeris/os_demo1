#ifndef _H_PROC
#define _H_PROC
#include "ctype.h"
#include "gdt.h"
#include "msg.h"
#include "fs.h"

#ifndef interrupt
#define interrupt __declspec (naked)
#endif

#define		LDT_SIZE	2

#define		INDEX_LDT_RW	1

#define		NR_TASKS			4
#define		NR_NATIVE_PROCS		2
#define		NR_PROCS			32

#define		SA_TI_MASK		0xfffb
#define		SA_RPL_MASK		0xfffc

#define		SELECTOR_KERNEL_CS		0x08
#define		SELECTOR_KERNEL_DS		0x10

#define		SELECTOR_LDT_FIRST		0x20
#define		SELECTOR_LDT_SECOND		0x28
#define		SELECTOR_TSS			0x18

#define	LIMIT_4K_SHIFT		  12

#define	RPL_KRNL	SA_RPL0
#define	RPL_TASK	SA_RPL1
#define	RPL_USER	SA_RPL3

/* Process */
#define SENDING			0x02	/* set when proc trying to send */
#define RECEIVING		0x04	/* set when proc trying to recv */
#define FREE_SLOT		0x08
#define WAITING			0x10
#define HANGING			0x20


#define ANY_TASK	(NR_TASKS + NR_PROCS + 10)
#define NO_TASK		(NR_TASKS + NR_PROCS + 20)

/* tasks */
#define INVALID_DRIVER	-20
#define INTERRUPT	-10

#define TASK_TTY	0
#define TASK_SYS	1
#define TASK_FLPY	2
#define TASK_FS		3

#define INIT		5

#define proc2pid(proc) (proc-proc_table)

#define save()	_asm{sub esp,4}\
				_asm{pushad}\
				_save();

typedef void (*PTASK_FUN)();


#define STACK_SIZE_TESTA	0x500
//#define STACK_SIZE_TESTB	0x500
#define STACK_SIZE_INIT		0x500

#define STACK_SIZE_TTY		0x1000
#define STACK_SIZE_SYS		0x1000
#define STACK_SIZE_FLPY		0x1000
#define STACK_SIZE_FS		0x1000

#define STACK_SIZE_TOTAL	STACK_SIZE_TESTA+STACK_SIZE_INIT+\
						STACK_SIZE_TTY+STACK_SIZE_SYS+STACK_SIZE_FLPY+STACK_SIZE_FS

#define	PROCS_BASE					0xA00000 /* 10 MB */
#define	PROC_IMAGE_SIZE_DEFAULT		0x100000 /*  1 MB */
#define	PROC_ORIGIN_STACK			0x400    /*  1 KB */

typedef struct _tagTASK{
	PTASK_FUN	initial_eip;
	uint32_t	stack_size;
	char		name[20];
}TASK;

typedef struct _tagLDT_DESCRIPTOR{
	uint16_t		limit;
	uint16_t		baseLo;
	uint8_t			baseMid;
	uint8_t			flags;
	uint8_t			grand;
	uint8_t			baseHi;
}LDT_DESCRIPTOR;

typedef struct _tagTSS{
	uint32_t	backlink;
	uint32_t	esp0;
	uint32_t	ss0;
	uint32_t	esp1;
	uint32_t	ss1;
	uint32_t	esp2;
	uint32_t	ss2;
	uint32_t	cr3;
	uint32_t	eip;
	uint32_t	flags;
	uint32_t	eax;
	uint32_t	ecx;
	uint32_t	edx;
	uint32_t	ebx;
	uint32_t	esp;
	uint32_t	ebp;
	uint32_t	esi;
	uint32_t	edi;
	uint32_t	es;
	uint32_t	cs;
	uint32_t	ss;
	uint32_t	ds;
	uint32_t	fs;
	uint32_t	gs;
	uint32_t	ldt;
	uint32_t	trap;
	uint32_t	iobase;
}TSS;
typedef struct _tagSTACK_FRAME{
	uint32_t	gs;
	uint32_t	fs;
	uint32_t	es;
	uint32_t	ds;
	uint32_t	edi;
	uint32_t	esi;
	uint32_t	ebp;
	uint32_t	kernel_esp;
	uint32_t	ebx;
	uint32_t	edx;
	uint32_t	ecx;
	uint32_t	eax;
	uint32_t	retaddr;
	uint32_t	eip;
	uint32_t	cs;
	uint32_t	eflags;
	uint32_t	esp;
	uint32_t	ss;
}STACK_FRAME;

typedef	struct _tagPROC{
	STACK_FRAME regs;
	uint16_t	ldt_sel;
	LDT_DESCRIPTOR	ldts[LDT_SIZE];
	uint32_t	pid;
	char	p_name[20];

	int		nr_tty;

	int p_flags;
	MESSAGE* p_msg;
	int p_recvfrom;
	int p_sendto;
	int has_int_msg;

	struct _tagPROC* q_sending;
	struct _tagPROC* next_sending;

	int file_desc[MAX_FILES_PER_PROCESS];

	int p_parent;

	void* base;
	int blocks;

	int exit_code;
}PROCESS;

void TestA();
void TestB();
void Init();

void delay(uint32_t t);

void __cdecl clock_irq();

int i86_ldt_initialize();

void init_proc();

int i86_tss_initialize();

int	i86_initialize_clock_irq();

void ldt_set_descriptor(LDT_DESCRIPTOR* desc,uint32_t base, uint32_t limit,uint16_t attr);

void restart();

void task_start();

void clock_handler(int irq);

int i86_enable_clock_interrupt();

void _save();

void restart_reenter();

void milli_delay(int32_t milli_sec);

void schedule();

void get_kernel_map(uint32_t* base,uint32_t* limit);


#endif
