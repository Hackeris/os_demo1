#include "ipc.h"

void exit(int code)
{
	MESSAGE msg;

	msg.type = EXIT;
	msg.STATUS = code;
	send_recv(BOTH,TASK_SYS,&msg);
}