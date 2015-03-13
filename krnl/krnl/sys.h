#ifndef _H_MM
#define _H_MM
#include "ctype.h"
#include "ipc.h"

int do_fork(MESSAGE* msg);

void do_exit(MESSAGE* msg,int status);

int do_excute(MESSAGE* msg);

void cleanup(PROCESS* p);

void task_sys();

int fork();

void exit(int code);

#endif