#include "ipc.h"
#include "stdio.h"
#include "string.h"

int exec(char* pathname)
{
	MESSAGE msg;
	msg.NAME_LEN = strlen(pathname);
	msg.PATHNAME = pathname;
	msg.type = EXCUTE;

	send_recv(BOTH,TASK_SYS,&msg);
	return msg.RETVAL;
}