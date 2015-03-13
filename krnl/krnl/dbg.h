#ifndef _H_DBG
#define _H_DBG
#include "ctype.h"

//#define _DEBUG

int DbgPrintf(const char* fmt,...);

void DbgSetColor(uint8_t color);

void DbgGotoXY(uint8_t x,uint8_t y);

void DbgUpdateCur();

void DbgSetColor(uint8_t color);

void DbgCls(uint8_t color);

void DbgPrintMem(void* src,uint32_t len);

void putc(unsigned char c);

#endif