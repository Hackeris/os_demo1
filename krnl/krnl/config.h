#ifndef _H_CONFIG
#define _H_CONFIG

//	start at 4kb
#define DMA_BUFFER				0x1000 //~ 0x10000

//	start at 256 kb
#define KERNEL_ADDR				0x40000

//	start at 320 kb
#define FS_BUFFER				0x50000

//	start at 384 kb
#define FS_FILE_TABLE			0x60000

//	start at 1MB
#define TASK_STACK				0x100000

#endif