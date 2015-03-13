#ifndef _H_IPC
#define _H_IPC
#include "ctype.h"
#include "proc.h"
#include "msg.h"

/* ipc */
#define SEND		1
#define RECEIVE		2
#define BOTH		3	/* BOTH = (SEND | RECEIVE) */


void reset_msg(MESSAGE* msg);

void block(PROCESS* p);

void unblock(PROCESS* p);

int deadlock(int src,int dest);

int msg_send(PROCESS* current,int dest,MESSAGE* m);

int msg_receive(PROCESS* current,int src,MESSAGE* m);

void inform_int(int task_nr);

int send_recv(int function,int src_dest,MESSAGE* msg);

#endif