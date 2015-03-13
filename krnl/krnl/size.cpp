#include "ipc.h"
#include "stdio.h"
#include "proc.h"


int size(int fd)
{
	MESSAGE msg;

	msg.FD = fd;
	msg.type = SIZE;

	send_recv(BOTH,TASK_FS,&msg);

	return msg.RETVAL;
}