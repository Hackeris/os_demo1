#ifndef _H_STRING
#define _H_STRING
#include "ctype.h"

char* strcpy(char* dest,const char* src);

size_t strlen(const char* str);

int strcmp(const char* str1,const char* str2);

void* memcpy(void* dest,const void* src, size_t count);

void* memset(void* dest,char val,size_t count);

unsigned short* memsetw(unsigned short* dest,unsigned short val,size_t count);

char* strchr(char* str,int character);

#endif