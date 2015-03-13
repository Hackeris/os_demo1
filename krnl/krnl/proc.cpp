#include "proc.h"
#include "gdt.h"
#include "string.h"
#include "idt.h"
#include "pic.h"
#include "klib.h"
#include "hal.h"
#include "syscall.h"
#include "pit.h"
#include "tty.h"
#include "stdio.h"
#include "ipc.h"
#include "flpy.h"
#include "config.h"
#include "fs.h"
#include "sys.h"

#include "dbg.h"

PROCESS	proc_table[NR_TASKS+NR_PROCS];
TASK	task_table[NR_TASKS]={
	{task_tty,STACK_SIZE_TTY,"tty"},
	{task_sys,STACK_SIZE_SYS,"sys"},
	{task_flpy,STACK_SIZE_FLPY,"flpy"},
	{task_fs,STACK_SIZE_FS,"fsys"}
};
TASK	user_proc_table[NR_NATIVE_PROCS]={
	{TestA,STACK_SIZE_TESTA,"TestA"},
	{Init,STACK_SIZE_INIT,"Init"}
};

PROCESS* p_proc_ready;

extern GDT_DESCRIPTOR _gdt[];

TSS	tss;

char* task_stack = (char*)TASK_STACK;

extern	int32_t		k_reenter;

uint32_t	ticks;

extern TTY tty_table[];

struct{
	char stack[2 * 1024];
	char stack_top;
}stack_top;

uint16_t	ldtr;

void TestA()
{
	int i = 0;
	while(1){
		//printf("A%d",i);
		i++;
		if(i >= 127) exit(0);
		milli_delay(1000);
	}
}

void Init()
{
	int i = 0;
	printf("Init\n");
	while(1){
		milli_delay(1);
	}
}

void delay(uint32_t t)
{
	int  j ,k;
	for(k=0;k < t;k++){
		for(j=0;j<100;j++){
			_asm nop
		}
	}
}

void ldt_set_descriptor(LDT_DESCRIPTOR* desc,uint32_t base, uint32_t limit,uint16_t attr)
{
	desc->baseLo	= (uint16_t)(base & 0xffff);
	desc->baseMid	= (uint8_t)((base >>16) & 0xff);
	desc->baseHi	= (uint8_t)((base >> 24) & 0xff);
	desc->limit		= (uint16_t)(limit & 0xffff);
	desc->flags		= (attr & 0xff);
	desc->grand		= ((attr >> 8) & 0xf0) | (uint8_t)((limit >> 16) & 0x0f);
}

