#include "ipc.h"
#include "string.h"
#include "stdio.h"
#include "vmm.h"
#include "syscall.h"

#include "dbg.h"

extern PROCESS proc_table[];


void reset_msg(MESSAGE* msg)
{
	memset(msg,0,sizeof(MESSAGE));
}

void block(PROCESS* p)
{
	//	assert(p->p_flags);
	schedule();
}

void unblock(PROCESS* p)
{
	//	assert(p->p_flags == 0);
}


int deadlock(int src,int dest)
{
	PROCESS* p = proc_table + dest;
	while(1){
		if(p->p_flags & SENDING){
			if(p->p_sendto == src){
				p = proc_table + dest;
				DbgPrintf("=_=%s",p->p_name);
				do{
					//	assert(p->p_msg);
					p = proc_table + p->p_sendto;
					DbgPrintf("->%s",p->p_name);
				}while(p != proc_table+src);
				DbgPrintf("=_=");
				return 1;
			}
			p = proc_table + p->p_sendto;
		}
		else{
			break;
		}
	}
	return 0;
}

int msg_send(PROCESS* current,int dest,MESSAGE* m)
{
	PROCESS*	sender = current;
	PROCESS*	p_dest = proc_table + dest;

	//	assert(proc2pid(sender) != dest)

	if(deadlock(proc2pid(sender),dest)){			//	check for dead lock
		//	dead lock
	}
	if((p_dest->p_flags & RECEIVING) && 
		(p_dest->p_recvfrom == proc2pid(sender)) ||
		(p_dest->p_recvfrom == ANY_TASK)){				//	dest is receiving the message
			//	assert(p_dest->p_msg);
			//	assert(m);

			memcpy(va2la(dest,p_dest->p_msg),
				va2la(proc2pid(sender),m),
				sizeof(MESSAGE));
			p_dest->p_msg = 0;
			p_dest->p_flags &= ~RECEIVING;
			p_dest->p_recvfrom = NO_TASK;
			unblock(p_dest);
	}
	else{		//	dest is not waiting for the message
		sender->p_flags |= SENDING;
		//	assert(sender->p_flags == SENDING);
		sender->p_sendto = dest;
		sender->p_msg = m;

		PROCESS* p;
		if(p_dest->q_sending){
			p = p_dest->q_sending;
			while(p->next_sending){
				p = p->next_sending;
			}
			p->next_sending = sender;
		}
		else{
			p_dest->q_sending = sender;
		}
		sender->next_sending = 0;
		block(sender);

		//	assert(sender->p_flags == SENDING);
		//	assert(sender->p_msg != 0);
		//	assert(sender->p_recvfrom == NO_TASK);
		//	assert(sender->p_sendto == dest);
	}
	return 0;
}

