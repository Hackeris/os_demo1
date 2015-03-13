#include "dbg.h"
#include "stdarg.h"
#include "va_list.h"
#include "string.h"
#include "stdio.h"

uint8_t cursor_x=0;
uint8_t	cursor_y=0;
uint8_t	_color=0x07;

uint16_t *video_memory=((uint16_t *)0xB8000);

void putc(unsigned char c)
{
	uint16_t attribute=_color<<8;
	if(c == 0x08 && cursor_x){
		cursor_x--;
	}
	else if(c == 0x09){
		cursor_x=(cursor_x+8)&~(8-1);
	}
	else if(c == '\r'){
		cursor_x=0;
	}
	else if(c == '\n'){
		cursor_x=0;
		cursor_y++;
	}

	else if(c >= ' '){
		uint16_t* location = video_memory + (cursor_y*80 + cursor_x);
		*location= c | attribute;
		cursor_x++;
	}

	if(cursor_x >= 80){
		cursor_x = 0;
		cursor_y ++;
	}
	DbgUpdateCur();
}

void puts(char* str)
{
	if(!str)
		return;
	while(*str){
		putc(*str);
		str++;
	}
}


int DbgPrintf(const char* str,...)
{
	va_list		args;
	va_start (args, str);
	size_t i;
	for (i=0; i<strlen(str);i++) {

		switch (str[i]) {

			case '%':

				switch (str[i+1]) {

					/*** characters ***/
					case 'c': {
						char c = va_arg (args, char);
						putc (c);
						i++;		// go to next character
						break;
					}

					/*** address of ***/
					case 's': {
						int c = (int&) va_arg (args, char);
						char str[64];
						strcpy (str,(const char*)c);
						puts (str);
						i++;		// go to next character
						break;
					}

					/*** integers ***/
					case 'd':
					case 'i': {
						int c = va_arg (args, int);
						char str[32]={0};
						itoa_s (c, 10, str);
						puts (str);
						i++;		// go to next character
						break;
					}

					/*** display in hex ***/
					case 'X':
					case 'x': {
						int c = va_arg (args, int);
						char str[32]={0};
						itoa_s (c,16,str);
						puts (str);
						i++;		// go to next character
						break;
					}

					default:
						va_end (args);
						return 1;
				}

				break;

			default:
				putc (str[i]);
				break;
		}

	}

	va_end (args);
	return i;
}

void DbgSetColor(uint8_t color)
{
	_color = color;
}

void DbgCls(uint8_t color)
{
	unsigned long ulBase=0xb8000;
	unsigned long i=0;
	while(i<80*25){
		*(char*)ulBase=' ';
		ulBase++;
		*(char*)ulBase=color;
		ulBase++;
		i++;
	}
	DbgGotoXY(0,0);
}

void DbgGotoXY(uint8_t x,uint8_t y)
{
	if (cursor_x <= 80)
	    cursor_x = x;

	if (cursor_y <= 25)
	    cursor_y = y;

	DbgUpdateCur();
}

void DbgUpdateCur()
{
	// the x is in the bl
	// the y is in the bh
	// write y*80+x to CRT data register
	__asm{
		pusha
		mov bl,[cursor_x]
		mov bh,[cursor_y]
		xor	eax,eax
		mov ecx,80
		mov al,bh
		mul	ecx
		add	al,bl
		mov ebx,eax				// ebx = y*80+x

		mov al,0x0f
		mov dx,0x03d4
		out	dx,al
		
		mov al,bl				//high byte
		mov dx,0x03d5
		out	dx,al

		xor eax,eax

		mov al,0x0e
		mov dx,0x03d4
		out	dx,al

		mov al,bh				//low byte
		mov dx,0x03d5
		out	dx,al

		popa
	}
}

void DbgPrintMem(void* src,uint32_t len)
{
	uint8_t* p = (uint8_t*)src;
	uint32_t i;
	for(i=0;i<len;i++){
		DbgPrintf("%x",p[i]);
	}
}