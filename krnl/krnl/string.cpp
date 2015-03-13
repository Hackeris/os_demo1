#include "string.h"

char* strcpy(char* dest,const char* src)
{
	char* s1_p = dest;
	while(*dest++ = *src++);
	return s1_p;
}

size_t strlen(const char* str)
{
	size_t len = 0;
	while(str[len++]);
	return len;
}

int strcmp(const char* str1,const char* str2)
{
	int res = 0;
	while(!(res = *(unsigned char *)str1 - *(unsigned char *)str2 ) && *str2){
		++str1;++str2;
	}
	if(res < 0)
		res = -1;
	if(res > 0)
		res = 1;
	return res;
}

void* memcpy(void* dest,const void* src, size_t count)
{
	const char* sp = (const char*)src;
	char* dp = (char*) dest;
	for(;count!=0;count--)*dp++ = *sp++;
	return dest;
}

void* memset(void* dest,char val,size_t count)
{
	unsigned char* tmp = (unsigned char *)dest;
	for(;count!=0;count--,tmp[count] = val);
	return dest;
}

unsigned short* memsetw(unsigned short* dest,unsigned short val,size_t count)
{
	unsigned short *tmp = (unsigned short *)dest;
	for(; count!=0;count--)*tmp++ = val;
	return dest;
}

char* strchr(char* str,int character)
{
	do{
		if(*str == character){
			return (char*)str;
		}
	}
	while(*str++);
	return 0;
}