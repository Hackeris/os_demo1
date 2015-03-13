#include "ipc.h"
#include "stdio.h"
#include "proc.h"

int read(int fd,char* buf,int len)
{
	MESSAGE msg;

	msg.FD = fd;
	msg.BUF = buf;
	msg.CNT = len;
	msg.type = READ;

	send_recv(BOTH,TASK_FS,&msg);

	return msg.RETVAL;
}