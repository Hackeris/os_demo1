#ifndef _H_SYSCALL
#define _H_SYSCALL
#include "ctype.h"
#include "proc.h"
#include "ipc.h"

#define		INT_VECTOR_SYS_CALL		0x90

#define NR_SYS_CALL				3

enum SYS_CALLS{
	SYS_SEND,
	SYS_RECV,
	SYS_PRINT,
};

typedef void (*PFUN_SYS_CALL)();

int init_syscall();

void _cdecl sys_call();

int32_t	sys_printx(char* buf,int len,PROCESS* p_proc);

int sys_sendrec(int function,int src_dest,MESSAGE* m,PROCESS* p);

int sys_send(int dest,MESSAGE* msg,PROCESS* p);

int sys_recv(int src,MESSAGE* msg,PROCESS* p);

int get_ticks();

int printx(char* buf,int len);

int send(int dest,MESSAGE* p_msg);

int recv(int src,MESSAGE* p_msg);

#endif