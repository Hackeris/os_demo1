#include "syscall.h"

int printx(char* buf,int len)
{
	_asm{
		mov eax,SYS_PRINT
		mov ecx,buf
		mov edx,len
		int	INT_VECTOR_SYS_CALL
		ret
	}
}