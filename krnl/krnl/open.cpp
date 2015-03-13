#include "ipc.h"
#include "stdio.h"
#include "proc.h"
#include "string.h"

int open(char* filename)
{
	MESSAGE	msg;

	msg.PATHNAME = filename;
	msg.NAME_LEN = strlen(filename);
	msg.type = OPEN;

	send_recv(BOTH,TASK_FS,&msg);
	return msg.FD;
}