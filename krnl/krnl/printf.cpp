#include "stdio.h"
#include "syscall.h"

int _cdecl printf(const char* fmt,...)
{
	int i;
	char buf[256];

	va_list arg = (va_list)((char*)(&fmt) + 4);
	i = vsprintf(buf,fmt,arg);
	printx(buf,i);

	return i;
}