#include "ipc.h"
#include "vmm.h"
#include "syscall.h"
#include "string.h"

int send_recv(int function,int src_dest,MESSAGE* msg)
{
	int ret = 0;

	if(function == RECEIVE){
		memset(msg,0,sizeof(MESSAGE));
	}

	switch(function){
	case BOTH:
		{
			ret = send(src_dest,msg);
			if(ret == 0){
				ret = recv(src_dest,msg);
			}
		}
		break;
	case SEND:
		{
			ret = send(src_dest,msg);
		}
		break;
	case RECEIVE:
		{
			ret = recv(src_dest,msg);
		}
		break;
	default:
		{
			//	assert((function == BOTH) || (function == SEND) || (function == RECEIVE));
		}
		break;
	}
	return ret;
}