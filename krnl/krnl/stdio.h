#ifndef _H_STDIO
#define	_H_STDIO
#include "ctype.h"
#include "stdarg.h"
#include "va_list.h"
#include "ipc.h"

void itoa(unsigned i,unsigned base,char* buf);

void itoa_s(int i,unsigned base,char* buf);

int _cdecl vsprintf(char* buf,const char* fmt,va_list args);

int _cdecl printf(const char* fmt,...);

int open(char* filename);

int close(int fd);

int read(int fd,char* buf,int len);

int size(int fd);

int exec(char* pathname);

int printx(char* buf,int len);

int send(int dest,MESSAGE* p_msg);

int recv(int src,MESSAGE* p_msg);

#endif