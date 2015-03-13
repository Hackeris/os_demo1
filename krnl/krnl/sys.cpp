#include "sys.h"
#include "gdt.h"
#include "proc.h"
#include "ipc.h"
#include "string.h"
#include "vmm.h"
#include "pmm.h"
#include "idt.h"
#include "stdio.h"

#include "dbg.h"

extern PROCESS proc_table[];

extern uint32_t	ticks;

void task_sys()
{
	while(1){
		MESSAGE msg;
		while(1){
			send_recv(RECEIVE,ANY_TASK,&msg);
			int src = msg.source;

			switch(msg.type){
			case GET_TICKS:
				{
					msg.RETVAL = ticks;
					send_recv(SEND,src,&msg);
				}
				break;
			case FORK:
				{
					msg.RETVAL = do_fork(&msg);
					msg.type = SYSCALL_RET;
					send_recv(SEND,src,&msg);
				}
				break;
			case EXIT:
				{
					do_exit(&msg,msg.STATUS);
					send_recv(SEND,src,&msg);
				}
				break;
			case EXCUTE:
				{
					msg.RETVAL = do_excute(&msg);
					send_recv(SEND,src,&msg);
				}
				break;
			default:
				{}
				break;
			}
		}
	}
}

int do_fork(MESSAGE* msg)
{
	PROCESS* p = proc_table;
	int i ;
	for(i = 0; i < NR_TASKS+NR_PROCS; i++,p++){
		if(p->p_flags == FREE_SLOT)
			break;
	}

	int child_pid = i;
	if(child_pid == NR_TASKS + NR_PROCS)
		return -1;

	//	duplicate the process table
	int pid = msg->source;
	uint16_t child_ldt_sel = p->ldt_sel;
	//*p = proc_table[pid];
	memcpy(p,&proc_table[pid],sizeof(PROCESS));
	p->ldt_sel = child_ldt_sel;
	p->p_parent = pid;
	p->nr_tty = proc_table[pid].nr_tty;

	LDT_DESCRIPTOR* ppd = &proc_table[pid].ldts[0];	//code ldt
	//	base of code seg, in bytes
	int caller_t_base = ppd->baseHi << 24 | ppd->baseMid << 16 | ppd->baseLo;
	//	limit
	int caller_t_limit = (ppd->grand & 0x0f) << 16 | ppd->limit;

	int caller_t_size = (caller_t_limit + 1)*
		((ppd->grand & (DA_LIMIT_4K >> 8)) ? 4096 : 1);

	//	data
	ppd = &proc_table[pid].ldts[1];
	int caller_d_s_base =  ppd->baseHi << 24 | ppd->baseMid << 16 | ppd->baseLo;
	int caller_d_s_limit = (ppd->grand & 0x0f) << 16 | ppd->limit;
	int caller_d_s_size = (caller_d_s_limit + 1)*
		((ppd->grand & (DA_LIMIT_4K >> 8)) ? 4096 : 1);
	//	base of child proc share the same space
	int child_base = (int)pmmgr_alloc_blocks(256);
	memcpy((void*)child_base,(void*)caller_t_base,caller_t_size);

	//	child's ldt
	ldt_set_descriptor(&p->ldts[0],child_base,
		(PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
		DA_LIMIT_4K | DA_32 | DA_C | PRIVILEGE_USER << 5);
	ldt_set_descriptor(&p->ldts[1],child_base,
		(PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
		DA_LIMIT_4K | DA_32 | DA_DRW | PRIVILEGE_USER << 5);

	//	return
	msg->PID = child_pid;

	MESSAGE m;
	m.type = SYSCALL_RET;
	m.RETVAL = 0;
	m.PID = 0;
	send_recv(SEND,child_pid,&m);

	return 0;
}

void do_exit(MESSAGE* msg,int status)
{
	int i;
	int pid = msg->source;
	int parent_id = proc_table[pid].p_parent;
	PROCESS* p = &proc_table[pid];

	if(p->base != 0 && p->blocks != 0){
		pmmgr_free_blocks(p->base,p->blocks * 4096);
	}

	p->exit_code = status;

	cleanup(&proc_table[pid]);

	for(i = 0;i < NR_TASKS+NR_PROCS; i++){
		if(proc_table[i].p_parent == pid){
			proc_table[i].p_parent = INIT;
		}
	}
}

void cleanup(PROCESS* p)
{
	p->p_flags = FREE_SLOT;
}

int do_excute(MESSAGE* msg)
{
	PROCESS* p = proc_table;
	int i ;
	for(i = 0; i < NR_TASKS+NR_PROCS; i++,p++){
		if(p->p_flags == FREE_SLOT)
			break;
	}

	int child_pid = i;
	if(child_pid == NR_TASKS + NR_PROCS){
		return -1;
	}

	//	duplicate the process table
	int pid = msg->source;
	uint16_t child_ldt_sel = p->ldt_sel;
	//*p = proc_table[pid];
	//memcpy(p,&proc_table[pid],sizeof(PROCESS));

	p->ldt_sel = child_ldt_sel;
	p->p_parent = pid;
	p->nr_tty = proc_table[pid].nr_tty;
	p->pid = proc2pid(p);

	LDT_DESCRIPTOR* ppd = &proc_table[pid].ldts[0];	//code ldt
	int child_base = (int)pmmgr_alloc_blocks(256);
	//memcpy((void*)child_base,(void*)caller_t_base,caller_t_size);
	p->base = (void*)child_base;
	p->blocks = 256;

	//	child's ldt
	ldt_set_descriptor(&p->ldts[0],child_base,
		(PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
		DA_LIMIT_4K | DA_32 | DA_C | PRIVILEGE_USER << 5);
	ldt_set_descriptor(&p->ldts[1],child_base,
		(PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
		DA_LIMIT_4K | DA_32 | DA_DRW | PRIVILEGE_USER << 5);
	p->regs.cs = (8*0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_USER;
	p->regs.ds = (8*1 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_USER;
	p->regs.es = (8*1 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_USER;
	p->regs.fs = (8*1 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_USER;
	p->regs.gs = (8*1 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_USER;
	p->regs.ss = (8*1 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_USER;
	p->regs.eflags = 0x202;

	int name_len = msg->NAME_LEN;
	char pathname[MAX_PATH];
	memcpy((void*)va2la(TASK_SYS,pathname),
		(void*)va2la(pid,msg->PATHNAME),
		name_len);
	pathname[name_len] = 0;

	int fd = open(pathname);
	if(fd == -1){
		return -1;
	}
	int file_size = size(fd);

	void* sysbuf = pmmgr_alloc_blocks(file_size / PMMGR_BLOCK_SIZE + 1);
	read(fd,(char*)sysbuf,file_size);
	close(fd);

	memcpy(va2la(child_pid, (void*)0x10000),
		va2la(TASK_SYS,sysbuf),
		file_size);

	proc_table[child_pid].regs.eip = 0x10000;
	proc_table[child_pid].regs.esp = 0x1000;

	strcpy(proc_table[child_pid].p_name,pathname);

	proc_table[child_pid].p_flags = 0;

	return child_pid;
}