int msg_receive(PROCESS* current,int src,MESSAGE* m)
{
	PROCESS*	p_who_wanna_recv = current;
	PROCESS*	p_from = 0;
	PROCESS*	prev = 0;
	int copyok = 0;

	//	assert(proc2pid(p_who_wanna_recv) != src);
	if( (p_who_wanna_recv->has_int_msg) &&
		((src == ANY_TASK) || (src == INTERRUPT)))
	{
		//	there is an interrupt needs p_who_wanna_recv's handling
		//	and p_who_wanna_recv is ready to handle it
		MESSAGE msg;
		reset_msg(&msg);
		msg.source = INTERRUPT;
		msg.type = HARD_INT;
		//	assert(m);
		memcpy(va2la(proc2pid(p_who_wanna_recv),m),&msg,sizeof(MESSAGE));
		p_who_wanna_recv->has_int_msg =0;

		//	assert(p_who_wanna_recv->p_flags == 0);
		//	assert(p_who_wanna_recv->p_msg ==0);
		//	assert(p_who_wanna_recv->p_sendto == NO_TASK);
		//	assert(p_who_wanna_recv->has_int_msg ==0);

		return 0;
	}

	if(src == ANY_TASK){
		//	p_who_wanna_recv is ready to receive message from 
		//	ANY_TASK proc,we'll check the sending queue and pick the first
		//	proc in it
		if(p_who_wanna_recv->q_sending){
			p_from = p_who_wanna_recv->q_sending;
			copyok = 1;

			//	assert(p_who_wanna_recv->p_flags == 0);
			//	assert(p_who_wanna_recv->p_msg == 0);
			//	assert(p_who_wanna_recv->p_recvfrom == 0);
			//	assert(p_who_wanna_recv->p_sendto == 0);
			//	assert(p_who_wanna_recv->q_sending != 0);
			//	assert(p_from->p_flags == SENDING);
			//	assert(p_from->p_msg != 0);
			//	assert(p_from->p_recvfrom == NO_TASK);
			//	assert(p_from->p_sendto == proc2pid(p_who_wanna_recv));
		}
	}
	else{
		//	p_who_wanna_recv wants to receive a message from
		//	a certain proc: src
		p_from = &proc_table[src];
		if((p_from->p_flags & SENDING) &&
			(p_from->p_sendto == proc2pid(p_who_wanna_recv))){
			//	perfect,src is sending a message to p_who_wanna_recv
			copyok = 1;
			
			PROCESS* p = p_who_wanna_recv;
			//	assert(p);

			while(p){
				//	assert(p_from->p_flags & SENDING);
				if(proc2pid(p) == src){
					p_from = p;
					break;
				}
				prev = p;
				p = p->next_sending;
			}
			//	assert(p_who_wanna_recv->p_flags ==0);
			//	assert(p_who_wanna_recv->p_msg ==0);
			//	assert(p_who_wanna_recv->p_sendto == NO_TASK);
			//	assert(p_who_wanna_recv->p_recvfrom == NO_TASK);
			//	assert(p_who_wanna_recv->q_sending != 0);
			//	assert(p_from->p_flags == SENDING);
			//	assert(p_from->p_msg != 0);
			//	assert(p_from->p_recvfrom == NO_TASK);
			//	assert(p_from->sendto == proc2pid(p_who_wanna_recv));
		}
	}
	if(copyok){
		//	it's determined from which proc the message whill 
		//	be copied. Note htat this proc must have been 
		//	waiting for the moment in the queue, so we should
		//	remove it from the queue
		if(p_from == p_who_wanna_recv->q_sending){
			//	assert(prev == 0);
			p_who_wanna_recv->q_sending = p_from->next_sending;
			p_from->next_sending = 0;
		}
		else{
			//	assert(prev);
			prev->next_sending = p_from->next_sending;
			p_from->next_sending = 0;
		}

		//	assert(m);
		//	assert(p_from->p_msg);
		//	copy the message
		memcpy(va2la(proc2pid(p_who_wanna_recv),m),
			va2la(proc2pid(p_from),p_from->p_msg),
			sizeof(MESSAGE));
		p_from->p_msg = 0;
		p_from->p_sendto = NO_TASK;
		p_from->p_flags &= ~SENDING;
		unblock(p_from);
	}
	else{
		//	nobody's sending msg
		//	set flags so that p_who_wanna_recv will not
		//	be scheduled until it is unblocked

		p_who_wanna_recv->p_flags |= RECEIVING;
		p_who_wanna_recv->p_msg = m;

		if(src == ANY_TASK){
			p_who_wanna_recv->p_recvfrom = ANY_TASK;
		}
		else{
			p_who_wanna_recv->p_recvfrom = proc2pid(p_from);
		}

		block(p_who_wanna_recv);

		//	assert(p_who_wanna_recv->p_flags == RECEIVING);
		//	assert(p_who_wanna_recv->p_msg != 0);
		//	assert(p_who_wanna_recv->p_recvfrom != NO_TASK);
		//	assert(p_who_wanna_recv->p_sendto == NO_TASK);
		//	assert(p_who_wanna_recv->has_int_msg == 0);
	}
	return 0;
}

void inform_int(int task_nr)
{
	PROCESS* p = proc_table + task_nr;

	if ((p->p_flags & RECEIVING) && /* dest is waiting for the msg */
	    ((p->p_recvfrom == INTERRUPT) || (p->p_recvfrom == ANY_TASK))) {
		p->p_msg->source = INTERRUPT;
		p->p_msg->type = HARD_INT;
		p->p_msg = 0;
		p->has_int_msg = 0;
		p->p_flags &= ~RECEIVING; /* dest has received the msg */
		p->p_recvfrom = NO_TASK;
		//assert(p->p_flags == 0);
		unblock(p);

		//assert(p->p_flags == 0);
		//assert(p->p_msg == 0);
		//assert(p->p_recvfrom == NO_TASK);
		//assert(p->p_sendto == NO_TASK);
	}
	else {
		p->has_int_msg = 1;
	}
}


