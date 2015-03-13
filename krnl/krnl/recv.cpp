#include "syscall.h"

int recv(int src,MESSAGE* p_msg)
{
	_asm{
		mov eax,SYS_RECV
		mov ecx,src
		mov edx,p_msg
		int INT_VECTOR_SYS_CALL
		ret
	}
}