#ifndef _H_FS
#define _H_FS
#include "ctype.h"

#define DEVICE_MAX		4

typedef struct _FILE{
	char name[32];
	uint32_t	flags;
	uint32_t	file_len;
	uint32_t	id;
	uint32_t	eof;
	uint32_t	position;
	uint32_t	current_cluster;
	uint32_t	device_id;
}FILE;

typedef struct _FILE_SYS{
	char name[8];
	FILE		(*directory)(const char* directory_name);
	void		(*mount)();
	void		(*read)(FILE* file,unsigned char* buffer,uint32_t len);
	void		(*close)(FILE*);
	FILE		(*open)(const char* file_name);
}FILE_SYSTEM;

#define FS_FILE			0
#define FS_DIRECTORY	1
#define FS_INVALID		2
#define FS_NOTHING		FS_INVALID

#define MAX_FILES_PER_PROCESS		8
#define INVALID_FILE_DESC			-1

#define MAX_OPEN_FILE				32

#define MAX_PATH					64

void task_fs();

FILE volOpenFile (const char* fname);
void volReadFile (FILE* file, unsigned char* Buffer, unsigned int Length);
void volCloseFile (FILE* file);
void volRegisterFileSystem (FILE_SYSTEM*, unsigned int deviceID);
void volUnregisterFileSystem (FILE_SYSTEM*);
void volUnregisterFileSystemByID (unsigned int deviceID);

void init_fsys();

int fsys_do_open(const char* filename);

int fsys_do_read(int fd,unsigned char* buffer,unsigned int len);

int fsys_do_close(int fd);

int fsys_get_file_len(int fd);

#endif