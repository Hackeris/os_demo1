#include "syscall.h"
#include "idt.h"
#include "hal.h"
#include "proc.h"
#include "tty.h"
#include "vmm.h"

#include "dbg.h"

PROCESS*		p_current_proc;
uint32_t		sys_call_num;

extern uint32_t	ticks;

uint32_t		ret_val;

extern TTY	tty_table[];

extern PROCESS proc_table[];

PFUN_SYS_CALL sys_call_table[NR_SYS_CALL] = {
	(PFUN_SYS_CALL)sys_send,
	(PFUN_SYS_CALL)sys_recv,
	(PFUN_SYS_CALL)sys_printx
};

int init_syscall()
{
	init_idt_desc(INT_VECTOR_SYS_CALL,sys_call,DA_386IGATE,PRIVILEGE_USER,0x8);
	return 0;
}

void __declspec(naked) _cdecl sys_call()
{
	_asm mov [sys_call_num],eax
	save();
	//	the function save() put the address of process in esi
	_asm mov [p_current_proc],esi
	_asm sti
	_asm push p_current_proc
	_asm push edx
	_asm push ecx
	sys_call_table[sys_call_num]();
	_asm add esp,4*3
	_asm mov [ret_val],eax
	p_current_proc->regs.eax = ret_val;
	_asm cli
	_asm ret
}


int32_t	sys_printx(char* buf,int len,PROCESS* p_proc)
{
	char* s =(char*)va2la(p_proc->pid,buf);
	tty_write(&tty_table[p_proc->nr_tty],s,len);
	return 0;
}

int sys_sendrec(int function,int src_dest,MESSAGE* m,PROCESS* p)
{
	int ret = 0;
	int caller = p->pid;

	MESSAGE* mla = (MESSAGE*)va2la(caller,m);

	mla->source = caller;

	if(function == SEND){
		ret = msg_send(p,src_dest,m);
	}

	else if(function == RECEIVE){
		ret = msg_receive(p,src_dest,m);
		if(ret != 0)
			return ret;
	}
	else{
		//	panic("sys_sendrec invalid function");
	}

	return 0;
}

int sys_send(int dest,MESSAGE* msg,PROCESS* p)
{
	return sys_sendrec(SEND,dest,msg,p);
}

int sys_recv(int src,MESSAGE* msg,PROCESS* p)
{
	return sys_sendrec(RECEIVE,src,msg,p);
}

int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH,TASK_SYS,&msg);
	return msg.RETVAL;
}