void init_proc()
{
	i86_tss_initialize();
	i86_ldt_initialize();
	TASK*	p_task = task_table;
	PROCESS*	p_proc = proc_table;
	char*	p_task_stack = task_stack + STACK_SIZE_TOTAL;
	uint16_t	selector_ldt = SELECTOR_LDT_FIRST;

	uint8_t		privilage;
	uint8_t		rpl;
	int32_t		eflags;

	int i;
	for(i=0;i<NR_TASKS+NR_PROCS;i++){
		if(i >= NR_TASKS+NR_NATIVE_PROCS){
			p_proc->p_flags = FREE_SLOT;
			p_proc ++;
			continue;
		}

		if(i<NR_TASKS){
			p_task = task_table + i;
			privilage = PRIVILEGE_TASK;
			rpl = RPL_TASK;
			eflags = 0x1202;
		}
		else{
			p_task = user_proc_table + (i - NR_TASKS);
			privilage = PRIVILEGE_USER;
			rpl = RPL_USER;
			eflags = 0x202;
		}

		strcpy(p_proc->p_name,p_task->name);
		p_proc->pid = i;
		
		p_proc->ldt_sel = selector_ldt;
		memcpy(&p_proc->ldts[0],&_gdt[1],sizeof(LDT_DESCRIPTOR));
		p_proc->ldts[0].flags = (DA_C | privilage << 5);
		memcpy(&p_proc->ldts[1],&_gdt[2],sizeof(LDT_DESCRIPTOR));
		p_proc->ldts[1].flags = (DA_DRW | privilage << 5);

		p_proc->regs.cs = (8*0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds = (8*1 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es = (8*1 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs = (8*1 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs = (8*1 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss = (8*1 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;

		p_proc->regs.eip = (uint32_t)p_task->initial_eip;
		p_proc->regs.esp = (uint32_t)p_task_stack;
		p_proc->regs.eflags = eflags;

		memset(&p_proc->file_desc,INVALID_FILE_DESC,sizeof(int)*MAX_FILES_PER_PROCESS);

		p_proc->nr_tty = 0;

		p_proc->p_flags = 0;

		p_task_stack -= p_task->stack_size;
		p_proc ++;
		p_task ++;
		selector_ldt += (1 << 3);
	}

	proc_table[5].nr_tty = 1;

	p_proc_ready = proc_table;
}

int i86_ldt_initialize()
{
	PROCESS* p_proc = proc_table;
	uint16_t	selector_ldt = SELECTOR_LDT_FIRST;
	int i;
	for(i=0;i<NR_TASKS+NR_PROCS;i++){
		proc_table[i].ldt_sel = selector_ldt;
		gdt_set_descriptor(selector_ldt >> 3,(uint32_t)&p_proc->ldts,
			LDT_SIZE * sizeof(LDT_DESCRIPTOR) - 1,DA_LDT);
		p_proc ++;
		selector_ldt += (1 << 3);
	}
	
	return 0;
}

int i86_tss_initialize()
{
	memset(&tss,0,sizeof(tss));
	tss.ss0 = SELECTOR_KERNEL_DS;
	tss.cs = SELECTOR_KERNEL_CS;
	gdt_set_descriptor(SELECTOR_TSS >> 3,(uint32_t)&tss,
		sizeof(tss) - 1,DA_386TSS);
	tss.iobase = sizeof(tss);

	_asm{
		xor	eax,eax
		mov ax,SELECTOR_TSS
		ltr	ax
	}

	return 0;
}

int	i86_initialize_clock_irq()
{
	uint8_t	state;
	setvect(32,clock_irq);

	//out_byte(TIMER_MODE,RATE_GENERATOR);
	//out_byte(TIMER0,(uint8_t)(TIMER_FREQ / HZ));
	//out_byte(TIMER0,(uint8_t)(TIMER_FREQ / HZ) >> 8);

	state = in_byte(INT_M_CTLMASK);
	state &= 0xfe;
	//	master OCW1
	out_byte(INT_M_CTLMASK,state);
	return 0;
}

void interrupt restart()
{
	ldtr = p_proc_ready->ldt_sel;
	_asm{
		mov	esp,[p_proc_ready]
		lldt [ldtr]
	}
	tss.esp0 =(uint32_t) & p_proc_ready->ldt_sel;	//	esp0 in tss point to stack frame
	k_reenter --;
	_asm{
		pop	gs
		pop	fs
		pop	es
		pop	ds
		popad

		add	esp,4
	}
	_asm iretd
}

void task_start()
{
	restart();
}

void interrupt restart_reenter()
{
	k_reenter --;
	_asm{
		pop	gs
		pop	fs
		pop	es
		pop	ds
		popad

		add	esp,4
	}
	_asm iretd
}

uint32_t	save_ret_addr;

void __declspec (naked) _cdecl _save()
{
	_asm mov eax,[esp]
	_asm add esp,4
	_asm mov save_ret_addr,eax
	_asm{
		push ds
		push es
		push fs
		push gs

		mov esi,edx

		mov dx,ss
		mov ds,dx
		mov es,dx

		mov edx,esi

		mov esi,esp
	}
	//	re-enable interrupt
	k_reenter ++;
	if(k_reenter == 0){
		_asm{
			lea esp,[stack_top.stack_top]	//	switch to kernel stack
			push restart
		}
	}
	else{
		_asm{
			push restart_reenter
		}
	}
	_asm mov eax,save_ret_addr
	_asm jmp eax
}

void interrupt __cdecl clock_irq()
{
	save();
	//	save registers
	_asm{
		in	al,INT_M_CTLMASK
		or	al,1
		out	INT_M_CTLMASK,al

		mov al,EOI
		out INT_M_CTL,al
	}
	_asm sti
	clock_handler(0);					//	handle clock irq
	_asm cli
	_asm{
		in	al,INT_M_CTLMASK
		and	al,0xfe
		out	INT_M_CTLMASK,al
	}
	_asm ret
}

void clock_handler(int irq)
{
	ticks ++;
	if(k_reenter != 0){
		return;
	}
	schedule();
}

void schedule()
{
	int p = (int)p_proc_ready;
	do{
		p_proc_ready ++;
		if((p_proc_ready >= proc_table+NR_TASKS+NR_PROCS)){
			p_proc_ready = proc_table;
		}
		if(p_proc_ready->p_recvfrom == NO_TASK) p_proc_ready->p_flags&= ~RECEIVE;
	}while(p_proc_ready->p_flags != 0);
	//if(p_proc_ready->pid == 5)DbgPrintf("f%d",p_proc_ready->p_flags);
	//if(proc_table[4].p_flags & HANGING)DbgPrintf("H");
	//DbgPrintf("%d ",proc_table[4].p_flags);
	//DbgPrintf("%d",proc_table[6].p_flags);
	//if(p != (int)p_proc_ready)DbgPrintf("%d",p_proc_ready->pid);
}

void milli_delay(int32_t milli_sec)
{
	int t = get_ticks();
	while(((get_ticks() - t) * 1000 / HZ) < milli_sec){}
}

extern uint16_t	kernelSize;
void get_kernel_map(uint32_t* base,uint32_t* limit)
{
	*base = KERNEL_ADDR;
	*limit = kernelSize*512 - 1;
}