#include "syscall.h"

int send(int dest,MESSAGE* p_msg)
{
	_asm{
		mov eax,SYS_SEND
		mov ecx,dest
		mov edx,p_msg
		int INT_VECTOR_SYS_CALL
		ret
	}
}